/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <cctype>
#include <iostream>
#include <string>

#include "engine.hpp"

using namespace tictactoe;

Player get_player() {
	while (true) {
		std::cout << "Do you want to start? [y/n] ";

		std::string s;
		std::getline(std::cin, s);
		if (s.length() != 1) continue;

		switch (s[0]) {
		case 'Y':
		case 'y':return Player::ONE;
		case 'N':
		case 'n':return Player::TWO;
		default: continue;
		}
	}
}

int main() {
	std::cout << "Welcome to TicTacToe\n";
	Engine e;
	std::cout << e << '\n';
	const auto player = get_player();
	if (player == Player::ONE) {
		std::cout << "You play with x\n";
	} else {
		e.play_best(other(player));
		std::cout << e << "\nYou play with o\n";
	}

	while (!e.is_finished()) {
		std::cout << "Your move? [a1-c3] ";
		std::string s;
		std::getline(std::cin, s);
		if (!e.play(player, s)) {
			std::cout << "Illegal Move!\n";
			continue;
		}
		e.play_best(other(player));
		std::cout << e << '\n';
	}
	if (e.is_won(other(player))) {
		std::cout << "I win!\n";
	} else if (e.is_won(player)) {
		std::cout << "You win!\n";
	} else {
		std::cout << "Game drawn!\n";
	}
	return 0;
}
