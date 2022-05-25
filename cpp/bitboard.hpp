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
#include <vector>

namespace tictactoe {

enum class Player : bool { ONE, TWO };

constexpr Player other(Player p) noexcept { return static_cast<Player>(!static_cast<bool>(p)); }

namespace bitboard {
// Type definitions

using BBoard  = unsigned int; /// One bitboard
using Board   = unsigned int; /// All three bitboard
using BPlayer = int;          /// 0, 9, or 18
using Move    = int;          /// From 0 to 8, however, the <bit> functions return signed
using Eval    = int;          /// -1, 0 or 1

/// Index of start of bitboards
enum BPlayers : BPlayer { ONE = 0, TWO = 9, BOTH = 18 };

constexpr BPlayer bplayer(Player p) noexcept {
	std::array<BPlayers, 2> ar{BPlayers::ONE, BPlayers::TWO};
	return ar[p != Player::ONE];
}

/// Bitboard constants
enum BBoards : BBoard { EMPTY = 0, FULL = 0x1FF, LENGTH = 9 };

/// Evaluation constants
enum Evals : Eval { DRAW = 0, WON = 1 << BPlayers::BOTH };

/// Return other player.
constexpr BPlayer other(const BPlayer p) noexcept {
	// 0^9 -> 9
	// 9^9 -> 0
	return p ^ BPlayers::TWO;
}

/**
Return board of bitboards b1 and b2. No boundary nor type check for
performance reason!
 */
constexpr Board board(const BBoard b1, const BBoard b2) noexcept {
	return b1 | (b2 << BPlayers::TWO) | ((b1 | b2) << BPlayers::BOTH);
}


/**
Return bitboard of board b for player p. No boundary nor type check for
performance reason!
 */
constexpr BBoard bboard(const Board b, const BPlayer p) noexcept { return (b >> p) & BBoards::FULL; }

template<BPlayer p>
constexpr BBoard bboard(const Board b) noexcept { return (b >> p) & BBoards::FULL; }
template<>
constexpr BBoard bboard<0>(const Board b) noexcept { return b & BBoards::FULL; }

/**
Play move for player and return new board
No boundary nor type check for performance reason!
*/
constexpr Board play(const Board b, const BPlayer p, const Move m) noexcept {
	return b | (1 << (m + p)) | (1 << (m + BPlayers::BOTH));
}

/// Is board b legal?
constexpr bool is_legal(const Board b) noexcept {
	const BBoard bb1     = bboard<BPlayers::ONE>(b);
	const BBoard bb2     = bboard<BPlayers::TWO>(b);
	const BBoard bb_both = bboard<BPlayers::BOTH>(b);
	return !(bb1 & bb2) && ((bb1 | bb2) == bb_both);
}

/// Is board b full?
constexpr bool is_full(const Board b) noexcept { return bboard<BPlayers::BOTH>(b) == BBoards::FULL; }

/// Is board b won?
constexpr bool is_won(const Board b, const BPlayer p) noexcept {
	constexpr std::array<BBoard, 8> wins = {0b000000111, 0b000111000, 0b111000000, 0b100100100,
											0b010010010, 0b001001001, 0b100010001, 0b001010100};

	const BBoard bb = bboard(b, p);
	return std::any_of(wins.begin(), wins.end(), [bb](const BBoard w) { return (w & bb) == w; });
}

constexpr bool is_move(const Board b, const Move m) noexcept {
	return static_cast<unsigned int>(m) < 9 && (play(b, BPlayers::BOTH, m) != b);
}

/// Find first unset bit
constexpr Move find_first(const BBoard bb) noexcept { return std::countr_one(bb); }

/// Find next unset bit after m
constexpr Move find_next(const BBoard bb, const Move m) noexcept {
	const Move next = m + 1;
	return next + std::countr_one(bb >> next);
}

/**
Return score of move (already played in order not to put it onto the stack).
Uses minimax (negamax) algorithm.
*/
constexpr Eval minimax(const Board b, const BPlayer p) noexcept {
	if (is_won(b, p)) { return Evals::WON; }
	if (is_full(b)) { return Evals::DRAW; }
	const BPlayer o = other(p);
	// Mutable:
	Eval e          = Evals::WON;
	BBoard bb       = bboard(b, BPlayers::BOTH);
	for (Move m = find_first(bb); m < BPlayers::TWO && e != -Evals::WON; bb ^= (1 << m), m = find_first(bb)) {
		e = std::min(e, -minimax(play(b, o, m), o));
	}
	return e;
}

/**
Return score of move (already played in order not to put it onto the stack).
Uses alpha beta pruning algorithm.
*/
constexpr Eval alphabeta(const Board b, const BPlayer p, const Eval alpha = -Evals::WON) noexcept {
	if (is_won(b, p)) { return Evals::WON; }
	if (is_full(b)) { return Evals::DRAW; }
	const BPlayer o = other(p);
	// Mutable:
	Eval beta       = Evals::WON;
	BBoard bb       = bboard<BPlayers::BOTH>(b);
	for (Move m = find_first(bb); m < BPlayers::TWO; m = find_first(bb)) {
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
template<bool randomize = true> std::pair<Move, Eval> best_move(const Board b, const BPlayer p) noexcept {
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::bernoulli_distribution rand_bool;

	// Mutable:
	Eval ev   = -2 * Evals::WON;
	Move mo   = -1;
	BBoard bb = bboard(b, BPlayers::BOTH);
	for (Move m = find_first(bb); m < BPlayers::TWO; m = find_first(bb)) {
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

template<> constexpr std::pair<Move, Eval> best_move<false>(const Board b, const BPlayer p) noexcept {
	// Mutable:
	Eval ev   = -2 * Evals::WON;
	Move mo   = -1;
	BBoard bb = bboard<BPlayers::BOTH>(b);
	for (Move m = find_first(bb); m < BPlayers::TWO; m = find_first(bb)) {
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
	Board b = BBoards::EMPTY;
	for (size_t i = 0; i < BBoards::LENGTH; ++i) {
		switch(s[i]) {
		case 'X': 
		case 'x': b = play(b, BPlayers::ONE, Move(i)); break;
		case 'O':
		case 'o': b = play(b, BPlayers::TWO, Move(i)); break;
		case '.': break;
		default: return {};
		}
	}
	return b;
}

constexpr std::optional<std::string> board2str(Board b) noexcept {
	if (!is_legal(b)) { return {}; }
	const auto one = bboard(b, BPlayers::ONE);
	const auto two = bboard(b, BPlayers::TWO);
	auto s         = std::string(9, '.');
	for (size_t i = 0; i < BBoards::LENGTH; ++i) {
		const BBoard bi = 1 << i;
		if (one & bi) {
			s[i] = 'x';
		} else if (two & bi) {
			s[i] = 'o';
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

	const Move m = row * 3 + col;

	if (is_move(b, m)) { return m; }
	return {};
}

inline void test() {
	// Random device
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::uniform_int_distribution<BBoard> rand_bboard(0, BBoards::FULL);

	// other
	assert(other(BPlayers::ONE) == BPlayers::TWO);
	assert(other(BPlayers::TWO) == BPlayers::ONE);
	assert(other(other(BPlayers::ONE)) == BPlayers::ONE);
	assert(other(other(BPlayers::TWO)) == BPlayers::TWO);

	// board
	assert(board(BBoards::EMPTY, BBoards::EMPTY) == BBoards::EMPTY);

	// bboard
	assert(bboard(BBoards::EMPTY, BPlayers::ONE) == BBoards::EMPTY);
	assert(bboard(BBoards::EMPTY, BPlayers::TWO) == BBoards::EMPTY);
	assert(bboard(BBoards::EMPTY, BPlayers::BOTH) == BBoards::EMPTY);
	for (BBoard bb = BBoards::EMPTY; bb < BBoards::FULL + 1; ++bb) {
		const BBoard noise1 = rand_bboard(rand_engine);
		const BBoard noise2 = rand_bboard(rand_engine);
		const Board b1      = board(bb, noise2);
		const Board b2      = board(noise1, bb);
		assert(bboard(b1, BPlayers::ONE) == bb);
		assert(bboard(b2, BPlayers::TWO) == bb);
	}

	// is_legal, is_move
	assert(is_legal(board(BBoards::EMPTY, BBoards::EMPTY)));
	assert(is_legal(board(1, BBoards::EMPTY)));
	assert(!is_legal(1));

	Board b   = BBoards::EMPTY;
	BPlayer p = BPlayers::ONE;
	for (Move m = 0; m < 9; ++m) {
		assert(bboard(play(BBoards::EMPTY, BPlayers::ONE, m), BPlayers::ONE) ==
			   bboard(play(BBoards::EMPTY, BPlayers::TWO, m), BPlayers::TWO));
		assert(is_move(b, m));
		assert(!is_move(b, m - 1));
		b = play(b, p, m);
		p = other(p);
		assert(is_legal(b));
	}

	// is_full
	assert(is_full(BBoards::FULL | (BBoards::FULL << BPlayers::BOTH)));
	assert(is_full((BBoards::FULL << BPlayers::TWO) | (BBoards::FULL << BPlayers::BOTH)));
	assert(!is_full(BBoards::EMPTY));

	// is_won
	assert(!is_won(BBoards::EMPTY, BPlayers::ONE));

	// minimax
	b = play(BBoards::EMPTY, BPlayers::ONE, 0);
	assert(minimax(b, BPlayers::ONE) == Evals::DRAW);
	b = play(b, BPlayers::ONE, 1);
	assert(minimax(b, BPlayers::ONE) == Evals::WON);
	assert(minimax(b, BPlayers::TWO) == -Evals::WON);

	// alphabeta
	b = play(BBoards::EMPTY, BPlayers::ONE, 0);
	assert(alphabeta(b, BPlayers::ONE) == Evals::DRAW);
	b = play(b, BPlayers::ONE, 1);
	assert(alphabeta(b, BPlayers::ONE) > Evals::DRAW);
	assert(alphabeta(b, BPlayers::TWO) < -Evals::DRAW);

	// best_move
	assert(best_move<false>(BBoards::EMPTY, BPlayers::ONE) == std::make_pair(Move(0), Eval(0)));
	assert(best_move(BBoards::EMPTY, BPlayers::ONE).second == Evals::DRAW);
	// b.one = 0b110000000 and is won whoever plays
	assert(best_move(b, BPlayers::ONE).second > Evals::DRAW);
	assert(best_move(b, BPlayers::TWO).second < -Evals::DRAW);
}
} // namespace bitboard
} // namespace tictactoe
#endif
