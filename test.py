# /// script
# dependencies = [
#   "hypothesis",
#   "numpy",
# ]
# ///

import unittest
from ctypes import c_uint8

import numpy as np
from hypothesis import given
from hypothesis import strategies as st

from batamiaou import (
    DISCARD_PILE,
    PLAYER_1,
    PLAYER_2,
    SIZE,
    BatamiaouGame,
)

DECK_SIZE = SIZE
DISCARDED = DISCARD_PILE


class Testbasics(unittest.TestCase):
    @given(st.integers())
    def test_init(self, seed: int):
        game = BatamiaouGame(seed=seed)
        deck = game.deck()
        self.assertEqual(len(deck), DECK_SIZE)  # 36 cards in the deck
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
    def test_mask_deck(self, seed: int):
        game = BatamiaouGame(seed=seed)
        self.assertEqual(game.mask_deck(PLAYER_1), SIZE // 2)
        self.assertEqual(game.mask_deck(PLAYER_2), SIZE // 2)

    @given(st.integers())
    def test_play(self, seed: int):
        game = BatamiaouGame(seed=seed)
        initial_deck = np.frombuffer(game.deck(), dtype=c_uint8)
        game.play()
        final_deck = np.frombuffer(game.deck(), dtype=c_uint8)
        self.assertFalse(np.array_equal(initial_deck, final_deck))  # deck has changed
        self.assertGreater((final_deck & DISCARDED).sum(), 0)  # cards have been played
        self.assertEqual(
            (final_deck & DISCARDED).sum() % 2, 0
        )  # even number of cards played
        self.assertEqual(
            (final_deck == 1).sum(),
            (final_deck == 2).sum(),
        )  # players have same number of REMAINING cards
        self.assertNotEqual(
            (final_deck == 5).sum(),
            (final_deck == 6).sum(),
        )  # only one player has won cards

    @given(st.integers())
    def test_run(self, seed: int):
        game = BatamiaouGame(seed=seed)
        rounds = game.run()
        deck = np.frombuffer(game.deck(), dtype=c_uint8)
        self.assertGreaterEqual(rounds, 1)
        self.assertTrue(
            (deck & 1).sum() == 0 or (deck & 2).sum() == 0
        )  # one player has lost

    @given(st.integers())
    def test_reset(self, seed: int):
        game = BatamiaouGame(seed=seed)
        deck = game.deck()
        game.run()
        game.reset(seed=seed)
        self.assertEqual(game.deck(), deck)


if __name__ == "__main__":
    unittest.main()
