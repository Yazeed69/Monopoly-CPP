// Rules regression tests. No framework, no dependencies: build with `make test` from the repo
// root so src/assets/default-board.txt can be found.
//
// Everything here drives the engine through its public API only. award_auction is used as the
// setup primitive for handing a deed to a chosen player, because unlike buy_property it does not
// require that player to be the one whose turn it is.

#include "GameContext.h"
#include "Game.h"

#include <iostream>
#include <string>
#include <vector>

static unsigned int checks = 0;
static unsigned int failures = 0;
static std::string current_test;

static void check(bool condition, const std::string& what, int line) {
    checks++;
    if (!condition) {
        failures++;
        std::cout << "  FAIL " << current_test << " line " << line << ": " << what << std::endl;
    }
}

static void check_equal(long long actual, long long expected, const std::string& what, int line) {
    checks++;
    if (actual != expected) {
        failures++;
        std::cout << "  FAIL " << current_test << " line " << line << ": " << what
                  << " was " << actual << ", expected " << expected << std::endl;
    }
}

#define CHECK(condition) check((condition), #condition, __LINE__)
#define CHECK_EQUAL(actual, expected) check_equal((long long)(actual), (long long)(expected), #actual, __LINE__)

// board indices used by the tests, straight out of src/assets/default-board.txt
static const unsigned int MEDITERRANEAN = 1;
static const unsigned int BALTIC = 3;
static const unsigned int READING_RAILROAD = 5;
static const unsigned int ORIENTAL = 6;
static const unsigned int VERMONT = 8;
static const unsigned int CONNECTICUT = 9;
static const unsigned int PENNSYLVANIA_RAILROAD = 15;
static const unsigned int B_AND_O_RAILROAD = 25;
static const unsigned int ELECTRIC_COMPANY = 12;
static const unsigned int WATER_WORKS = 28;
static const unsigned int SHORT_LINE = 35;
static const unsigned int PARK_PLACE = 37;
static const unsigned int BOARDWALK = 39;

static std::vector<std::string> two_players() {
    return {"One", "Two"};
}

static Street* street_at(GameContext& context, unsigned int index) {
    return dynamic_cast<Street*>(context.get_board()->get_tile(index));
}

static Property* property_at(GameContext& context, unsigned int index) {
    return dynamic_cast<Property*>(context.get_board()->get_tile(index));
}

// hand a deed to a player for free so a scenario can be set up in one line
static void give(GameContext& context, Player* player, unsigned int index) {
    Property* property = property_at(context, index);
    unsigned int before = player->get_balance();
    context.award_auction(player, property, 0);
    player->set_balance(before);
}


static void test_board_loads() {
    GameContext context(two_players(), false);
    CHECK(context.get_board()->is_loaded());
    CHECK(context.get_board()->get_load_error().empty());
    for (unsigned int i = 0; i < Board::board_size; i++) {
        CHECK(context.get_board()->get_tile(i) != nullptr);
    }
    CHECK_EQUAL(context.get_board()->get_color_set_assets("Brown").size(), 2);
    CHECK_EQUAL(context.get_board()->get_color_set_assets("Light Blue").size(), 3);
    // railroads and utilities are not streets, so they carry no colour-set entry
    CHECK_EQUAL(context.get_board()->get_color_set_assets("Railroad").size(), 0);
    CHECK_EQUAL(context.get_board()->get_set_assets("Railroad").size(), 4);
    CHECK_EQUAL(context.get_board()->get_set_assets("Utility").size(), 2);
    // a colour the board never mentions must not throw
    CHECK_EQUAL(context.get_board()->get_color_set_assets("Chartreuse").size(), 0);
}

static void test_full_set_doubles_rent() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];

    CHECK_EQUAL(street_at(context, MEDITERRANEAN)->get_rent(), 2);
    give(context, one, MEDITERRANEAN);
    CHECK_EQUAL(street_at(context, MEDITERRANEAN)->get_rent(), 2);

    give(context, one, BALTIC);
    CHECK_EQUAL(street_at(context, MEDITERRANEAN)->get_rent(), 4);
    CHECK_EQUAL(street_at(context, BALTIC)->get_rent(), 8);

    // breaking the set puts the rents back
    Trade trade;
    trade.proposer = one;
    trade.partner = two;
    trade.offered.push_back(property_at(context, BALTIC));
    trade.cash_offered = 0;
    trade.cash_requested = 0;
    trade.jail_cards_offered = 0;
    trade.jail_cards_requested = 0;
    CHECK(context.execute_trade(trade) == Trade_State::SUCCESS);
    CHECK_EQUAL(street_at(context, MEDITERRANEAN)->get_rent(), 2);
    CHECK_EQUAL(street_at(context, BALTIC)->get_rent(), 4);
}

