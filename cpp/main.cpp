#include <cctype>
#include <iostream>
#include <string>

int main()
{
	std::cout << "Welcome to TicTacToe\n";
	Engine e;
	std::cout << e << '\n';
	std::string s, user, me;

	while (true) {
		std::cout << "Do you want to start? [y/n] ";
		std::getline(std::cin, s);
		if (s.length() != 1) {
			continue;
		}
		auto c = toupper(s[0]);

		if (c == 'Y') {
			user = "x";
			me   = "o";
			break;
		}
		if (c == 'N') {
			user = "o";
			me   = "x";
			e.play(me);
			std::cout << e << '\n';
			break;
		}
	}
	std::cout << "You play with " << user << '\n';

	while (!e.is_finished()) {
		std::cout << "Your move? [a1-c3] ";
		std::getline(std::cin, s);
		if (!e.play(user, s)) {
			std::cout << "Illegal Move!\n";
			continue;
		}
		e.play(me);
		std::cout << e << '\n';
	}
	if (e.is_won(me)) {
		std::cout << "I win!\n";
	}
	else if (e.is_won(user)) {
		std::cout << "You win!\n";
	}
	else {
		std::cout << "Game drawn!\n";
	}
	return 0;
}
