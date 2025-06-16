# /// script
# dependencies = [
#   "hypothesis",
#   "numpy",
# ]
# ///

import unittest
from ctypes import c_int, sizeof

import numpy as np
from hypothesis import given
from hypothesis import strategies as st

from batamiaou import BatamiaouGame  # type: ignore

DECK_SIZE = 36


class Testbasics(unittest.TestCase):
    @given(st.integers())
    def test_init(self, seed: int):
        game = BatamiaouGame(seed=seed)
        deck = game.deck()
        self.assertEqual(len(deck), sizeof(c_int) * DECK_SIZE)  # 36 cards in the deck
        self.assertEqual(deck.count(b"\x01"), DECK_SIZE // 2)  # player 1
        self.assertEqual(deck.count(b"\x02"), DECK_SIZE // 2)  # player 2

    @given(st.integers())
    def test_seed(self, seed: int):
        game0 = BatamiaouGame(seed=seed)
        deck0 = game0.deck()
        game1 = BatamiaouGame(seed=seed)
        deck1 = game1.deck()
        self.assertEqual(deck0, deck1)

    @given(st.integers())
    def test_reset(self, seed: int):
        game = BatamiaouGame(seed=seed)
        deck = game.deck()
        game.run()
        game.reset(seed=seed)
        self.assertEqual(game.deck(), deck)

    @given(st.integers())
    def test_play(self, seed: int):
        game = BatamiaouGame(seed=seed)
        initial_deck = np.frombuffer(game.deck(), dtype=c_int)
        game.play()
        final_deck = np.frombuffer(game.deck(), dtype=c_int)
        self.assertFalse(np.array_equal(initial_deck, final_deck))  # deck has changed
        self.assertGreater((final_deck < 0).sum(), 0)  # cards have been played
        self.assertEqual((final_deck < 0).sum() % 2, 0)  # even number of cards played
        self.assertEqual(
            (final_deck == 1).sum(),
            (final_deck == 2).sum(),
        )  # players have same number of REMAINING cards
        self.assertNotEqual(
            (final_deck == -1).sum(),
            (final_deck == -2).sum(),
        )  # only one player has won cards

    @given(st.integers())
    def test_run(self, seed: int):
        game = BatamiaouGame(seed=seed)
        rounds = game.run()
        deck = np.frombuffer(game.deck(), dtype=c_int)
        abs_deck = np.abs(deck)
        self.assertGreaterEqual(rounds, 1)
        self.assertTrue(
            (abs_deck == 1).sum() == 0 or (abs_deck == 2).sum() == 0
        )  # one player has lost


if __name__ == "__main__":
    unittest.main()