static void test_even_building() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    one->set_balance(10000);

    Street* mediterranean = street_at(context, MEDITERRANEAN);
    Street* baltic = street_at(context, BALTIC);

    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
    CHECK_EQUAL(mediterranean->get_num_houses(), 1);
    CHECK_EQUAL(mediterranean->get_rent(), 10);
    // second house on the same street while its sibling is empty is not allowed
    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_FAIL_NOT_EQUAL_BUILDINGS);
    CHECK(context.build_house(baltic) == Manage_Assets_State::BUILD_SUCCESS);
    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
    CHECK_EQUAL(mediterranean->get_num_houses(), 2);

    // selling has to come off the taller street first
    CHECK(context.sell_house(baltic) == Manage_Assets_State::SELL_FAIL_NOT_EQUAL_BUILDINGS);
    CHECK(context.sell_house(mediterranean) == Manage_Assets_State::SELL_SUCCESS);
    CHECK_EQUAL(mediterranean->get_num_houses(), 1);
}

static void test_building_costs_and_stock() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    one->set_balance(10000);

    Street* mediterranean = street_at(context, MEDITERRANEAN);
    unsigned int before = one->get_balance();
    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
    CHECK_EQUAL(one->get_balance(), before - mediterranean->get_build_cost());
    CHECK_EQUAL(context.get_houses_available(), GameContext::house_stock - 1);

    before = one->get_balance();
    CHECK(context.sell_house(mediterranean) == Manage_Assets_State::SELL_SUCCESS);
    CHECK_EQUAL(one->get_balance(), before + mediterranean->get_sell_price());
    CHECK_EQUAL(context.get_houses_available(), GameContext::house_stock);
}

static void test_hotels() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    one->set_balance(10000);

    Street* mediterranean = street_at(context, MEDITERRANEAN);
    Street* baltic = street_at(context, BALTIC);
    for (unsigned int i = 0; i < 4; i++) {
        CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
        CHECK(context.build_house(baltic) == Manage_Assets_State::BUILD_SUCCESS);
    }
    CHECK_EQUAL(mediterranean->get_num_houses(), 4);
    CHECK_EQUAL(mediterranean->get_rent(), 160);
    CHECK_EQUAL(context.get_houses_available(), GameContext::house_stock - 8);

    // the fifth building is a hotel: four houses go back in the box, one hotel comes out
    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
    CHECK_EQUAL(mediterranean->get_num_houses(), 5);
    CHECK_EQUAL(mediterranean->get_num_hotels(), 1);
    CHECK_EQUAL(mediterranean->get_rent(), 250);
    CHECK_EQUAL(context.get_houses_available(), GameContext::house_stock - 4);
    CHECK_EQUAL(context.get_hotels_available(), GameContext::hotel_stock - 1);
    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_FAIL_MAX_BUILDINGS);

    // and it breaks back down into four houses
    CHECK(context.sell_house(mediterranean) == Manage_Assets_State::SELL_SUCCESS);
    CHECK_EQUAL(mediterranean->get_num_houses(), 4);
    CHECK_EQUAL(context.get_hotels_available(), GameContext::hotel_stock);
    CHECK_EQUAL(context.get_houses_available(), GameContext::house_stock - 8);
}

static void test_house_stock_runs_out() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    one->set_balance(1000000);
    // every street on the board, so the bank's 32 houses can actually be exhausted
    const unsigned int streets[] = {1, 3, 6, 8, 9, 11, 13, 14, 16, 18, 19, 21, 23, 24, 26, 27, 29, 31, 32, 34, 37, 39};
    for (unsigned int index : streets) {
        give(context, one, index);
    }
    unsigned int built = 0;
    bool ran_out = false;
    for (unsigned int round = 0; round < 4 && !ran_out; round++) {
        for (unsigned int index : streets) {
            Manage_Assets_State result = context.build_house(street_at(context, index));
            if (result == Manage_Assets_State::BUILD_SUCCESS) {
                built++;
            } else if (result == Manage_Assets_State::BUILD_FAIL_NO_HOUSES_LEFT) {
                ran_out = true;
                break;
            }
        }
    }
    CHECK(ran_out);
    CHECK_EQUAL(built, GameContext::house_stock);
    CHECK_EQUAL(context.get_houses_available(), 0);
}

static void test_hotel_stock_runs_out() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    one->set_balance(1000000);
    // one colour set at a time, because taking a whole set to hotels only ever needs
    // twelve houses out of the box at once and hands them all back on the upgrade
    const std::vector<std::vector<unsigned int>> sets = {
        {1, 3}, {6, 8, 9}, {11, 13, 14}, {16, 18, 19}, {21, 23, 24}
    };
    for (const std::vector<unsigned int>& set : sets) {
        for (unsigned int index : set) {
            give(context, one, index);
        }
    }
    unsigned int hotels = 0;
    bool ran_out = false;
    for (const std::vector<unsigned int>& set : sets) {
        for (unsigned int round = 0; round < 5 && !ran_out; round++) {
            for (unsigned int index : set) {
                Manage_Assets_State result = context.build_house(street_at(context, index));
                if (result == Manage_Assets_State::BUILD_FAIL_NO_HOTELS_LEFT) {
                    ran_out = true;
                    break;
                }
                if (result == Manage_Assets_State::BUILD_SUCCESS && street_at(context, index)->get_num_houses() == 5) {
                    hotels++;
                }
            }
        }
        if (ran_out) {
            break;
        }
    }
    CHECK(ran_out);
    CHECK_EQUAL(hotels, GameContext::hotel_stock);
    CHECK_EQUAL(context.get_hotels_available(), 0);
    // and one of them can still come down, which puts a hotel back on the shelf
    for (const std::vector<unsigned int>& set : sets) {
        for (unsigned int index : set) {
            if (street_at(context, index)->get_num_houses() == 5) {
                CHECK(context.sell_house(street_at(context, index)) == Manage_Assets_State::SELL_SUCCESS);
                CHECK_EQUAL(context.get_hotels_available(), 1);
                return;
            }
        }
    }
}

