/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

module bitboard;

alias BBoard = uint;  /// One bitboard
alias Board  = uint;  /// All three bitboard
alias Move   = ulong; /// From 0 to 8
alias Eval   = int;   /// -(1 << 18) to (1 << 18)

enum Player  {ONE = 0, TWO = 9, BOTH = 18};
enum BBoards {EMPTY = 0, FULL = 0x1FF, LENGTH=9};
enum Evals   {DRAW = 0, WON = 1 << Player.BOTH}

/// Return other player.
Player other(Player p) {
	return p ^ Player.TWO;
}
///
unittest {
	Player p;
	assert(p == Player.ONE);
	assert(p.other == Player.TWO);
	assert(p.other.other == Player.ONE);
}

/// Return board of bitboards b1 and b2.
Board board(BBoard b1, BBoard b2) {
	return b1 | (b2 << Player.TWO) | ((b1 | b2) << Player.BOTH);
}
///
unittest {
	assert(board(0, 0) == BBoards.EMPTY);
}

/// Return bitboard of board b for player p. 
BBoard bboard(Board b, Player p) {
	return (b >> p) & BBoards.FULL;
}
///
unittest {
	assert(0.bboard(Player.ONE) == BBoards.EMPTY);
	assert(0.bboard(Player.TWO) == BBoards.EMPTY);
	assert(BBoards.FULL.bboard(Player.ONE) == BBoards.FULL);
	assert(bboard(0.board(BBoards.FULL), Player.TWO) == BBoards.FULL);
}

/// Play move for player and return new board
Board play(Board b, Player p, Move m) {
	return b | (1 << (m + p)) | (1 << (m + Player.BOTH));
}
///
unittest {
	foreach(m; 0 .. 9) {
		auto b = BBoards.EMPTY;
		assert(bboard(b.play(Player.ONE, m), Player.ONE)
		       == bboard(b.play(Player.TWO, m), Player.TWO));
	}	
}

/// Is board b full?
bool is_full(Board b) {
	return b.bboard(Player.BOTH) == BBoards.FULL;
}
///
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

/// Is board b won?
bool is_won(Board b, Player p) {
	import std.algorithm;
	immutable bb = b.bboard(p);
	enum WINS = [
	        0b000000111, 0b000111000, 0b111000000, 0b100100100,
	        0b010010010, 0b001001001, 0b100010001, 0b001010100];
	return WINS.any!(w => (w & bb) == w);
}
///
unittest {
	Board b;
	Player p;
	assert(b.play(p, 0).play(p, 1).play(p, 2).is_won(p));
}

/// Is the move legal?
bool is_move(Board b, Move m) {
	return (m < 9) && (b.play(Player.BOTH, m) != b);
}

Move str2move(string s) {
	import std.regex;
	import std.ascii;
	auto ctr = ctRegex!(`^[a-cA-C][1-3]$`);
	if (s.matchFirst(ctr).empty) return cast(Move)-1;
		
	return (s[1] - '1')*3 + (s[0].toLower - 'a');
}
///
unittest {
	foreach (char c; "abc") {
		foreach (char n; "123") {
			string m = "" ~ c ~ n;
			assert(0.is_move(str2move(m)));
		}	
	}
	assert(!0.is_move(str2move("d1")));
}

string board2str(Board b) {
	import std.conv;
	immutable one = b.bboard(Player.ONE);
	immutable two = b.bboard(Player.TWO);
	string s = "";
	for (int i = 2; i >= 0; --i) {
		s ~= "" ~ cast(char)('1' + i) ~ '|';
		for (int j = 0; j < 3; ++j) {
			immutable ij = i*3 + j;
			immutable bij = 1 << ij;
			if (one & bij) {
				s ~= 'x';
			}
			else if (two & bij) {
				s ~= 'o';
			}
			else {
				s ~= '.';
			}
		}
		s ~= " |\n";
	}
	s ~= "  ---\n |abc|";
	return s;
}

/**    
Return score of move (already played in order not to put it onto the stack).
Uses minimax (negamax) algorithm.*/
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
///
unittest {
	Board b;
	Player p;
	assert(b.minimax(p) == Evals.DRAW);
	b = b.play(p, 0);
	b = b.play(p, 1);
	assert(b.minimax(p) == Evals.WON);
	assert(b.minimax(p.other) == -Evals.WON);
}

/**    
Return score of move (already played in order not to put it onto the stack).
Uses alpha beta pruning algorithm.*/
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
///
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

/**
Return best move.
If randomize is set, chose randomly between moves with equal score.*/
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
///
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
