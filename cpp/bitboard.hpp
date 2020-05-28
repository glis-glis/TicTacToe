#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

namespace bitboard
{
// Type definitions

using BBoard = int; /// One bitboard
using Board  = int; /// All three bitboard
using Player = int; /// 0, 9, or 18
using Move   = int; /// From 0 to 8
using Eval   = int; /// -1, 0 or 1

enum Players { ONE = 0, TWO = 9, BOTH = 18 }; /// Index of start of bitboards
enum BBoards { EMPTY = 0, FULL = 0x1FF, LENGTH = 9 }; /// Bitboard constants
enum Evals { DRAW = 0, WON = 1 };                     /// Evaluation constants

/// Return other player.
Player other(const Player p)
{
	// 0^9 -> 9
	// 9^9 -> 0
	return p ^ Players::TWO;
}

/**
Return board of bitboards b1 and b2. No boundary nor type check for
performance reason!
 **/
Board board(const BBoard b1, const BBoard b2)
{
	return b1 | (b2 << Players::TWO) | ((b1 | b2) << Players::BOTH);
}

/**
Return bitboard of board b for player p. No boundary nor type check for
performance reason!
 **/
BBoard bboard(const Board b, const Player p)
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
	const BBoard bb1     = bboard(b, Players::ONE);
	const BBoard bb2     = bboard(b, Players::TWO);
	const BBoard bb_both = bboard(b, Players::BOTH);
	return !(bb1 & bb2) && ((bb1 | bb2) == bb_both);
}

/// Is board b full?
bool is_full(const Board b)
{
	return bboard(b, Players::BOTH) == BBoards::FULL;
}

/// Is board b won?
bool is_won(const Board b, const Player p) {
	constexpr std::array<BBoard, 8> WINS = {
	        0b000000111, 0b000111000, 0b111000000, 0b100100100,
	        0b010010010, 0b001001001, 0b100010001, 0b001010100};

	const BBoard bb = bboard(b, p);
	return std::any_of(WINS.begin(), WINS.end(),
	                   [bb](const BBoard w) { return (w & bb) == w; });
}

/**    
Return score of move (already played in order not to put it onto the stack).
Uses minimax (negamax) algorithm.
**/
Eval minimax(const Board b, const Player p) {
	if (is_won(b, p)) {
		return Evals::WON;
	}
	if (is_full(b)) {
		return Evals::DRAW;
	}

	Eval         e  = Evals::WON;
	const Player o  = other(p);
	const BBoard bb = bboard(b, Players::BOTH);
	for (Move m = 0; m < BBoards::LENGTH && e != -Evals::WON; ++m) {
		if ((bb >> m) & 1) {
			continue;	
		}
		e = std::min(e, -minimax(play(b, o, m), o));
	}
	return e;
}

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
	std::random_device                    r;
	std::default_random_engine            rand_engine(r());
	std::uniform_int_distribution<BBoard> rand_bboard(0, BBoards::FULL);

	// other
	assert(other(Players::ONE) == Players::TWO);
	assert(other(Players::TWO) == Players::ONE);
	assert(other(other(Players::ONE)) == Players::ONE);
	assert(other(other(Players::TWO)) == Players::TWO);

	// board
	assert(board(BBoards::EMPTY, BBoards::EMPTY) == BBoards::EMPTY);

	// bboard 
	assert(bboard(BBoards::EMPTY, Players::ONE) == BBoards::EMPTY);
	assert(bboard(BBoards::EMPTY, Players::TWO) == BBoards::EMPTY);
	assert(bboard(BBoards::EMPTY, Players::BOTH) == BBoards::EMPTY);
	for (int b = BBoards::EMPTY; b < BBoards::FULL + 1; ++b) {
		const BBoard noise1 = rand_bboard(rand_engine);
		const BBoard noise2 = rand_bboard(rand_engine);
		const Board b1 = board(b, noise2);
		const Board b2 = board(noise1, b);
		assert(bboard(b1, Players::ONE) == b);
		assert(bboard(b2, Players::TWO) == b);
	}

	// is_legal
	assert(is_legal(board(BBoards::EMPTY, BBoards::EMPTY)));
	assert(is_legal(board(1, BBoards::EMPTY)));
	assert(!is_legal(1));

	Board  b = BBoards::EMPTY;
	Player p = Players::ONE;
	for (Move m = 0; m < 9; ++m) {
		assert(bboard(play(BBoards::EMPTY, Players::ONE, m),
		              Players::ONE) ==
		       bboard(play(BBoards::EMPTY, Players::TWO, m),
		              Players::TWO));
		b = play(b, p, m);
		p = other(p);
		assert(is_legal(b));
	}

	// is_full
	assert(is_full(BBoards::FULL | (BBoards::FULL << Players::BOTH)));
	assert(is_full((BBoards::FULL << Players::TWO) |
	               (BBoards::FULL << Players::BOTH)));
	assert(!is_full(BBoards::EMPTY));

	// is_won
	assert(!is_won(BBoards::EMPTY, Players::ONE));

	// minimax
	b = play(BBoards::EMPTY, Players::ONE, 0);
	assert(minimax(b, Players::ONE) == Evals::DRAW);
	b = play(b, Players::ONE, 1);
	assert(minimax(b, Players::ONE) == Evals::WON);
	assert(minimax(b, Players::TWO) == -Evals::WON);

}
} // namespace bitboard