static void test_mortgages() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    one->set_balance(10000);

    Street* mediterranean = street_at(context, MEDITERRANEAN);
    Street* baltic = street_at(context, BALTIC);

    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
    // a set with buildings anywhere on it cannot be mortgaged
    CHECK(context.mortgage_property(baltic) == Manage_Assets_State::MORTGAGE_FAIL_HAS_BUILDINGS);
    CHECK(context.sell_house(mediterranean) == Manage_Assets_State::SELL_SUCCESS);

    unsigned int before = one->get_balance();
    CHECK(context.mortgage_property(baltic) == Manage_Assets_State::MORTGAGE_SUCCESS);
    CHECK_EQUAL(one->get_balance(), before + baltic->get_mortgage_value());
    CHECK(baltic->get_mortgaged());
    CHECK_EQUAL(baltic->get_rent(), 0);
    // the board marks it, and the marker is still two columns wide so the row stays aligned
    CHECK(baltic->get_houses().find("MM") != std::string::npos);
    CHECK_EQUAL(context.rent_due(baltic, two), 0);
    CHECK(baltic->Action(two) == Tile_State::MORTGAGED);
    // and nothing can be built anywhere on that set while it is mortgaged
    CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_FAIL_MORTGAGED);

    before = one->get_balance();
    CHECK(context.unmortgage_property(baltic) == Manage_Assets_State::UNMORTGAGE_SUCCESS);
    CHECK_EQUAL(one->get_balance(), before - baltic->get_unmortgage_value());
    CHECK(!baltic->get_mortgaged());
    CHECK_EQUAL(baltic->get_rent(), 8);
    CHECK(baltic->get_houses().find("MM") == std::string::npos);

    one->set_balance(0);
    CHECK(context.mortgage_property(baltic) == Manage_Assets_State::MORTGAGE_SUCCESS);
    one->set_balance(0);
    CHECK(context.unmortgage_property(baltic) == Manage_Assets_State::UNMORTGAGE_FAIL_NO_MONEY);
    CHECK(baltic->get_mortgaged());
}

static void test_railroad_and_utility_rent() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    Railroad* reading = dynamic_cast<Railroad*>(context.get_board()->get_tile(READING_RAILROAD));

    give(context, one, READING_RAILROAD);
    CHECK_EQUAL(reading->get_rent(one->get_num_railroads()), 25);
    give(context, one, PENNSYLVANIA_RAILROAD);
    CHECK_EQUAL(reading->get_rent(one->get_num_railroads()), 50);
    give(context, one, B_AND_O_RAILROAD);
    CHECK_EQUAL(reading->get_rent(one->get_num_railroads()), 100);
    // owning all four is what used to crash on the colour-set lookup
    give(context, one, SHORT_LINE);
    CHECK_EQUAL(one->get_num_railroads(), 4);
    CHECK_EQUAL(reading->get_rent(one->get_num_railroads()), 200);

    Utility* electric = dynamic_cast<Utility*>(context.get_board()->get_tile(ELECTRIC_COMPANY));
    give(context, one, ELECTRIC_COMPANY);
    CHECK_EQUAL(electric->get_multiplier(one->get_num_utilities()), 4);
    give(context, one, WATER_WORKS);
    CHECK_EQUAL(one->get_num_utilities(), 2);
    CHECK_EQUAL(electric->get_multiplier(one->get_num_utilities()), 10);
    two->set_roll(7);
    CHECK_EQUAL(context.rent_due(electric, two), 70);

    // a mortgaged railroad collects nothing
    CHECK(context.mortgage_property(reading) == Manage_Assets_State::MORTGAGE_SUCCESS);
    CHECK_EQUAL(reading->get_rent(one->get_num_railroads()), 0);
    CHECK(reading->Action(two) == Tile_State::MORTGAGED);
}

static void test_debt_is_paid_to_the_creditor() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    one->set_balance(500);
    two->set_balance(500);

    context.charge(one, two, 120);
    CHECK(context.has_pending_debt());
    CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    CHECK_EQUAL(one->get_balance(), 380);
    CHECK_EQUAL(two->get_balance(), 620);
    CHECK(!context.has_pending_debt());
}

static void test_debt_offers_liquidation_before_bankruptcy() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, BOARDWALK);
    one->set_balance(10);

    // Boardwalk mortgages for 200, so 150 is coverable and must not eliminate the player
    context.charge(one, two, 150);
    CHECK(context.raisable_cash(one) >= 150);
    CHECK(context.settle_pending_debt(true) == Payment_State::SHORT);
    CHECK(one->is_active());
    CHECK(context.has_pending_debt());

    CHECK(context.mortgage_property(property_at(context, BOARDWALK)) == Manage_Assets_State::MORTGAGE_SUCCESS);
    CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    CHECK(one->is_active());
    CHECK_EQUAL(one->get_balance(), 60);
}

