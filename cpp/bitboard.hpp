/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <regex>
#include <type_traits>
#include <vector>

namespace tictactoe {

enum class Player : bool { ONE, TWO };

constexpr Player other(Player p) noexcept { return static_cast<Player>(!static_cast<bool>(p)); }

namespace bitboard {
// Type definitions

enum class BBoard : uint16_t { EMPTY = 0, FULL = 0x1FF, LENGTH = 9 };
enum class Move : int {begin = 0, end = 9};
enum class Eval : int { DRAW = 0, WON = 1 << 20, LOST = -WON };

template<typename T>
auto underlying(T t) {
	return static_cast<std::underlying_type_t<T>>(t);
}

constexpr BBoard operator|(BBoard lhs, BBoard rhs) noexcept {return BBoard(underlying(lhs) | underlying(rhs));}
constexpr BBoard& operator|=(BBoard &lhs, BBoard rhs) noexcept {return lhs = lhs | rhs;}

constexpr BBoard operator&(BBoard lhs, BBoard rhs) noexcept {return BBoard(underlying(lhs) & underlying(rhs));}
constexpr BBoard& operator&=(BBoard &lhs, BBoard rhs) noexcept {return lhs = lhs & rhs;}

constexpr BBoard operator^(BBoard lhs, BBoard rhs) noexcept {return BBoard(underlying(lhs) ^ underlying(rhs));}
constexpr BBoard& operator^=(BBoard &lhs, BBoard rhs) noexcept {return lhs = lhs ^ rhs;}

constexpr BBoard operator<<(int lhs, Move rhs) noexcept {return BBoard(lhs << underlying(rhs));}

constexpr BBoard operator>>(BBoard lhs, int rhs) noexcept {return BBoard(underlying(lhs) >> rhs);}
constexpr BBoard operator<<(BBoard lhs, int rhs) noexcept {return BBoard(underlying(lhs) << rhs);}

constexpr Move &operator++(Move &m) {
	m = Move(underlying(m) + 1);
	return m;
}

constexpr Eval operator-(Eval e) noexcept {return Eval(-(underlying(e)));}
constexpr Eval operator/(Eval lhs, int rhs) noexcept {return Eval(underlying(lhs) / rhs);}
constexpr Eval operator*(Eval lhs, int rhs) noexcept {return Eval(underlying(lhs) * rhs);}

class Board {
private:
	std::array<BBoard, 2> _bboards;
public:
	constexpr Board() = default;
	constexpr Board(BBoard lhs, BBoard rhs): _bboards{lhs, rhs} {}

	constexpr BBoard operator[](Player p) const noexcept {return _bboards[underlying(p)];}
	constexpr BBoard& operator[](Player p) noexcept {return _bboards[underlying(p)];}

	constexpr BBoard one() const noexcept {return _bboards.front();};
	constexpr BBoard& one() noexcept {return _bboards.front();};

