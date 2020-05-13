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
/// All information of the board, i.e. all three bitboard (1e player, 2e player,
/// both blayers)
using Board  = uint_fast32_t;
/// One bitboard
using BBoard = uint_fast32_t;
using Player = uint_fast32_t;
using Move   = uint_fast32_t;

enum Players {PLAYER1 = 0, PLAYER2 = 9, BOTH = 18 };
enum BBoards { EMPTY = 0, FULL = 0b111111111, LENGTH = 9 };
enum Evals { DRAW = 0, WON = 1 };

BBoard other(const Player p) { return p^Players::PLAYER2; }
BBoard get_bboard(const Board b, const Player p) {
	return (b >> p) & BBoards::FULL;
}
Board play(const Board b, const Player p, const Move m) {
	return b | (1 << (m + p)) | (1 << (m + Players::BOTH));
}

void test() {
	// other
	assert(other(Players::PLAYER1) == Players::PLAYER2);
	assert(other(Players::PLAYER2) == Players::PLAYER1);
	assert(other(other(Players::PLAYER1)) == Players::PLAYER1);
	assert(other(other(Players::PLAYER2)) == Players::PLAYER2);

	// get_board
	assert(get_bboard(BBoards::EMPTY, Players::PLAYER1) == BBoards::EMPTY);
	assert(get_bboard(BBoards::EMPTY, Players::PLAYER2) == BBoards::EMPTY);
	assert(get_bboard(BBoards::EMPTY, Players::BOTH)   == BBoards::EMPTY);
}
} // namespace bitboard

int main()
{
	bitboard::test();
	return 0;
}