static void test_bankruptcy_hands_the_estate_to_the_creditor() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    one->set_balance(10000);
    CHECK(context.build_house(street_at(context, MEDITERRANEAN)) == Manage_Assets_State::BUILD_SUCCESS);
    CHECK(context.build_house(street_at(context, BALTIC)) == Manage_Assets_State::BUILD_SUCCESS);
    one->set_balance(40);
    two->set_balance(0);

    context.charge(one, two, 100000);
    CHECK(context.settle_pending_debt(false) == Payment_State::BANKRUPT_TO_CREDITOR);
    CHECK(!one->is_active());
    CHECK_EQUAL(one->get_assets().size(), 0);
    CHECK_EQUAL(context.get_active_players().size(), 1);

    // creditor gets the cash, the deeds, and the money the bank paid for the buildings
    CHECK_EQUAL(two->get_assets().size(), 2);
    CHECK(property_at(context, MEDITERRANEAN)->get_owner() == two);
    CHECK_EQUAL(two->get_balance(), 40 + 2 * street_at(context, MEDITERRANEAN)->get_sell_price());
    CHECK_EQUAL(street_at(context, MEDITERRANEAN)->get_num_houses(), 0);
    CHECK_EQUAL(context.get_houses_available(), GameContext::house_stock);
    // and the set is now complete for the creditor
    CHECK_EQUAL(street_at(context, MEDITERRANEAN)->get_rent(), 4);
}

static void test_bankruptcy_to_the_bank_seizes_the_estate() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    give(context, one, BOARDWALK);
    CHECK(context.mortgage_property(property_at(context, BOARDWALK)) == Manage_Assets_State::MORTGAGE_SUCCESS);
    one->set_balance(0);

    context.charge(one, nullptr, 500);
    CHECK(context.settle_pending_debt(false) == Payment_State::BANKRUPT_TO_BANK);
    CHECK(!one->is_active());
    CHECK(property_at(context, BOARDWALK)->get_owner() == nullptr);
    // the loan rides with the deed until the bank manages to sell it
    CHECK(property_at(context, BOARDWALK)->get_mortgaged());
    CHECK(context.has_bank_auctions());
    Property* seized = context.next_bank_auction();
    CHECK(seized == property_at(context, BOARDWALK));
    CHECK(!context.has_bank_auctions());

    // nobody bids, so the bank writes the loan off and the deed goes back on the market clean
    context.abandon_deed(seized);
    CHECK(!seized->get_mortgaged());
}

static void test_seized_estate_can_be_auctioned_with_its_loan() {
    GameContext context({"One", "Two", "Three"}, false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, BOARDWALK);
    Property* boardwalk = property_at(context, BOARDWALK);
    CHECK(context.mortgage_property(boardwalk) == Manage_Assets_State::MORTGAGE_SUCCESS);
    one->set_balance(0);
    context.charge(one, nullptr, 500);
    CHECK(context.settle_pending_debt(false) == Payment_State::BANKRUPT_TO_BANK);

    Property* seized = context.next_bank_auction();
    CHECK(seized == boardwalk);
    unsigned int interest = context.transfer_interest(seized);
    CHECK(interest > 0);
    CHECK_EQUAL(interest, boardwalk->get_unmortgage_value() - boardwalk->get_mortgage_value());

    two->set_balance(1000);
    CHECK(context.award_auction(two, seized, 100));
    CHECK(seized->get_owner() == two);
    CHECK(seized->get_mortgaged());
    CHECK_EQUAL(two->get_balance(), 1000 - 100 - interest);
}

static void test_optional_jail_fine_cannot_bankrupt_you() {
    Game game(two_players(), false);
    GameContext* context = game.get_context();
    Player* one = context->get_players()[0];
    context->take_to_jail();
    one->set_balance(10);

    Game_State game_state = Game_State::NO_REWARD;
    bool valid_option = false;
    game.process_turn(State::JAIL, game_state, 2, valid_option);
    // the fine was optional, so an unaffordable one is refused rather than fatal
    CHECK(!valid_option);
    CHECK(!context->has_pending_debt());
    CHECK(one->is_active());
    CHECK(one->get_in_jail());

    // once there is something to mortgage the option comes back
    context->award_auction(one, dynamic_cast<Property*>(context->get_board()->get_tile(BOARDWALK)), 0);
    one->set_balance(10);
    valid_option = false;
    game.process_turn(State::JAIL, game_state, 2, valid_option);
    CHECK(valid_option);
    CHECK(game_state == Game_State::PAID_JAIL_FINE);
    CHECK(context->has_pending_debt());
    CHECK(game.resolve_debt(true) == End_Turn_State::SHORT_ON_CASH);
}

