# TODO

Stuff I still want to add.

## Rules I skipped or simplified
- Auctions are one round of sealed bids right now. Make it a proper open auction where people
  keep raising until only one bidder is left.
- Income Tax is a flat $200. Should be a choice between $200 and 10% of net worth.
- If you inherit a mortgaged property from someone going bankrupt you don't pay the 10% interest.
  Trades and bank auctions already charge it, so make bankruptcy do the same.
- Going bankrupt on the "pay each player" card hands everything to the bank. Should split it
  between the players you actually owed.

## Engine
- Save and load a game. Everything lives in GameContext so it should mostly be serialising that.
- Make the dice and the card draws injectable so I can script a whole turn in a test instead of
  only testing the rules directly.
- Move all the money numbers into one place. Right now rents and prices are in the board file but
  GO bonus, jail fine, starting balance and the house/hotel counts are constants in GameContext.
- Proper log events instead of every card returning a finished sentence. Would let me format money
  and names in one place and write tests against the output.

## Board
- More board files. The loader already handles any 40 tile layout, just need to write them.
- Let the player pick which board file to use at startup.

## UI
- A no-colour mode for terminals that don't do ANSI.
- Show what a property actually costs to build on before you pick it.
- Redraw less. Right now every prompt clears the screen and redraws the whole board.

## Maybe
- A simple bot player so you can play alone.
- Track stats across a game: how much rent each player collected, most landed on tile, etc.