	constexpr BBoard two() const noexcept {return _bboards.back();};
	constexpr BBoard& two() noexcept {return _bboards.back();};
};

/**
Play move for player and return new board
No boundary nor type check for performance reason!
*/
constexpr Board play(Board b, const Player p, const Move m) noexcept {
	b[p] |= 1 << m;
	return b;
}

/// Is board b legal?
constexpr bool is_legal(const Board b) noexcept {
	return (b.one() & b.two()) == BBoard::EMPTY
		&& (b.one() >> 9) == BBoard::EMPTY
		&& (b.two() >> 9) == BBoard::EMPTY;
}

constexpr BBoard both(Board b) noexcept { return b.one() | b.two(); }

/// Is board b full?
constexpr bool is_full(const Board b) noexcept { return both(b) == BBoard::FULL; }

/// Is board b won?
constexpr bool is_won(const Board b, const Player p) noexcept {
	constexpr std::array<BBoard, 8> wins = {BBoard{0b000000111}, BBoard{0b000111000}, BBoard{0b111000000},
											BBoard{0b100100100}, BBoard{0b010010010}, BBoard{0b001001001},
											BBoard{0b100010001}, BBoard{0b001010100}};

	const BBoard bb = b[p];
	return std::any_of(wins.begin(), wins.end(), [bb](const BBoard w) { return (w & bb) == w; });
}

constexpr bool is_move(const Board b, const Move m) noexcept {
	const auto bb = both(b);
	return static_cast<unsigned int>(m) < 9 // underflow becomes overflow for unsigned
		&& (bb | 1 << m) != bb;
}

/// Find first unset bit
constexpr Move find_first(const BBoard bb) noexcept { return Move(std::countr_one(underlying(bb))); }

/// Find next unset bit after m
constexpr Move find_next(const BBoard bb, const Move m) noexcept {
	const int next = underlying(m) + 1;
	return Move(next + std::countr_one(underlying(bb >> next)));
}

/**
Return score of move (already played in order not to put it onto the stack).
Uses minimax (negamax) algorithm.
*/
constexpr Eval minimax(const Board b, const Player p) noexcept {
	if (is_won(b, p)) { return Eval::WON; }
	if (is_full(b)) { return Eval::DRAW; }
	const Player o = other(p);

	Eval e          = Eval::WON;
	BBoard bb       = both(b);
	for (Move m = find_first(bb); m < Move::end && e != Eval::LOST; bb ^= (1 << m), m = find_first(bb)) {
		e = std::min(e, -minimax(play(b, o, m), o));
	}
	return e;
}

/**
Return score of move (already played in order not to put it onto the stack).
Uses alpha beta pruning algorithm.
*/
constexpr Eval alphabeta(const Board b, const Player p, const Eval alpha = Eval::LOST) noexcept {
	if (is_won(b, p)) { return Eval::WON; }
	if (is_full(b)) { return Eval::DRAW; }
	const Player o = other(p);
	// Mutable:
	Eval beta       = Eval::WON;
	BBoard bb       = both(b);
	for (Move m = find_first(bb); m < Move::end; m = find_first(bb)) {
		beta = std::min(beta, -alphabeta(play(b, o, m), o, -beta));
		if (beta <= alpha) break;
		bb ^= (1 << m);
	}
	return beta / 2; // The "closer" a win, the higher it's score
}

/**
Return best move.
If randomize is set, chose randomly between moves with equal score.
 */
template<bool randomize = true> std::pair<Move, Eval> best_move(const Board b, const Player p) noexcept {
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::bernoulli_distribution rand_bool;

	// Mutable:
	Eval ev   = Eval::LOST * 2;
	Move mo{-1};
	BBoard bb = both(b);
	for (Move m = find_first(bb); m < Move::end; m = find_first(bb)) {
		const Eval e = alphabeta(play(b, p, m), p);
		if (e > ev) {
			ev = e;
			mo = m;
		} else if (e == ev && rand_bool(rand_engine)) {
			mo = m;
		}
		bb ^= (1 << m);
	}
	return {mo, ev};
}

template<> constexpr std::pair<Move, Eval> best_move<false>(const Board b, const Player p) noexcept {
	// Mutable:
	Eval ev   = Eval::LOST * 2;
	Move mo{-1};
	BBoard bb = both(b);
	for (Move m = find_first(bb); m < Move::end; m = find_first(bb)) {
		const Eval e = alphabeta(play(b, p, m), p);
		if (e > ev) {
			ev = e;
			mo = m;
		}
		bb ^= (1 << m);
	}
	return {mo, ev};
}

constexpr std::optional<Board> str2board(std::string_view s) noexcept {
	if (s.size() != 9) { return {}; }
	Board b{};
	for (Move m = Move::begin; m < Move::end; ++m) {
		switch(s[underlying(m)]) {
		case 'X': 
		case 'x': b = play(b, Player::ONE, m); break;
		case 'O':
		case 'o': b = play(b, Player::TWO, m); break;
		case '.': break;
		default: return {};
		}
	}
	return b;
}

constexpr std::optional<std::string> board2str(Board b) noexcept {
	if (!is_legal(b)) { return {}; }
	const auto one = b.one();
	const auto two = b.two();
	auto s         = std::string(9, '.');
	for (Move m = Move::begin; m < Move::end; ++m) {
		const BBoard bi = 1 << m;
		if ((one & bi) != BBoard::EMPTY) {
			s[underlying(m)] = 'x';
		} else if ((two & bi) != BBoard::EMPTY) {
			s[underlying(m)] = 'o';
		}
	}
	return s;
}

constexpr std::optional<Move> str2move(Board b, std::string_view s) noexcept {
	if (s.size() != 2) { return {}; }

	const int col = std::tolower(s[0]) - 'a';
	if (col < 0 || col > 2){ return {}; }

	const int row = s[1] - '1';
	if (row < 0 || row > 2){ return {}; }

	const Move m{row * 3 + col};

	if (is_move(b, m)) { return m; }
	return {};
}

inline void test() {
	// Random device
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::uniform_int_distribution<uint16_t> rand_bboard(0, underlying(BBoard::FULL));

	// other
	assert(other(Player::ONE) == Player::TWO);
	assert(other(Player::TWO) == Player::ONE);
	assert(other(other(Player::ONE)) == Player::ONE);
	assert(other(other(Player::TWO)) == Player::TWO);

	// is_legal, is_move
	assert(is_legal(Board{}));
	assert(is_legal(Board(BBoard{1}, BBoard::EMPTY)));

	Board b{};
	Player p = Player::ONE;
	for (Move m = Move::begin; m < Move::end; ++m) {
		assert(play(Board{}, Player::ONE, m).one() == play(Board{}, Player::TWO, m).two());
		assert(is_move(b, m));
		assert(!is_move(b, Move(underlying(m) - 1)));
		b = play(b, p, m);
		p = other(p);
		assert(is_legal(b));
	}

	// is_full
	assert(is_full(Board{BBoard::FULL, BBoard::EMPTY}));
	assert(is_full(Board{BBoard::EMPTY, BBoard::FULL}));
	assert(!is_full(Board{}));

	// is_won
	assert(!is_won(Board{}, Player::ONE));

	// minimax
	b = play(Board{}, Player::ONE, Move(0));
	assert(minimax(b, Player::ONE) == Eval::DRAW);
	b = play(b, Player::ONE, Move(1));
	assert(minimax(b, Player::ONE) == Eval::WON);
	assert(minimax(b, Player::TWO) == Eval::LOST);

	// alphabeta
	b = play(Board{}, Player::ONE, Move(0));
	assert(alphabeta(b, Player::ONE) == Eval::DRAW);
	b = play(b, Player::ONE, Move(1));
	assert(alphabeta(b, Player::ONE) > Eval::DRAW);
	assert(alphabeta(b, Player::TWO) < -Eval::DRAW);

	// best_move
	assert(best_move<false>(Board{}, Player::ONE) == std::make_pair(Move(0), Eval(0)));
	assert(best_move(Board{}, Player::ONE).second == Eval::DRAW);
	// b.one = 0b110000000 and is won whoever plays
	assert(best_move(b, Player::ONE).second > Eval::DRAW);
	assert(best_move(b, Player::TWO).second < -Eval::DRAW);
}
} // namespace bitboard
} // namespace tictactoe
#endif