static void test_pay_each_player_and_collect_from_each() {
    GameContext context({"One", "Two", "Three"}, false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    Player* three = context.get_players()[2];
    one->set_balance(300);
    two->set_balance(300);
    three->set_balance(300);

    context.charge(one, nullptr, 100, true);
    CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    CHECK_EQUAL(one->get_balance(), 200);
    CHECK_EQUAL(two->get_balance(), 350);
    CHECK_EQUAL(three->get_balance(), 350);

    // collecting from each opponent runs one debt per opponent through the same funnel
    context.charge_opponents(one, 50);
    CHECK(context.has_pending_collections());
    while (context.next_collection()) {
        CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    }
    CHECK_EQUAL(one->get_balance(), 300);
    CHECK_EQUAL(two->get_balance(), 300);
    CHECK_EQUAL(three->get_balance(), 300);
}

static void test_collect_from_each_never_short_changes_the_collector() {
    GameContext context({"One", "Two", "Three"}, false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    Player* three = context.get_players()[2];
    give(context, two, BOARDWALK);
    one->set_balance(0);
    two->set_balance(0);
    three->set_balance(100);

    context.charge_opponents(one, 50);
    CHECK(context.next_collection());
    CHECK(context.get_pending_debtor() == two);
    // two is broke but holds Boardwalk, so they are asked to raise the money, not let off
    CHECK(context.settle_pending_debt(true) == Payment_State::SHORT);
    CHECK(context.mortgage_property(property_at(context, BOARDWALK)) == Manage_Assets_State::MORTGAGE_SUCCESS);
    CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    CHECK_EQUAL(one->get_balance(), 50);

    CHECK(context.next_collection());
    CHECK(context.get_pending_debtor() == three);
    CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    CHECK_EQUAL(one->get_balance(), 100);
    CHECK(!context.next_collection());

    // and an opponent with nothing at all is eliminated rather than paying part of it
    one->set_balance(0);
    two->set_balance(0);
    context.declare_bankrupt(three, nullptr);
    context.charge_opponents(one, 50);
    CHECK(context.next_collection());
    CHECK(context.get_pending_debtor() == two);
    CHECK(context.settle_pending_debt(true) == Payment_State::BANKRUPT_TO_CREDITOR);
    CHECK(!two->is_active());
}

static void test_raisable_cash_respects_the_bank_box() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    one->set_balance(100000);

    Street* mediterranean = street_at(context, MEDITERRANEAN);
    Street* baltic = street_at(context, BALTIC);
    for (unsigned int i = 0; i < 5; i++) {
        CHECK(context.build_house(mediterranean) == Manage_Assets_State::BUILD_SUCCESS);
        CHECK(context.build_house(baltic) == Manage_Assets_State::BUILD_SUCCESS);
    }
    CHECK_EQUAL(mediterranean->get_num_hotels(), 1);
    CHECK_EQUAL(baltic->get_num_hotels(), 1);
    one->set_balance(0);

    // with houses in the box both hotels can be broken up and sold
    CHECK_EQUAL(context.raisable_cash(one), 10 * mediterranean->get_sell_price() + mediterranean->get_mortgage_value() + baltic->get_mortgage_value());

    // drain the bank with loose houses and the hotels stop being cash the player can reach
    Player* two = context.get_players()[1];
    const unsigned int fillers[] = {6, 8, 9, 11, 13, 14, 16, 18, 19, 21, 23, 24};
    for (unsigned int index : fillers) {
        give(context, two, index);
    }
    two->set_balance(1000000);
    for (unsigned int round = 0; round < 4 && context.get_houses_available() >= 4; round++) {
        for (unsigned int index : fillers) {
            if (context.get_houses_available() < 4) {
                break;
            }
            context.build_house(street_at(context, index));
        }
    }
    CHECK(context.get_houses_available() < 4);
    CHECK(context.sell_house(mediterranean) == Manage_Assets_State::SELL_FAIL_NO_HOUSES_LEFT);
    // neither hotel can come down, so nothing on the set can be sold or mortgaged either
    CHECK_EQUAL(context.raisable_cash(one), 0);

    // give the box back enough for exactly one hotel and only that one break is reachable
    CHECK(context.sell_house(street_at(context, ORIENTAL)) == Manage_Assets_State::SELL_SUCCESS);
    CHECK_EQUAL(context.get_houses_available(), 4);
    CHECK_EQUAL(context.raisable_cash(one), mediterranean->get_sell_price());
}

static void test_mortgaged_deeds_carry_their_interest_into_a_trade() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, BOARDWALK);
    Property* boardwalk = property_at(context, BOARDWALK);
    one->set_balance(0);
    CHECK(context.mortgage_property(boardwalk) == Manage_Assets_State::MORTGAGE_SUCCESS);
    unsigned int interest = boardwalk->get_unmortgage_value() - boardwalk->get_mortgage_value();
    CHECK(interest > 0);

    Trade trade;
    trade.proposer = one;
    trade.partner = two;
    trade.offered.push_back(boardwalk);
    trade.cash_offered = 0;
    trade.cash_requested = 0;
    trade.jail_cards_offered = 0;
    trade.jail_cards_requested = 0;

    // the receiving player has to be able to cover the interest
    two->set_balance(interest - 1);
    CHECK(context.execute_trade(trade) == Trade_State::FAIL_NO_FUNDS);
    CHECK(boardwalk->get_owner() == one);

    two->set_balance(interest + 10);
    CHECK(context.execute_trade(trade) == Trade_State::SUCCESS);
    CHECK(boardwalk->get_owner() == two);
    CHECK(boardwalk->get_mortgaged());
    CHECK_EQUAL(two->get_balance(), 10);
}

