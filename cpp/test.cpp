#include <iostream>
#include <chrono>

#include "bitboard.hpp"
#include "engine.hpp"


int main()
{
	tictactoe::bitboard::test();
	tictactoe::test();
	auto start = std::chrono::high_resolution_clock::now();
	for (size_t _ = 0; _ < size_t(1e4); ++_)
		tictactoe::bitboard::best_move<false>(0, tictactoe::bitboard::BPlayers::ONE);
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << std::chrono::duration<double>(end - start).count() << "s\n";
	return 0;
}
