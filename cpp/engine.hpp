/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cassert>
#include <string>

#include "bitboard.hpp"

namespace tictactoe {

class Engine {
private:
	bitboard::Board _board = bitboard::BBoards::EMPTY;
public:
	constexpr Engine() noexcept = default;

	constexpr void reset() noexcept { this->_board = bitboard::BBoards::EMPTY; }

	/// set board to `board`, return false if `board` illegal
	bool set(std::string_view board) noexcept {
		if (const auto b = bitboard::str2board(board)) {
			this->_board = *b;
			return true;
		}
		return false;
	}

	constexpr bool is_won(Player player) const noexcept { return bitboard::is_won(this->_board, bitboard::bplayer(player)); }
	constexpr bool is_full() const noexcept { return bitboard::is_full(this->_board); }
	constexpr bool is_finished() const noexcept {
		return this->is_full() || this->is_won(Player::ONE) || this->is_won(Player::TWO);
	}

	/// Play `move` for `player`, return false if `move` illegal
	bool play(Player player, std::string_view move) noexcept {
		if (const auto m = bitboard::str2move(this->_board, move)) {
			this->_board = bitboard::play(this->_board, bitboard::bplayer(player), *m);
			return true;
		}
		return false;
	}

	/// Return actual board
	std::string board() const noexcept {
		if (const auto s = bitboard::board2str(this->_board)) { return *s; }
		return std::string(9, '.');
	}

	/// Play best move for `player`, return new board
	bool play_best(Player player) noexcept {
		if (is_finished()) { return false; }
		const auto bp = bitboard::bplayer(player);
		const auto m  = bitboard::best_move(this->_board, bp).first;
		this->_board  = bitboard::play(this->_board, bp, m);
		return true;
	}
};

inline std::ostream &operator<<(std::ostream &os, const Engine e) {
	const auto b = e.board();
	return os << "3|" << b.substr(6, 3) << "|\n"
			  << "2|" << b.substr(3, 3) << "|\n"
			  << "1|" << b.substr(0, 3) << "|\n"
			  << "  --- \n"
			  << " |abc|";
}

inline void test() {
	Engine e;
	assert(!e.is_won(Player::ONE));
	assert(!e.is_won(Player::TWO));
	assert(e.set("xxx......"));
	assert(e.is_won(Player::ONE));
	assert(!e.is_won(Player::TWO));

	assert(e.set("ooo......"));
	assert(e.is_won(Player::TWO));
	assert(!e.is_won(Player::ONE));

	assert(e.set("oxoxoxoxo"));
	assert(e.is_full());
	assert(e.is_won(Player::TWO));
	assert(!e.is_won(Player::ONE));
	assert(e.is_finished());
	e.reset();

	assert(e.play(Player::ONE, "a1"));
	assert(!e.play(Player::ONE, "a1"));
	assert(e.board() == "x........");

	assert(e.play(Player::TWO, "b1"));
	assert(!e.play(Player::TWO, "b1"));
	assert(e.board() == "xo.......");

	assert(e.play(Player::ONE, "b2"));
	assert(!e.play(Player::ONE, "b2"));
	assert(e.board() == "xo..x....");

	assert(e.play(Player::TWO, "b3"));
	assert(!e.play(Player::TWO, "b3"));
	assert(e.board() == "xo..x..o.");

	assert(e.play(Player::ONE, "c3"));
	assert(!e.play(Player::ONE, "c3"));
	assert(e.board() == "xo..x..ox");

	assert(e.is_won(Player::ONE));
	assert(e.is_finished());

	e.reset();

	assert(e.play(Player::ONE, "b2"));
	assert(e.play(Player::TWO, "b1"));
	assert(e.play_best(Player::ONE));
	assert(e.play_best(Player::TWO));
	assert(e.play_best(Player::ONE));
	assert(e.play_best(Player::TWO));
	assert(e.play_best(Player::ONE));
	assert(e.is_won(Player::ONE));
}
} // namespace tictactoe
#endif
