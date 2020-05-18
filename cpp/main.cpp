#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>

#define ASSERT(e)                                                              \
	{                                                                          \
		if (!(e)) { throw "ASSERT: " __FILE__ " " #e; }                        \
	}

namespace bitboard
{
using Board = int; /// All information of the board, i.e. all three bitboard (1e
                   /// player, 2e player, both blayers)

using BBoard = int; /// One bitboard
using Player = int;
using Move   = int;

enum Players {ONE = 0, TWO = 9, BOTH = 18 };
enum BBoards { EMPTY = 0, FULL = 0b111111111, LENGTH = 9 };
enum Evals { DRAW = 0, WON = 1 };

BBoard other(const Player p) { return p ^ Players::TWO; }

BBoard get_bboard(const Board b, const Player p)
{
	return (b >> p) & BBoards::FULL;
}

Board play(const Board b, const Player p, const Move m)
{
	return b | (1 << (m + p)) | (1 << (m + Players::BOTH));
}

void test() {
	// other
	assert(other(Players::ONE) == Players::TWO);
	assert(other(Players::TWO) == Players::ONE);
	assert(other(other(Players::ONE)) == Players::ONE);
	assert(other(other(Players::TWO)) == Players::TWO);

	// get_board
	assert(get_bboard(BBoards::EMPTY, Players::ONE) == BBoards::EMPTY);
	assert(get_bboard(BBoards::EMPTY, Players::TWO) == BBoards::EMPTY);
	assert(get_bboard(BBoards::EMPTY, Players::BOTH)   == BBoards::EMPTY);
}
} // namespace bitboard

int main()
{
	bitboard::test();
	return 0;
}

