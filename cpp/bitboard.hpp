#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

namespace bitboard
{
// Type definitions

using Board = int; /// All information of the board, i.e. all three bitboard (1e
                   /// player, 2e player, both blayers)

using BBoard = int; /// One bitboard
using Player = int; /// 0, 9, or 18
using Move   = int; /// From 0 to 8
using Eval   = int; /// -1, 0 or 1

enum Players { ONE = 0, TWO = 9, BOTH = 18 }; /// Index of start of bitboards
enum BBoards { EMPTY = 0, FULL = 0x1FF, LENGTH = 9 }; /// Bitboard constants
enum Evals { DRAW = 0, WON = 1 };                     /// Evaluation constants

/// Return other player.
BBoard other(const Player p)
{
	// 0^9 -> 9
	// 9^9 -> 0
	return p^Players::TWO;
}

/**
Return bitboard of board b for player p. No boundary nor type check for
performance reason!
 **/
BBoard get_bboard(const Board b, const Player p)
{
	return (b >> p) & BBoards::FULL;
}
/**
Play move for player and return new board
No boundary nor type check for performance reason!
**/
Board play(const Board b, const Player p, const Move m)
{
	return b | (1 << (m + p)) | (1 << (m + Players::BOTH));
}

/// Is board b legal?
bool is_legal(const Board b)
{
	const BBoard bb1     = get_bboard(b, Players::ONE);
	const BBoard bb2     = get_bboard(b, Players::TWO);
	const BBoard bb_both = get_bboard(b, Players::BOTH);
	return !(bb1 & bb2) && ((bb1 | bb2) == bb2);
}

/// Is board b full?
bool is_full(const Board b) { return false; }

/// Is board b full?
bool is_won(const Board b) { return false; }

/**    
Return score of move (already played in order not to put it onto the stack).
Uses minimax (negamax) algorithm.
**/
Eval minimax(const Board b, const Player p) { return -Evals::WON; }

/**    
Return score of move (already played in order not to put it onto the stack).
Uses alpha beta pruning algorithm.
**/
Eval alphabeta(const Board b, const Player p, const Eval alpha = Evals::WON)
{ 
	return -Evals::WON;
}

/**
Return best move.
If randomize is set, chose randomly between moves with equal score.
 **/
Move best_move(const Board b, const Player p, const bool randomize = true)
{
	return -1;
}

void test()
{
	// Random device
	std::random_device r;
	std::default_random_engine rand_engine(r());
	std::uniform_int_distribution<BBoard> rand_bboard(0, BBoards::FULL);
	// other
	assert(other(Players::ONE) == Players::TWO);
	assert(other(Players::TWO) == Players::ONE);
	assert(other(other(Players::ONE)) == Players::ONE);
	assert(other(other(Players::TWO)) == Players::TWO);

	// get_board
	assert(get_bboard(BBoards::EMPTY, Players::ONE) == BBoards::EMPTY);
	assert(get_bboard(BBoards::EMPTY, Players::TWO) == BBoards::EMPTY);
	assert(get_bboard(BBoards::EMPTY, Players::BOTH) == BBoards::EMPTY);
	for (int b = 0; b < BBoards::FULL + 1; ++b) {
		const BBoard noise1   = rand_bboard(rand_engine);
		const BBoard noise2   = rand_bboard(rand_engine) << 9;
		const BBoard noise_bo = rand_bboard(rand_engine) << 18;
		assert(get_bboard(b | noise2 | noise_bo, Players::ONE) == b);
		assert(get_bboard(noise1 | (b << 9) | noise_bo,
		                  Players::TWO) == b);
		assert(get_bboard(noise1 | noise2 | b << 18,
		                  Players::BOTH) == b);
	}

	// is_legal
	assert(is_legal(BBoards::EMPTY));
	assert(!is_legal(1));

	for (Move m = 0; m <= 9; ++m) {
		assert(get_bboard(play(BBoards::EMPTY, Players::ONE, m),
		                  Players::ONE) ==
		       get_bboard(play(BBoards::EMPTY, Players::TWO, m),
		                  Players::TWO));
	}
}
} // namespace bitboard
