#ifndef STATE_H
#define STATE_H

enum class State {
    NORMAL,
    JAIL,
    JAIL_LAST_ROUND
};

// which deck a card came from, so a held "get out of jail free" card
// can be returned to the pile it was drawn from
enum class Deck_Type {
    CHANCE,
    COMMUNITY_CHEST
};

// after landing on a tile the tile will process its own state
enum class Tile_State {
    UNOWNED,
    OWN_PROPERTY,
    OWNED_STREET,
    OWNED_RAILROAD,
    OWNED_UTILITY,
    MORTGAGED,
    HYBRID,
    DRAW_CC,
    DRAW_CHANCE,
    TAX,
    FREE,
    FREE_POT,
    TO_JAIL,
    GO
};

// which asset menu the player is looking at
enum class Manage_Assets_Menu {
    ROOT,
    BUILD,
    SELL,
    MORTGAGE,
    UNMORTGAGE,
    EXIT
};

enum class Manage_Assets_State {
    BUILD_SUCCESS,
    BUILD_FAIL_NO_MONEY,
    BUILD_FAIL_NOT_EQUAL_BUILDINGS,
    BUILD_FAIL_MAX_BUILDINGS,
    BUILD_FAIL_MORTGAGED,
    BUILD_FAIL_NO_HOUSES_LEFT,
    BUILD_FAIL_NO_HOTELS_LEFT,
    SELL_SUCCESS,
    SELL_FAIL_NOT_EQUAL_BUILDINGS,
    SELL_FAIL_NO_BUILDINGS,
    SELL_FAIL_NO_HOUSES_LEFT,
    MORTGAGE_SUCCESS,
    MORTGAGE_FAIL_HAS_BUILDINGS,
    UNMORTGAGE_SUCCESS,
    UNMORTGAGE_FAIL_NO_MONEY,
    NULL_STATE
};

enum class Game_State {
    PASSED_GO_REWARD,
    LANDED_ON_GO_REWARD,
    NO_REWARD,
    MANAGE_ASSETS,
    TRADE,
    RESET_STATE,
    PAID_JAIL_FINE,
    FORCED_JAIL_FINE,
    STAYED_IN_JAIL,
    ROLLED_DOUBLE,
    THIRD_DOUBLE_TO_JAIL
};

// how a debt ended once the player had no more cash to give
enum class Payment_State {
    PAID,
    SHORT,
    BANKRUPT_TO_CREDITOR,
    BANKRUPT_TO_BANK
};

enum class Trade_State {
    SUCCESS,
    DECLINED,
    CANCELLED,
    FAIL_NO_FUNDS,
    FAIL_HAS_BUILDINGS,
    FAIL_NOTHING_OFFERED
};

// after the player reaction/action to the tile the player will get a end turn state where they are told what happened : you bought property {continue} and swapped to next turn etc...
enum class End_Turn_State {
    BOUGHT_PROPERTY,
    PASSED_ON_PROPERTY,
    NO_MONEY_TO_BUY,
    AUCTION_WON,
    AUCTION_NO_BIDS,
    PAYED_RENT,
    PAYED_TAX,
    FREE_POT,
    TO_JAIL,
    CONTINUE,
    SHORT_ON_CASH,
    BANKRUPT
};


#endif