static void test_free_parking_pot() {
    GameContext context(two_players(), true);
    Player* one = context.get_players()[0];
    one->set_balance(500);
    CHECK(context.get_free_parking() != nullptr);
    CHECK_EQUAL(context.get_free_parking()->peek_pot(), 0);

    context.charge(one, nullptr, 200);
    CHECK(context.settle_pending_debt(true) == Payment_State::PAID);
    CHECK_EQUAL(context.get_free_parking()->peek_pot(), 200);
    CHECK_EQUAL(context.get_free_parking()->get_pot(), 200);
    CHECK_EQUAL(context.get_free_parking()->peek_pot(), 0);

    // with the house rule off the money simply leaves the game
    GameContext plain(two_players(), false);
    plain.get_players()[0]->set_balance(500);
    plain.charge(plain.get_players()[0], nullptr, 200);
    CHECK(plain.settle_pending_debt(true) == Payment_State::PAID);
    CHECK_EQUAL(plain.get_free_parking()->peek_pot(), 0);
}

static void test_trade_is_all_or_nothing() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, MEDITERRANEAN);
    give(context, one, BALTIC);
    give(context, two, ORIENTAL);
    one->set_balance(1000);
    two->set_balance(1000);

    // a set with a building on it cannot be traded, and nothing must move when it is refused
    CHECK(context.build_house(street_at(context, MEDITERRANEAN)) == Manage_Assets_State::BUILD_SUCCESS);
    unsigned int one_before = one->get_balance();
    unsigned int two_before = two->get_balance();
    Trade blocked;
    blocked.proposer = one;
    blocked.partner = two;
    blocked.offered.push_back(property_at(context, BALTIC));
    blocked.requested.push_back(property_at(context, ORIENTAL));
    blocked.cash_offered = 100;
    blocked.cash_requested = 0;
    blocked.jail_cards_offered = 0;
    blocked.jail_cards_requested = 0;
    CHECK(context.execute_trade(blocked) == Trade_State::FAIL_HAS_BUILDINGS);
    CHECK_EQUAL(one->get_balance(), one_before);
    CHECK_EQUAL(two->get_balance(), two_before);
    CHECK(property_at(context, BALTIC)->get_owner() == one);

    CHECK(context.sell_house(street_at(context, MEDITERRANEAN)) == Manage_Assets_State::SELL_SUCCESS);
    one->set_balance(1000);

    // asking for more cash than the partner has must also change nothing
    Trade broke = blocked;
    broke.cash_offered = 0;
    broke.cash_requested = 5000;
    CHECK(context.execute_trade(broke) == Trade_State::FAIL_NO_FUNDS);
    CHECK(property_at(context, ORIENTAL)->get_owner() == two);

    Trade good = blocked;
    good.cash_offered = 100;
    CHECK(context.execute_trade(good) == Trade_State::SUCCESS);
    CHECK(property_at(context, BALTIC)->get_owner() == two);
    CHECK(property_at(context, ORIENTAL)->get_owner() == one);
    CHECK_EQUAL(one->get_balance(), 900);
    CHECK_EQUAL(two->get_balance(), 1100);
    CHECK_EQUAL(one->get_assets().size(), 2);
    CHECK_EQUAL(two->get_assets().size(), 1);
}

static void test_jail_card_leaves_and_returns_to_its_deck() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];

    // walk the whole chance deck twice; the card can only ever be drawn once
    unsigned int held_after_first_pass = 0;
    for (unsigned int i = 0; i < 40; i++) {
        context.draw_chance();
        if (one->has_jail_card(Deck_Type::CHANCE)) {
            held_after_first_pass = one->get_jail_cards();
            break;
        }
    }
    CHECK_EQUAL(held_after_first_pass, 1);
    for (unsigned int i = 0; i < 40; i++) {
        context.draw_chance();
    }
    // still held, and never handed out a second time
    CHECK(one->has_jail_card(Deck_Type::CHANCE));
    CHECK_EQUAL(one->get_jail_cards(), 1);

    // using it puts it back in circulation
    CHECK(context.use_jail_card(one));
    CHECK(!one->has_jail_card(Deck_Type::CHANCE));
    bool drawn_again = false;
    for (unsigned int i = 0; i < 40 && !drawn_again; i++) {
        context.draw_chance();
        drawn_again = one->has_jail_card(Deck_Type::CHANCE);
    }
    CHECK(drawn_again);
}

