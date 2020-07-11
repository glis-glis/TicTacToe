/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <cassert>
#include <string>

#include "bitboard.hpp"

namespace tictactoe
{

enum class Player : bool { ONE, TWO };

Player other(Player p)
{
	return static_cast<Player>(!static_cast<bool>(p));
}

class Engine
{
    public:
	/// Constructor, on purpose without board parameter to remove the need
	/// of exceptions
	Engine() noexcept {}

	/// set board to `board`, return false if `board` illegal
	void reset() noexcept { this->_board = bitboard::BBoards::EMPTY; }

	/// set board to `board`, return false if `board` illegal
	bool set(std::string board) noexcept {

		if (const auto b = bitboard::str2board(board)) {
			this->_board = *b;
			return true;
		}
		return false;
	}

	bool is_won(Player player) const noexcept
	{
		const auto p = (player == Player::ONE ? bitboard::Players::ONE
		                                      : bitboard::Players::TWO);
		return bitboard::is_won(this->_board, p);
	}
	bool is_full()  const noexcept
	{
		return bitboard::is_full(this->_board);
	}
	bool is_finished() const noexcept {
		return this->is_full() ||
		       this->is_won(Player::ONE) || this->is_won(Player::TWO);
	}

	/// Play `move` for `player`, return new board
	bool play(Player player, std::string move) noexcept 
	{
		const auto p = (player == Player::ONE ? bitboard::Players::ONE
		                                      : bitboard::Players::TWO);

		if (const auto m = bitboard::str2move(this->_board, move)) {
			this->_board = bitboard::play(this->_board, p, *m);
			return true;
		}
		return false;
	}

	/// Return actual board
	std::string board() const noexcept { return ""; }

	/// Play best move for `player`, return new board
	bool play_best(Player player) noexcept { return bool(player); }

    private:
	bitboard::Board _board = bitboard::BBoards::EMPTY;

};


std::ostream &operator<<(std::ostream &os, const Engine e) { 
	return os << e.board();
}

void test()
{
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
	assert(e.board() == "x..o.....");

	assert(e.play(Player::ONE, "b2"));
	assert(!e.play(Player::ONE, "b2"));
	assert(e.board() == "x..ox....");

	assert(e.play(Player::TWO, "b3"));
	assert(!e.play(Player::TWO, "b3"));
	assert(e.board() == "x..oxo...");

	assert(e.play(Player::ONE, "c3"));
	assert(!e.play(Player::ONE, "c3"));
	assert(e.board() == "x..ox0..x");

	assert(e.is_won(Player::ONE));
	assert(e.is_finished());

	e.reset();

	assert(e.play_best(Player::ONE));
	assert(e.play_best(Player::ONE));
	assert(e.play_best(Player::ONE));
	assert(e.is_won(Player::ONE));
	e.reset();
}
} // namespace tictactoe
