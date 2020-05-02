# Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@epfl.ch>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

"""
Lowlevel bitboard operations. Use with care as no boundary nor type checks are
performed for performance reasons.
"""

from random import randint, getrandbits
from enum import IntEnum

class Index(IntEnum):
	"""Index of start of bitboards"""
	PLAYER1 = 0
	PLAYER2 = 9
	TOTAL   = 18

class BBoard(IntEnum):
	"""Bitboard constants"""
	EMPTY  = 0
	FULL   = 0b111111111
	LENGTH = 9

class Eval(IntEnum):
	"""Evaluation constants"""
	WON  = 1
	DRAW = 0

def other(player):
	"""
	Return other player.
	0 ^ 9 -> 9
	9 ^ 9 -> 0
	"""
	return player ^ Index.PLAYER2

def bboard(board, index):
	"""
	return bitboard of index.
	index = 0 -> player 1
	index = 1 -> player 2
	index = 2 -> total board
	No boundary nor type check for performance reason!
	"""
	return (board >> index) & BBoard.FULL

def play(board, player, move):
	"""
	play move for player and return new board
	No boundary nor type check for performance reason!
	"""
	return board | (1 << (move + player)) | (1 << (move + Index.TOTAL))

def is_legal(board):
	"""Is board legal?"""
	bb0 = bboard(board, Index.PLAYER1)
	bb1 = bboard(board, Index.PLAYER2)
	bb2 = bboard(board, Index.TOTAL)
	return (not bb0 & bb1) and ((bb0 | bb1) == bb2)

def is_full(board):
	"""Is board full?"""
	return (bboard(board, Index.TOTAL)) == BBoard.FULL

def is_won(board, player):
	"""Is position won?"""
	WINS = [0b000000111, 0b000111000, 0b111000000,
			0b100100100, 0b010010010, 0b001001001,
			0b100010001, 0b001010100]
	return any([bboard(board, player) & bw == bw for bw in WINS])

def moves(board):
	"""Return all possible moves"""
	bb = bboard(board, Index.TOTAL)
	for i in range(BBoard.LENGTH):
		if not (bb >> i) & 1:
			yield i

def score(board, player):
	"""Return score of move."""
	if is_won(board, player):
		return Eval.WON
	if is_full(board):
		return Eval.DRAW

	sc = Eval.WON
	o  = other(player)
	for m in moves(board):
		sc = min(sc, -score(play(board, o, m), o))
		if sc == -Eval.WON:
			break
	return sc

def best_move(board, player, randomize=True):
	"""
	Return best move and score.
	If randomize is set, chose randomly between moves with equal score.
	If board is already won, this leads to strange behaviour.
	If board is full, return is (-1, -2)
	"""
	sc = 2*-Eval.WON
	m  = -1
	for mi in moves(board):
		sci = score(play(board, player, mi), player)
		if sci > sc:
			sc = sci
			m  = mi
		if randomize and sci == sc and getrandbits(1):
			sc = sci
			m  = mi
	return m, sc


def str2move(smove):
	"""
	Parse coordinates to move (0-8).
	Return -1 if illegal move.
	"""
	smove = smove.lower()
	if (not smove
		or (len(smove) != 2)
		or smove[0] not in "abc"
		or smove[1] not in "123"):
		return -1

	return (int(smove[1]) - 1)*3 + ord(smove[0]) - ord('a')

def move2str(move):
	"""
	Parse move (0-8) to coordinates.
	Return "" if illegal move.
	"""
	if not 0 <= move <= 8:
		return ""

	i = move%3
	j = move//3
	return "abc"[i] + "123"[j]

def str2board(sboard):
	"""
	Parse string to move. Return -1 if illegal board
	'.' -> empty
	'x' -> player 1
	'o' -> player 2
	"""
	sboard = sboard.lower()

	if (not sboard
		or len(sboard) != 9
		or set(sboard).difference(set(".ox"))):
		return -1

	board = 0
	for i, s in enumerate(sboard):
		if s == "x":
			board = play(board, Index.PLAYER1, i)
		elif s == "o":
			board = play(board, Index.PLAYER2, i)

	return board

def board2str(board):
	"""
	Parse board to string. Return "" if illegal board
	'.' -> empty
	'x' -> player 1
	'o' -> player 2
	"""
	if not is_legal(board):
		return ""

	s = ["."]*9
	white = bboard(board, Index.PLAYER1)
	black = bboard(board, Index.PLAYER2)

	for i in range(BBoard.LENGTH):
		bi = 1 << i
		if white & bi:
			s[i] = "x"
		elif black & bi:
			s[i] = "o"

	return "".join(s)

def str2player(splayer):
	"""Parse string to player"""
	if len(splayer) > 1:
		return -1

	splayer = splayer.lower()
	if splayer in ["1", "x"]:
		return Index.PLAYER1
	if splayer in ["2", "o"]:
		return Index.PLAYER2

	return -1

