# Copyright (C) 2020 Andreas Füglistaler <andreas.fueglistaler@epfl.ch>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

"""
Engine class, inteface between lowlevel bitboards and string input
"""

import bitboard as bb

class Engine:
    """
    Engine class, inteface between lowlevel bitboards and string input.
    The design choise is to use no exception, everything is handled through
    boolean returns.
    """

    def __init__(self):
        """Create engine with initially empty board"""
        self._board = 0

    def __str__(self):
        """Return string representation of board"""
        return bb.board2str(self._board)

    def pretty(self):
        """Prettey, 2D string of board"""
        sb    = bb.board2str(self._board)
        return "3|%3s|\n2|%3s|\n1|%3s|\n  --- \n  abc "%(sb[6:9], sb[3:6], sb[0:3])

    def set(self, board):
        """
        Set Board to different state.
        Return False if board is not altered.
        """
        b = bb.str2board(board)
        if b == -1:
            return False

        if self._board != b:
            self._board = b
            return True

        return False

    def reset(self):
        """Set Board to initial empty state."""
        self._board = 0

    def is_full(self):
        """Is board full?"""
        return bb.is_full(self._board)

    def is_won(self, player):
        """Is the position won? If player is not legal, return false"""
        p = bb.str2player(player)
        if p < 0:
            return False

        return bb.is_won(self._board, p)

    def is_finished(self):
        """Is game over?"""
        return (bb.is_full(self._board)
                or bb.is_won(self._board, bb.Players.ONE)
                or bb.is_won(self._board, bb.Players.TWO))

    def play(self, player, move):
        """
        Play move for player, update board.
        Return False if board is not altered (illegal player or
        illegal/impossible move)
        """
        if self.is_finished():
            return False

        m = bb.str2move(move)
        if m not in bb.moves(self._board):
            return False

        p = bb.str2player(player)
        if p < 0:
            return False

        b_old       = self._board
        self._board = bb.play(self._board, p, m)

        return b_old != self._board # impossible move

    def play_best(self, player, randomize=True):
        """
        Find and play best move. If randomize is set, chose randomly
        between moves with equal score.
        """
        if self.is_finished():
            return False

        p = bb.str2player(player)
        if p < 0:
            return False

        m = bb.best_move(self._board, p, randomize)[0]
        if m < 0:
            return False

        b_old = self._board
        self._board = bb.play(self._board, p, m)
        return b_old != self._board # Should not happen

def test():
    """Test cases"""
    e = Engine()
    # constructor
    assert str(Engine()) == 9*"."
    assert str(e) == 9*"."

    # set
    assert not e.set("")
    assert not e.set("ox")
    assert not e.set(9*" ") # Same board as before

    assert e.set("x" + 8*".")
    assert str(e) == "x" + 8*"."

    # reset
    e.reset()
    assert str(Engine()) == 9*"." # Again empty board

    # is_full
    e.set("xxxxxxxxx")
    assert e.is_full()
    e.reset()


    # play
    assert not e.play("", "")
    assert not e.play("x", "")
    assert not e.play("", "a1")
    assert str(Engine()) == 9*"." # Still empty board

    assert e.play("x", "a1")
    assert str(e) == "x"+8*"."
    assert not e.play("x", "a1")
    assert not e.play("o", "a1")

    assert e.play("o", "b1")
    assert str(e) == "xo"+7*"."
    assert not e.play("b", "b1")
    e.reset()

    # is_won
    assert e.play("x", "a1")
    assert e.play("x", "b1")
    assert e.play("x", "c1")
    assert e.is_won("x")
    e.reset()

