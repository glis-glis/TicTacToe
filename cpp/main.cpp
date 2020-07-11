/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <cctype>
#include <iostream>
#include <string>

#include "engine.hpp"

int main()
{
	using namespace tictactoe;
	std::cout << "Welcome to TicTacToe\n";
	Engine e;
	std::cout << e << '\n';
	std::string s;
	Player player;

	while (true) {
		std::cout << "Do you want to start? [y/n] ";
		std::getline(std::cin, s);
		if (s.length() != 1) {
			continue;
		}
		auto c = toupper(s[0]);

		if (c == 'Y') {
			player = Player::ONE;
			std::cout << "You play with x\n";
			break;
		}
		if (c == 'N') {
			player = Player::TWO;
			e.play_best(other(player));
			std::cout << e << '\n';
			std::cout << "You play with o\n";
			break;
		}
	}

	while (!e.is_finished()) {
		std::cout << "Your move? [a1-c3] ";
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
	}
	else if (e.is_won(player)) {
		std::cout << "You win!\n";
	}
	else {
		std::cout << "Game drawn!\n";
	}
	return 0;
}