static void test_movement_wraps_and_pays_go() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Game_State reward = Game_State::NO_REWARD;

    context.move_player(37, reward);
    CHECK(reward == Game_State::NO_REWARD);
    CHECK_EQUAL(context.get_player_location(one), 37);

    // 37 + 5 wraps past GO to 2
    context.move_player(5, reward);
    CHECK(reward == Game_State::PASSED_GO_REWARD);
    CHECK_EQUAL(context.get_player_location(one), 2);

    // landing exactly on GO is reported separately
    context.move_player(38, reward);
    CHECK(reward == Game_State::LANDED_ON_GO_REWARD);
    CHECK_EQUAL(context.get_player_location(one), 0);

    // several laps must not push the stored position past the end of the board
    for (unsigned int i = 0; i < 30; i++) {
        context.move_player(11, reward);
        CHECK(context.get_player_location(one) < Board::board_size);
    }
}

static void test_move_to_tile_pays_go_only_when_it_passes() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Game_State reward = Game_State::NO_REWARD;

    context.move_player(30, reward);
    unsigned int before = one->get_balance();
    // 30 -> 5 goes past GO
    context.move_player_to_tile(context.get_tiles()[5], 5, true, GameContext::go_bonus);
    CHECK_EQUAL(context.get_player_location(one), 5);
    CHECK_EQUAL(one->get_balance(), before + GameContext::go_bonus);

    // 5 -> 24 does not
    before = one->get_balance();
    context.move_player_to_tile(context.get_tiles()[24], 24, true, GameContext::go_bonus);
    CHECK_EQUAL(context.get_player_location(one), 24);
    CHECK_EQUAL(one->get_balance(), before);

    // a card that says "advance to Boardwalk" pays nothing even though it wraps forward
    before = one->get_balance();
    context.move_player_to_tile(context.get_tiles()[39], 39, false, 0);
    CHECK_EQUAL(context.get_player_location(one), 39);
    CHECK_EQUAL(one->get_balance(), before);
}

static void test_jail_moves_the_pawn_and_the_position() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Game_State reward = Game_State::NO_REWARD;
    context.move_player(30, reward);
    CHECK_EQUAL(context.get_player_location(one), 30);

    context.take_to_jail();
    CHECK(one->get_in_jail());
    CHECK_EQUAL(context.get_player_location(one), GameContext::jail_index);
    CHECK_EQUAL(context.get_tiles()[30]->get_path().find(one->get_pawn()), std::string::npos);

    context.release_from_jail();
    CHECK(!one->get_in_jail());
    CHECK_EQUAL(one->get_rounds_in_jail(), 0);
    CHECK(context.get_tiles()[GameContext::jail_index]->get_path().find(one->get_pawn()) != std::string::npos);
    // releasing a player who is not in jail must not throw
    context.release_from_jail();
}

static void test_asset_counters_track_transfers() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    give(context, one, READING_RAILROAD);
    give(context, one, ELECTRIC_COMPANY);
    CHECK_EQUAL(one->get_num_railroads(), 1);
    CHECK_EQUAL(one->get_num_utilities(), 1);

    Trade trade;
    trade.proposer = one;
    trade.partner = two;
    trade.offered.push_back(property_at(context, READING_RAILROAD));
    trade.offered.push_back(property_at(context, ELECTRIC_COMPANY));
    trade.cash_offered = 0;
    trade.cash_requested = 0;
    trade.jail_cards_offered = 0;
    trade.jail_cards_requested = 0;
    CHECK(context.execute_trade(trade) == Trade_State::SUCCESS);
    CHECK_EQUAL(one->get_num_railroads(), 0);
    CHECK_EQUAL(one->get_num_utilities(), 0);
    CHECK_EQUAL(two->get_num_railroads(), 1);
    CHECK_EQUAL(two->get_num_utilities(), 1);
}

static void test_turn_order_skips_eliminated_players() {
    GameContext context({"One", "Two", "Three"}, false);
    Player* one = context.get_players()[0];
    Player* two = context.get_players()[1];
    Player* three = context.get_players()[2];
    CHECK(context.get_current_player() == one);

    context.declare_bankrupt(two, nullptr);
    context.next_player();
    CHECK(context.get_current_player() == three);
    context.next_player();
    CHECK(context.get_current_player() == one);
    CHECK_EQUAL(context.get_active_players().size(), 2);
    CHECK_EQUAL(context.get_opponents(one).size(), 1);
}

static void test_buying_checks_the_balance() {
    GameContext context(two_players(), false);
    Player* one = context.get_players()[0];
    Property* boardwalk = property_at(context, BOARDWALK);
    context.set_current_tile(boardwalk);

    one->set_balance(10);
    CHECK(!context.buy_property());
    CHECK(boardwalk->get_owner() == nullptr);
    CHECK_EQUAL(one->get_balance(), 10);

    one->set_balance(400);
    CHECK(context.buy_property());
    CHECK(boardwalk->get_owner() == one);
    CHECK_EQUAL(one->get_balance(), 0);
    // a deed that already has an owner cannot be bought again
    CHECK(!context.buy_property());
}

static void test_auction_awards_the_deed() {
    GameContext context(two_players(), false);
    Player* two = context.get_players()[1];
    Property* park_place = property_at(context, PARK_PLACE);
    two->set_balance(500);
    CHECK(context.award_auction(two, park_place, 120));
    CHECK(park_place->get_owner() == two);
    CHECK_EQUAL(two->get_balance(), 380);
    CHECK(!context.award_auction(context.get_players()[0], park_place, 10));
}

