# Monopoly

Monopoly for the terminal, written in C++. The whole board is loaded from a text file, and every
tile decides its own behaviour instead of the rules living in one big switch.

2 to 4 players, hot seat.

## Build and run

```
make
make run
```

Run it from the project root so it can find `src/assets/default-board.txt`.

```
make test
```

runs the rules tests. No framework, no dependencies, just a file of assertions.

## Layout

Headers are in `src/public`, implementations in `src/private`.

| | |
| --- | --- |
| `Board` | reads the board file into 40 tiles and indexes the colour sets |
| `Tile` and its subclasses | each tile answers `Action(Player*)` with a `Tile_State` |
| `GameContext` | owns the board, players, dice and card decks, and does all the state changes |
| `Game` | the rules, dispatched off the state enums in `State.h` |
| `UIController` | draws the board and handles all the input |
| `main.cpp` | the turn loop |

`State.h` is the place to start reading. The whole thing is a set of small state machines and
every enum in that file is the handoff between two layers.

## The board file

One line per tile. First letter is the tile type, then that type's data, then an ANSI colour.
Changing a price or recolouring the board is a text edit.

```
s "Mediterranean_Avenue" MA "Brown" 60 30 33 50 2 4 10 30 90 160 250 <esc>[38;2;119;60;0m
r "Reading_Railroad" RR "Railroad" 200 100 110 <esc>[38;2;39;34;36m
```

| Letter | Tile |
| --- | --- |
| `g` | GO |
| `s` | street |
| `r` | railroad |
| `u` | utility |
| `d` | Chance / Community Chest (`0` = Chance, `1` = Community Chest) |
| `t` | tax |
| `h` | Jail / Just Visiting |
| `j` | Go To Jail |
| `f` | Free Parking |

## What's in

Buying, rent, doubled rent on a full colour set, railroad and utility rent scaling, houses and
hotels with the even build and even sell rules, the bank's 32 houses and 12 hotels, mortgaging,
trading, auctions, Chance and Community Chest, jail with all three ways out, doubles and three
doubles sending you to jail, bankruptcy with liquidation, and a Free Parking jackpot house rule
you can turn on at the start.

Every debt in the game goes through the same path, so there is exactly one place that decides
whether a player is out.

See TODO.md for what I still want to add.
