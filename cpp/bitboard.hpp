/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <random>
#include <vector>
#include <regex>

namespace tictactoe
{
namespace bitboard
{
// Type definitions

using BBoard = unsigned int; /// One bitboard
using Board  = unsigned int; /// All three bitboard
using Player = int; /// 0, 9, or 18
using Move   = int; /// From 0 to 8, however, the <bit> functions return signed
using Eval   = int; /// -1, 0 or 1

/// Index of start of bitboards
enum Players : Player {
	ONE  = 0,
	TWO  = 9,
	BOTH = 18
};

/// Bitboard constants
enum BBoards : BBoard {
	EMPTY  = 0,
	FULL   = 0x1FF,
	LENGTH = 9
};
/// Evaluation constants
enum Evals : Eval { DRAW = 0, WON = 1 };

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

bool is_move(const Board b, const Move m)
{
	return static_cast<unsigned int>(m) < 9 &&
	       (play(b, Players::BOTH, m) != b);
}

/// Find first unset bit
Move find_first(const BBoard bb) {
	return std::countr_one(bb);
}

/// Find next unset bit after m
Move find_next(const BBoard bb, const Move m) {
	const Move next = m + 1;
	return next + std::countr_one(bb >> next);
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
	const Player o = other(p);
	// Mutable:
	Eval   e  = Evals::WON;
	BBoard bb = bboard(b, Players::BOTH);
	for (Move m = find_first(bb); m < Players::TWO && e != -Evals::WON;
	     bb ^= (1 << m), m = find_first(bb)) {
		e = std::min(e, -minimax(play(b, o, m), o));
	}
	return e;
}

/**    
Return score of move (already played in order not to put it onto the stack).
Uses alpha beta pruning algorithm.
**/
Eval alphabeta(const Board b, const Player p, const Eval alpha = -Evals::WON)
{ 
	if (is_won(b, p)) {
		return Evals::WON;
	}
	if (is_full(b)) {
		return Evals::DRAW;
	}
	const Player o = other(p);
	// Mutable:
	Eval   beta = Evals::WON;
	BBoard bb   = bboard(b, Players::BOTH);
	for (Move m = find_first(bb); m < Players::TWO && beta > alpha;
	     bb ^= (1 << m), m = find_first(bb)) {
		beta = std::min(beta, -alphabeta(play(b, o, m), o, -beta));
	}
	return beta;
}

/**
Return best move.
If randomize is set, chose randomly between moves with equal score.
 **/
std::pair<Move, Eval> best_move(const Board b, const Player p,
                                const bool randomize = true)
{

	std::random_device                    r;
	std::default_random_engine            rand_engine(r());
	std::bernoulli_distribution           rand_bool;

	// Mutable:
	Eval   ev = -2 * Evals::WON;
	Move   mo = -1;
	BBoard bb = bboard(b, Players::BOTH);
	for (Move m = find_first(bb); m < Players::TWO;
	     bb ^= (1 << m), m = find_first(bb)) {
		//for (Move m = 0; m < BBoards::LENGTH; ++m) {
		Eval e = alphabeta(play(b, p, m), p);
		if (e > ev) {
			ev = e;
			mo = m;
		}
		else if (randomize && e == ev && rand_bool(rand_engine)) {
			ev = e;
			mo = m;
		}
	}
	return std::make_pair(mo, ev);
}

std::optional<Board> str2board(const std::string &s)
{
	std::regex re("^[.xoXO]{9}$");
	if (!std::regex_match(s, re)) {
		return {};
	}
	Board b = BBoards::EMPTY;
	for (size_t i = 0; i < BBoards::LENGTH; ++i) {
		const auto c = toupper(s[i]);
		if (c == 'X') {
			b = play(b, Players::ONE, Move(i));
		}
		else if (c == 'O') {
			b = play(b, Players::TWO, Move(i));
		}
	}

	return b;
}

std::optional<Move> str2move(const std::string &s)
{
	std::regex re("^[a-cA-C][1-3]$");
	if (!std::regex_match(s, re)) {
		return {};
	}
	return int(s[1] - 1)*3 + (toupper(s[0]) - 'A');
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
	for (BBoard bb = BBoards::EMPTY; bb < BBoards::FULL + 1; ++bb) {
		const BBoard noise1 = rand_bboard(rand_engine);
		const BBoard noise2 = rand_bboard(rand_engine);
		const Board b1 = board(bb, noise2);
		const Board b2 = board(noise1, bb);
		assert(bboard(b1, Players::ONE) == bb);
		assert(bboard(b2, Players::TWO) == bb);
	}

	// is_legal, is_move
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
		assert(is_move(b, m));
		assert(!is_move(b, m-1));
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

	// alphabeta
	b = play(BBoards::EMPTY, Players::ONE, 0);
	assert(alphabeta(b, Players::ONE) == Evals::DRAW);
	b = play(b, Players::ONE, 1);
	assert(alphabeta(b, Players::ONE) == Evals::WON);
	assert(alphabeta(b, Players::TWO) == -Evals::WON);

	// best_move
	assert(best_move(BBoards::EMPTY, Players::ONE, false) ==
		std::make_pair(Move(0), Eval(0)));
	assert(best_move(BBoards::EMPTY, Players::ONE).second == Evals::DRAW);
	// b.one = 0b110000000 and is won whoever plays
	assert(best_move(b, Players::ONE).second == Evals::WON);
	assert(best_move(b, Players::TWO).second == -Evals::WON);
}
} // namespace bitboard
} // namespace tictactoe
