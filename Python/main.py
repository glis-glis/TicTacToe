#!/usr/bin/env python3

# Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@epfl.ch>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

"""
Simple user interface
"""

import argparse
import engine

def repl():
	e = engine.Engine()
	print(e.pretty())

	ans = " "
	while ans not in ["", "y", "n"]:
		ans = input("Do you want to start? [Y/n] ").lower()
	if ans == "n":
		user = "o"
		me   = "x"
		print("You play o")
		e.play_best(me)
		print(e.pretty())
	else:
		user = "x"
		me   = "o"
		print("You play x")

	while not e.is_finished():
		m = input("Your move? [a1-c3] ")
		if not e.play(user, m):
			print("Illegal move")
			continue
		e.play_best(me)
		print(e.pretty())

	if e.is_won(user):    #That shoud not be possible
		print("You win!")
	elif e.is_won(me):
		print("I win!")
	else:
		print("Game draw!")

def test():
	import bitboard
	engine.test()
	bitboard.test()


if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Python TicTacToe implementation')
	parser.add_argument("--test", "-t", action='store_true', default=False,
						help="Run testcases")

	args = parser.parse_args()
	if args.test:
		test()
	else:
		repl()