static void test_no_rent_is_owed_to_yourself() {
    Game game(two_players(), false);
    GameContext* context = game.get_context();
    Player* owner = context->get_players()[0];
    Property* vermont = dynamic_cast<Property*>(context->get_board()->get_tile(VERMONT));
    context->award_auction(owner, vermont, 0);
    owner->set_balance(1000);
    context->set_current_tile(vermont);
    CHECK(vermont->Action(owner) == Tile_State::OWN_PROPERTY);
    CHECK(game.process_turn(Tile_State::OWN_PROPERTY, 1) == End_Turn_State::CONTINUE);
    CHECK_EQUAL(owner->get_balance(), 1000);
}

static void test_rent_flows_through_the_game_layer() {
    Game game(two_players(), false);
    GameContext* context = game.get_context();
    Player* payer = context->get_players()[0];
    Player* owner = context->get_players()[1];
    Property* connecticut = dynamic_cast<Property*>(context->get_board()->get_tile(CONNECTICUT));
    context->award_auction(owner, connecticut, 0);
    owner->set_balance(0);
    payer->set_balance(100);
    context->set_current_tile(connecticut);

    CHECK(connecticut->Action(payer) == Tile_State::OWNED_STREET);
    CHECK(game.process_turn(Tile_State::OWNED_STREET, 1) == End_Turn_State::PAYED_RENT);
    CHECK_EQUAL(payer->get_balance(), 92);
    CHECK_EQUAL(owner->get_balance(), 8);
    CHECK_EQUAL(payer->get_last_paid_rent(), 8);
}

static void test_tax_uses_the_debt_funnel() {
    Game game(two_players(), false);
    GameContext* context = game.get_context();
    Player* payer = context->get_players()[0];
    context->set_current_tile(context->get_board()->get_tile(4));
    payer->set_balance(500);
    CHECK(game.process_turn(Tile_State::TAX, 1) == End_Turn_State::PAYED_TAX);
    CHECK_EQUAL(payer->get_balance(), 300);

    payer->set_balance(10);
    CHECK(game.process_turn(Tile_State::TAX, 1) == End_Turn_State::BANKRUPT);
    CHECK(!payer->is_active());
    CHECK(game.is_over());
}

typedef void (*test_function)();

struct TestCase {
    const char* name;
    test_function run;
};

int main() {
    const TestCase tests[] = {
        {"board loads", test_board_loads},
        {"full set doubles rent", test_full_set_doubles_rent},
        {"even building", test_even_building},
        {"building costs and stock", test_building_costs_and_stock},
        {"hotels", test_hotels},
        {"house stock runs out", test_house_stock_runs_out},
        {"hotel stock runs out", test_hotel_stock_runs_out},
        {"mortgages", test_mortgages},
        {"railroad and utility rent", test_railroad_and_utility_rent},
        {"debt is paid to the creditor", test_debt_is_paid_to_the_creditor},
        {"debt offers liquidation first", test_debt_offers_liquidation_before_bankruptcy},
        {"bankruptcy to a creditor", test_bankruptcy_hands_the_estate_to_the_creditor},
        {"bankruptcy to the bank seizes the estate", test_bankruptcy_to_the_bank_seizes_the_estate},
        {"seized estate auctioned with its loan", test_seized_estate_can_be_auctioned_with_its_loan},
        {"optional jail fine cannot bankrupt you", test_optional_jail_fine_cannot_bankrupt_you},
        {"pay and collect from each player", test_pay_each_player_and_collect_from_each},
        {"collect from each never short changes", test_collect_from_each_never_short_changes_the_collector},
        {"raisable cash respects the bank box", test_raisable_cash_respects_the_bank_box},
        {"mortgaged deeds carry interest into a trade", test_mortgaged_deeds_carry_their_interest_into_a_trade},
        {"free parking pot", test_free_parking_pot},
        {"trade is all or nothing", test_trade_is_all_or_nothing},
        {"jail card leaves and returns", test_jail_card_leaves_and_returns_to_its_deck},
        {"movement wraps and pays GO", test_movement_wraps_and_pays_go},
        {"move to tile pays GO only when passing", test_move_to_tile_pays_go_only_when_it_passes},
        {"jail moves pawn and position", test_jail_moves_the_pawn_and_the_position},
        {"asset counters track transfers", test_asset_counters_track_transfers},
        {"turn order skips eliminated players", test_turn_order_skips_eliminated_players},
        {"buying checks the balance", test_buying_checks_the_balance},
        {"auction awards the deed", test_auction_awards_the_deed},
        {"no rent is owed to yourself", test_no_rent_is_owed_to_yourself},
        {"rent flows through the game layer", test_rent_flows_through_the_game_layer},
        {"tax uses the debt funnel", test_tax_uses_the_debt_funnel}
    };

    for (const TestCase& test : tests) {
        current_test = test.name;
        unsigned int before = failures;
        test.run();
        std::cout << ((failures == before) ? "  ok   " : "  FAIL ") << test.name << std::endl;
    }

    std::cout << std::endl << checks << " checks, " << failures << " failures" << std::endl;
    return (failures == 0) ? 0 : 1;
}
