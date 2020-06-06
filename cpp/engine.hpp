/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <cassert>
#include <string>

namespace tictactoe {

class Engine
{
    public:
	/// Constructor, on purpose without board parameter to remove the need
	/// of exceptions
	Engine() {}

	/// set board to `board`, return false if `board` illegal
	void reset() {
	}

	/// set board to `board`, return false if `board` illegal
	bool set(std::string board) {
		return board == "";
	}

	bool is_won(std::string player) { return player == ""; }
	bool is_full() { return false; }
	bool is_finished() { return false; }

	/// Play `move` for `player`, return new board
	std::string play(std::string player, std::string move) {
		return player + move;
	}

	/// Return actual board
	std::string board() {
		return "";
	}

	/// Play best move for `player`, return new board
	std::string play_best(std::string player) {
		return player;
	}

};

void test() {
	Engine e;
	assert(!e.is_won("x"));
	assert(!e.is_won("o"));
	assert(e.set("xxx......"));
	assert(e.is_won("x"));
	assert(!e.is_won("o"));

	assert(e.set("ooo......"));
	assert(e.is_won("o"));
	assert(!e.is_won("x"));

	e.reset();

	assert(e.play("x", "a1")  == "x........");
	assert(e.board() == "x........");
	assert(e.play("o", "b1")  == "x..o.....");
	assert(e.play("x", "b2")  == "x..ox....");
	assert(e.play("o", "b3")  == "x..oxo...");
	assert(e.play("x", "c3")  == "x..oxo..x");
	assert(e.is_won(""));

	e.reset();

	assert(e.play_best("x") == "x........");
	e.play_best("x");
	assert(e.is_won("x"));
	e.reset();
}
}
