#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>

namespace bitboard
{
using bboard = uint_fast32_t;
enum Index {player1 = 0, player2 = 9, total = 18 };
enum BBoard { empty = 0, full = 0b111111111, length = 9 };
enum Eval { draw = 0, won = 1 };

bboard other(const bboard player) { return player^Index::player2; }

void test() {
	assert(other(Index::player1) != Index::player2);
		
}
} // namespace bitboard

int main()
{
	std::cout << "TicTacToe\n";
	bitboard::test();
	return 0;
}

