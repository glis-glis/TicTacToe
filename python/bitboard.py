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
import re

class Players(IntEnum):
    """Index of start of bitboards"""
    ONE  = 0
    TWO  = 9
    BOTH = 18

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
    """Return other player"""
#0^9 -> 9
    #9^9 -> 0
    return player^Players.TWO

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
    return board | (1 << (move + player)) | (1 << (move + Players.BOTH))

def is_legal(board):
    """Is board legal?"""
    bb0 = bboard(board, Players.ONE)
    bb1 = bboard(board, Players.TWO)
    bb2 = bboard(board, Players.BOTH)
    return (not bb0 & bb1) and ((bb0 | bb1) == bb2)

def is_full(board):
    """Is board full?"""
    return (bboard(board, Players.BOTH)) == BBoard.FULL

WINS = [0b000000111, 0b000111000, 0b111000000,
                   0b100100100, 0b010010010, 0b001001001,
                   0b100010001, 0b001010100]
def is_won(board, player):
    """Is position won?"""
    bb = bboard(board, player)
    return any([bb & bw == bw for bw in WINS])

def moves(board):
    """Return all possible moves"""
    bb = bboard(board, Players.BOTH)
    for i in range(BBoard.LENGTH):
        if not (bb >> i) & 1:
            yield i

def minimax(board, player):
    """
    Return score of move.
    Uses minimax (negamax) algorithm.
    """
    if is_won(board, player):
        return Eval.WON
    if is_full(board):
        return Eval.DRAW

    sc = Eval.WON
    o  = other(player)
    for m in moves(board):
        sc = min(sc, -minimax(play(board, o, m), o))
        if sc == -Eval.WON:
            break
    return sc

def alphabeta(board, player, alpha = -Eval.WON):
    """
    Return score of move.
    Uses alpha beta pruning algorithm.
    """
    if is_won(board, player):
        return Eval.WON
    if is_full(board):
        return Eval.DRAW

    beta = Eval.WON
    o  = other(player)
    for m in moves(board):
        beta = min(beta, -alphabeta(play(board, o, m), o, -beta))
        if beta <= alpha:
            break
    return beta

score = minimax
score = alphabeta

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
        elif randomize and sci == sc and getrandbits(1):
            sc = sci
            m  = mi
    return m, sc


def str2move(smove):
    """
    Parse coordinates to move (0-8).
    Return -1 if illegal move.
    """

    r = re.compile(r"^[a-cA-C][1-3]$")
    if not r.match(smove):
        return -1

    return (int(smove[1]) - 1)*3 + ord(smove[0].lower()) - ord('a')

    #smove = smove.lower()
    #if (not smove
    #    or (len(smove) != 2)
    #    or smove[0] not in "abc"
    #    or smove[1] not in "123"):
    #    return -1

    #return (int(smove[1]) - 1)*3 + ord(smove[0]) - ord('a')

def move2str(move):
    """
    Parse move (0-8) to coordinates.
    Return "" if illegal move.
    """
    if not 0 <= move <= 8:
        return ""

    i = move%3
    j = move//3
    return "".join(["abc"[i], "123"[j]])

def str2board(sboard):
    """
    Parse string to move. Return -1 if illegal board
    '.' -> empty
    'x' -> player 1
    'o' -> player 2
    """

    r      = re.compile("^[.xoXO]{9}$")
    if not r.match(sboard):
        return -1

    sboard = sboard.lower()
    board = 0
    for i, s in enumerate(sboard):
        if s == "x":
            board = play(board, Players.ONE, i)
        elif s == "o":
            board = play(board, Players.TWO, i)

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
    white = bboard(board, Players.ONE)
    black = bboard(board, Players.TWO)

    for i in range(BBoard.LENGTH):
        bi = 1 << i
        if white & bi:
            s[i] = "x"
        elif black & bi:
            s[i] = "o"

    return "".join(s)

def str2player(splayer):
    """Parse string to player"""

    r = re.compile("^[xoXO12]$")

    if not r.match(splayer):
        return -1

    splayer = splayer.lower()
    if splayer in ["1", "x"]:
        return Players.ONE
    if splayer in ["2", "o"]:
        return Players.TWO

    return -1

def test():
    """Test cases"""

    # other
    assert other(Players.ONE) == Players.TWO
    assert other(Players.TWO) == Players.ONE

    assert other(other(Players.ONE)) == Players.ONE
    assert other(other(Players.TWO)) == Players.TWO

    # white
    for i in range(0b111111111+1):
        noise = randint(0, 1 << 12) << BBoard.LENGTH
        assert bboard(i | noise, Players.ONE) == i

    assert bboard(0b111111111+1, Players.ONE) != 0b111111111+1

    # black
    for i in range(0b111111111+1):
        noise1 = randint(0, (1 << 9) - 1)
        noise2 = randint(0, 1 << 13) << 18
        assert bboard((i << 9) | noise1 | noise2, Players.TWO) == i

    assert bboard(1, Players.TWO) != 1

    # white + black
    for w in range(0b111111111+1):
        for b in range(0b111111111+1):
            noise = randint(0, 1 << 13) << 18
            board = w | (b << 9) | (noise << 18)

            assert bboard(board, Players.ONE) == w
            assert bboard(board, Players.TWO) == b

    # is_legal
    assert is_legal(BBoard.EMPTY)
    assert not is_legal(1)

    # play, is_legal
    for m in range(9):
        assert (bboard(play(BBoard.EMPTY, Players.ONE, m), Players.ONE)
                == bboard(play(0, Players.TWO, m), Players.TWO))
    for w in range(9):
        bs = list(range(9))
        bs.remove(w)
        for b in bs:
            bi = play(play(0, Players.ONE, w), Players.TWO, b)
            assert is_legal(bi)
            assert bboard(bi, Players.ONE) == 1 << w
            assert bboard(bi, Players.TWO) == 1 << b

    # is_full
    assert is_full(0b111111111111111111000000000)
    assert is_full(0b111111111000000000111111111)
    assert not is_full(BBoard.EMPTY)

    # is_won
    assert not is_won(BBoard.EMPTY, Players.ONE)
    assert is_won(0b111000000, Players.ONE)
    assert not is_won(0b111000000, Players.TWO)
    assert is_won(0b111000000 << 9, Players.TWO)

    # moves
    assert list(moves(0)) == list(range(9))
    assert not list(moves(0b111111111000000000111111111))
    assert not list(moves(0b111111111111111111000000000))

    # score
    b = play(BBoard.EMPTY, Players.ONE, 0)
    assert score(b, Players.ONE)     == 0
    b = play(b, Players.ONE, 1)
    assert score(b, Players.ONE)  == 1
    assert score(b, Players.TWO)  == -1

    # best_move
    assert best_move(BBoard.EMPTY, Players.ONE, False) == (0, 0)
    assert best_move(BBoard.EMPTY, Players.ONE)[1]     == 0

    assert best_move(0b11, Players.ONE)[1] == 1
    assert best_move(0b11, Players.TWO)[1] == -1

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

    p = Players.ONE
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
        assert str2player(p1) == Players.ONE
    for p2 in ["2", "o", "O"]:
        assert str2player(p2) == Players.TWO

def test_minimax_perf():
    minimax(BBoard.EMPTY, Players.ONE)

def test_alphabeta_perf():
    alphabeta(BBoard.EMPTY, Players.ONE)
