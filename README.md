# batamiaou

Battle game simulator

## Installation

```shell
pip install batamiaou
```

## Usage

```python
from batamiaou import BatamiaouGame

# create a new game instance
game = BatamiaouGame()
# run a whole game (it stops when a player has no more cards)
rounds = game.run()
# reset the game
game.reset()
# run a single round and returns 1 or 2 (the winning player)
round_winner = game.play()
```


## Deck

A `BatamiaouGame` embeds a `deck` attribute that an array of bytes of size 36 (`char[36]`), that represents the 36 cards of the game. 

The value of the i-th card is `(i % 6) + 1` (value between 1 and 6).

The elements of deck are used as follows:

| Content of `deck[i]` | Value | Meaning                                                             |
| -------------------- | ----- | ------------------------------------------------------------------- |
| `0b0001`             | `1`   | The card `i` is the player 1 main deck                              |
| `0b0010`             | `2`   | The card `i` is the player 2 main deck                              |
| `0b0101`             | `5`   | The card `i` is the player 1 discard deck                           |
| `0b0110`             | `6`   | The card `i` is the player 1 discard deck                           |
| `0b0000`             | `0`   | The card `i` is currently involved in a battle (generally not seen) |


This deck can be read at any time between hands.

```python
# deck is a `bytes` object of length 36
deck = game.deck()
# to get the integers
values = [v for v in map(int, deck)]
```

You can also use `numpy` to ease further deck analysis:

```python
import numpy as np

deck = np.frombuffer(game.deck(), dtype=np.uint8)
```