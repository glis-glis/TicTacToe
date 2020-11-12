/* Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

module main;

void main(string[] args) {
	import std.stdio;
	import std.string;
	import bitboard;
	writeln("Welcome to TicTacToe");
	Board b;
	b.board2str.writeln;
	string reply;
	while (reply != "y" && reply != "n") {
		write("Do you want to start? [y/n] ");
		reply = readln().strip.toLower;
	}
	immutable computer = (reply == "n" ? Player.ONE : Player.TWO);
	immutable human    = computer.other;
	if (computer == Player.ONE) {
		b = b.play(computer, b.bestMove(computer)[0]);
	}
	for (;;) {
		b.board2str.writeln;
		Move m = -1;
		while (!is_move(b, m)) {
		    write("Your move? [a1-c3] ");
		    reply = readln().strip;
		    m = str2move(reply);
		}
		b = b.play(human, m);
		if (b.is_won(human)) {
			b.board2str.writeln;
			writeln("You win!");
			return;
		}
		if (b.is_full) {
			break;
		}
	
		b = b.play(computer, b.bestMove(computer)[0]);
		if (b.is_won(computer)) {
			b.board2str.writeln;
			writeln("I win!");
			return;
		}
		if (b.is_full) {
			break;
		}
	}
	b.board2str.writeln;
	writeln("Game drawn!");
}