def test():
	"""Test cases"""

	# other
	assert other(Index.PLAYER1) == Index.PLAYER2
	assert other(Index.PLAYER2) == Index.PLAYER1

	assert other(other(Index.PLAYER1)) == Index.PLAYER1
	assert other(other(Index.PLAYER2)) == Index.PLAYER2

	# white
	for i in range(0b111111111+1):
		noise = randint(0, 1 << 12) << BBoard.LENGTH
		assert bboard(i | noise, Index.PLAYER1) == i

	assert bboard(0b111111111+1, Index.PLAYER1) != 0b111111111+1

	# black
	for i in range(0b111111111+1):
		noise1 = randint(0, (1 << 9) - 1)
		noise2 = randint(0, 1 << 13) << 18
		assert bboard((i << 9) | noise1 | noise2, Index.PLAYER2) == i

	assert bboard(1, Index.PLAYER2) != 1

	# white + black
	for w in range(0b111111111+1):
		for b in range(0b111111111+1):
			noise = randint(0, 1 << 13) << 18
			board = w | (b << 9) | (noise << 18)

			assert bboard(board, Index.PLAYER1) == w
			assert bboard(board, Index.PLAYER2) == b

	# is_legal
	assert is_legal(BBoard.EMPTY)
	assert not is_legal(1)

	# play, is_legal
	for m in range(9):
		assert (bboard(play(BBoard.EMPTY, Index.PLAYER1, m), Index.PLAYER1)
				== bboard(play(0, Index.PLAYER2, m), Index.PLAYER2))
	for w in range(9):
		bs = list(range(9))
		bs.remove(w)
		for b in bs:
			bi = play(play(0, Index.PLAYER1, w), Index.PLAYER2, b)
			assert is_legal(bi)
			assert bboard(bi, Index.PLAYER1) == 1 << w
			assert bboard(bi, Index.PLAYER2) == 1 << b

	# is_full
	assert is_full(0b111111111111111111000000000)
	assert is_full(0b111111111000000000111111111)
	assert not is_full(BBoard.EMPTY)

	# is_won
	assert not is_won(BBoard.EMPTY, Index.PLAYER1)
	assert is_won(0b111000000, Index.PLAYER1)
	assert not is_won(0b111000000, Index.PLAYER2)
	assert is_won(0b111000000 << 9, Index.PLAYER2)

	# moves
	assert list(moves(0)) == list(range(9))
	assert not list(moves(0b111111111000000000111111111))
	assert not list(moves(0b111111111111111111000000000))

	# score
	b = play(BBoard.EMPTY, Index.PLAYER1, 0)
	assert score(b, Index.PLAYER1)     == 0
	b = play(b, Index.PLAYER1, 1)
	assert score(b, Index.PLAYER1)  == 1
	assert score(b, Index.PLAYER2)  == -1

	# best_move
	assert best_move(BBoard.EMPTY, Index.PLAYER1, False) == (0, 0)
	assert best_move(BBoard.EMPTY, Index.PLAYER1)[1]     == 0

	assert best_move(0b11, Index.PLAYER1)[1] == 1
	assert best_move(0b11, Index.PLAYER2)[1] == -1

	# Str2move
	assert str2move("")    == -1
	assert str2move("1")   == -1
	assert str2move("123") == -1

	assert str2move("12")  == -1
	assert str2move("ab")  == -1
	assert str2move("1a")  == -1
	assert str2move("a1 ") == -1

	assert str2move("a1") == 0
	assert str2move("A1") == 0
	assert str2move("b1") == 1
	assert str2move("a3") == 6
	assert str2move("c3") == 8

	# Move2str
	assert move2str(-1) == ""
	assert move2str(9)  == ""

	assert move2str(0) == "a1"
	assert move2str(2) == "c1"
	assert move2str(7) == "b3"
	assert move2str(8) == "c3"

	# Str2board
	for i in range(8):
		assert str2board(i*".") == -1

	assert str2board(9*".") == 0
	assert str2board(9*"o") == 0b111111111111111111000000000
	assert str2board(9*"x") == 0b111111111000000000111111111
	assert str2board(9*"a") == -1

	assert str2board(".oxo.xx..") == 0b1101110000001010001100100

	#Board2str
	assert board2str(BBoard.EMPTY) == 9*"."
	assert board2str(1)            == ""

	p = Index.PLAYER1
	b = BBoard.EMPTY
	for i in range(9):
		b = play(b, p, i)
		p = other(p)
		assert str2move(move2str(i)) == i
		assert str2board(board2str(b)) == b

	#Str2player
	assert str2player("")   == -1
	assert str2player(" ")  == -1
	assert str2player("xx") == -1
	for p1 in ["1", "x", "X"]:
		assert str2player(p1) == Index.PLAYER1
	for p2 in ["2", "o", "O"]:
		assert str2player(p2) == Index.PLAYER2
