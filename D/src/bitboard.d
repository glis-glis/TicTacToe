/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

module tictactoe.bitboard;

alias BBoard = uint;
alias Board  = uint;
alias Move   = ulong;
alias Eval   = int;

enum Player  {ONE = 0, TWO = 9, BOTH = 18};
enum BBoards {EMPTY = 0, FULL = 0x1FF, LENGTH=9};
enum Evals   {DRAW = 0, WON = 1 << Player.BOTH}

Player other(Player p) {
	return p ^ Player.TWO;
}
unittest {
	Player p;
	assert(p == Player.ONE);
	assert(p.other == Player.TWO);
	assert(p.other.other == Player.ONE);
}

Board board(BBoard b1, BBoard b2) {
	return b1 | (b2 << Player.TWO) | ((b1 | b2) << Player.BOTH);
}
unittest {
	assert(board(0, 0) == BBoards.EMPTY);
}

BBoard bboard(Board b, Player p) {
	return (b >> p) & BBoards.FULL;
}
unittest {
	assert(0.bboard(Player.ONE) == BBoards.EMPTY);
	assert(0.bboard(Player.TWO) == BBoards.EMPTY);
	assert(BBoards.FULL.bboard(Player.ONE) == BBoards.FULL);
	assert(bboard(0.board(BBoards.FULL), Player.TWO) == BBoards.FULL);
}

Board play(Board b, Player p, Move m) {
	return b | (1 << (m + p)) | (1 << (m + Player.BOTH));
}
unittest {
	foreach(m; 0 .. 9) {
		auto b = BBoards.EMPTY;
		assert(bboard(b.play(Player.ONE, m), Player.ONE)
		       == bboard(b.play(Player.TWO, m), Player.TWO));
	}	
}

bool is_full(Board b) {
	return b.bboard(Player.BOTH) == BBoards.FULL;
}
unittest {
	Board b;
	Player p;
	foreach(m; 0 .. 9) {
		b = b.play(p, m);
		p = p.other;
	}
	assert(p == Player.TWO);
	assert(!(b.bboard(Player.ONE) == BBoards.FULL));
	assert(!(b.bboard(Player.TWO) == BBoards.FULL));
	assert(b.is_full);
}

bool is_won(Board b, Player p) {
	import std.algorithm;
	immutable bb = b.bboard(p);
	enum WINS = [
	        0b000000111, 0b000111000, 0b111000000, 0b100100100,
	        0b010010010, 0b001001001, 0b100010001, 0b001010100];
	return WINS.any!(w => (w & bb) == w);
}
unittest {
	Board b;
	Player p;
	assert(b.play(p, 0).play(p, 1).play(p, 2).is_won(p));
}

Eval minimax(Board b, Player p) {
	import core.bitop;
	import std.algorithm;

	if (b.is_won(p)) return Evals.WON;
	if (b.is_full) return Evals.DRAW;

	immutable o     = p.other;
	immutable moves = ~b.bboard(Player.BOTH);
	Eval      e     = Evals.WON;

	foreach (m; BitRange(cast(size_t*)&moves, 9)) {
		e = min(e, -minimax(b.play(o, m), o));	
		if (e == -Evals.WON) break;
	}
	return e;
}
unittest {
	Board b;
	Player p;
	assert(b.minimax(p) == Evals.DRAW);
	b = b.play(p, 0);
	b = b.play(p, 1);
	assert(b.minimax(p) == Evals.WON);
	assert(b.minimax(p.other) == -Evals.WON);
}

Eval alphabeta(Board b, Player p, Eval alpha = -Evals.WON) {
	import core.bitop;
	import std.algorithm;

	if (b.is_won(p)) return Evals.WON;
	if (b.is_full) return Evals.DRAW;

	immutable o     = p.other;
	immutable moves = ~b.bboard(Player.BOTH);
	Eval      beta  = Evals.WON;

	foreach (m; BitRange(cast(size_t*)&moves, 9)) {
		beta = min(beta, -alphabeta(b.play(o, m), o, -beta));	
		if (beta <= alpha) break;
	}
	return beta/2;
}
unittest {
	Board b;
	Player p;
	assert(b.alphabeta(p) == Evals.DRAW);
	b = b.play(p, 0);
	assert(b.alphabeta(p) == Evals.DRAW);
	b = b.play(p, 1);
	assert(b.alphabeta(p) > Evals.DRAW);
	assert(b.alphabeta(p.other) < -Evals.DRAW);
}

auto bestMove(Board b, Player p) {
	import core.bitop;
	import std.typecons;

	Eval      e     = -2 * Evals.WON;
	Move      m     = -1;
	immutable moves = ~b.bboard(Player.BOTH);
	foreach (mi; BitRange(cast(size_t*)&moves, 9)) {
		Eval ei = alphabeta(b.play(p, mi), p);
		if (ei > e) {
			e = ei;
			m = mi;
		}
	}
	return tuple(m, e);
}
unittest {
	import std.typecons;

	Board b;
	assert(b.bestMove(Player.ONE) == tuple(0, Evals.DRAW));	
	b = b.play(Player.ONE, 0);
	assert(b.bestMove(Player.ONE)[1] > Evals.DRAW);	
	assert(b.bestMove(Player.TWO)[1] == Evals.DRAW);	
	b = b.play(Player.ONE, 1);
	assert(b.bestMove(Player.ONE)[1] > Evals.DRAW);	
	assert(b.bestMove(Player.TWO)[1] < Evals.DRAW);	
}
