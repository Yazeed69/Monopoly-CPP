#include "GameContext.h"

#include <algorithm>
#include <chrono>
#include <random>

GameContext::GameContext(const std::vector<std::string>& names, bool free_parking_pot) {
    board = new Board();
    players = std::vector<Player*>();
    turn = 0;
    current_player = nullptr;
    current_tile = nullptr;
    free_parking = nullptr;
    chance_index = 0;
    community_chest_index = 0;

    houses_available = house_stock;
    hotels_available = hotel_stock;

    pending_debt = 0;
    pending_debtor = nullptr;
    pending_creditor = nullptr;
    pending_debt_to_all = false;
    pending_rent = 0;
    pending_collector = nullptr;
    pending_collection_amount = 0;
    advanced = false;

    dies = {new Dice(), new Dice()};
    dies[0]->roll();
    dies[1]->roll();

    // a board that failed to load leaves the context inert, main reports it and quits
    if (!board->is_loaded()) {
        return;
    }

    for (unsigned int i = 0; i < names.size(); i++) {
        Player* player = new Player(names[i], 'A' + i, starting_balance);
        players.push_back(player);
        players_locations[player] = 0;
        board->place_pawn(0, player->get_pawn());
    }
    current_player = players[0];
    current_tile = board->get_tiles()[0];

    for (unsigned int i = 0; i < Board::board_size; i++) {
        FreeParking* tile = dynamic_cast<FreeParking*>(board->get_tile(i));
        if (tile) {
            free_parking = tile;
            free_parking->set_using_pot(free_parking_pot);
            break;
        }
    }

    create_chance_deck();
    create_community_chest_deck();
}

GameContext::~GameContext(){
    delete board;
    for (Player* player : players) {
        delete player;
    }
    delete dies[0];
    delete dies[1];
}

std::vector<Player*> GameContext::get_players() const {
    return players;
}

Player* GameContext::get_current_player() const {
    return current_player;
}

void GameContext::set_current_tile(Tile* tile) {
    current_tile = tile;
}

std::unordered_map<Player *, unsigned int> GameContext::get_players_locations() const {
    return players_locations;
}

unsigned int GameContext::get_player_location(Player* player) const {
    auto it = players_locations.find(player);
    return (it == players_locations.end()) ? 0 : it->second % Board::board_size;
}

Tile *GameContext::get_current_tile() const {
    return current_tile;
}

Tile** GameContext::get_tiles() const {
    return board->get_tiles();
}

Board* GameContext::get_board() const {
    return board;
}

Dice* GameContext::get_die(unsigned int i) const {
    return dies[i];
}

FreeParking* GameContext::get_free_parking() const {
    return free_parking;
}

Tile *GameContext::move_player(unsigned int roll, Game_State& reward) {
    reward = Game_State::NO_REWARD;
    auto it = players_locations.find(current_player);
    if (it == players_locations.end()){
        return current_tile;
    }
    // positions are kept inside 0..39 so every later lookup is in range
    unsigned int initial_position = it->second % Board::board_size;
    get_tiles()[initial_position]->exit_path(it->first->get_pawn());
    unsigned int final_position = (initial_position + roll) % Board::board_size;
    it->second = final_position;
    get_tiles()[final_position]->enter_path(it->first->get_pawn());
    if (initial_position + roll >= Board::board_size) {
        reward = (final_position == 0) ? Game_State::LANDED_ON_GO_REWARD : Game_State::PASSED_GO_REWARD;
    }
    return get_tiles()[final_position];
}

/*
next railroad: 5     15       25     35
next Utility:  12       28
broadwalk:  39
st._Charles_Place:  11
Illinois_Avenue:  24
*/


// the destination is decided by tile_index alone, the tile pointer is only there so callers
// read as "advance to this tile" at the call site
void GameContext::move_player_to_tile(Tile *tile, unsigned int tile_index, bool passGo, unsigned int goBonus) {
    unsigned int normalized_start = get_player_location(current_player);
    unsigned int normalized_target = tile_index % Board::board_size;
    unsigned int distance = (normalized_target + Board::board_size - normalized_start) % Board::board_size;
    if (distance == 0) {
        // already standing there, the card still pays when it sends you to GO
        set_current_tile(get_tiles()[normalized_target]);
        if (passGo && normalized_target == 0 && goBonus > 0) {
            current_player->add_balance(goBonus);
        }
        return;
    }
    Game_State reward = Game_State::NO_REWARD;
    set_current_tile(move_player(distance, reward));
    bool passed_go = reward == Game_State::PASSED_GO_REWARD || reward == Game_State::LANDED_ON_GO_REWARD;
    if (passGo && passed_go && goBonus > 0) {
        current_player->add_balance(goBonus);
    }
}

void GameContext::roll_dice(Player *player) {
    dies[0]->roll();
    dies[1]->roll();
    player->set_rolled_double(dies[0]->get_rolled() == dies[1]->get_rolled());
    player->set_roll(dies[0]->get_rolled() + dies[1]->get_rolled());
}

bool GameContext::buy_property() {
    Property* property = dynamic_cast<Property*>(current_tile);
    if (!property || property->get_owner() != nullptr) {
        return false;
    }
    if (!current_player->withdraw_balance(property->get_purchase_price())) {
        return false;
    }
    bool full_set = false;
    current_player->add_asset(property, full_set);
    property->set_owner(current_player);
    update_rents_for_color_set(property->get_color_set());
    return true;
}

bool GameContext::award_auction(Player* winner, Property* property, unsigned int price) {
    if (!winner || !property || property->get_owner() != nullptr) {
        return false;
    }
    unsigned int interest = transfer_interest(property);
    if (!winner->withdraw_balance(price + interest)) {
        return false;
    }
    pay_bank(interest);
    bool full_set = false;
    winner->add_asset(property, full_set);
    property->set_owner(winner);
    update_rents_for_color_set(property->get_color_set());
    return true;
}

bool GameContext::transfer_property(Player* from, Player* to, Property* property, unsigned int price) {
    if (!from || !to || !property) {
        return false;
    }
    if (property->get_owner() != from) {
        return false;
    }
    Street* street = dynamic_cast<Street*>(property);
    if (street && street->get_num_houses() > 0) {
        return false;
    }
    if (!to->withdraw_balance(price)) {
        return false;
    }
    from->add_balance(price);
    from->remove_asset(property);
    bool full_set = false;
    to->add_asset(property, full_set);
    property->set_owner(to);
    update_rents_for_color_set(property->get_color_set());
    return true;
}

void GameContext::declare_bankrupt(Player* debtor, Player* creditor) {
    if (!debtor || !debtor->is_active()) {
        return;
    }
    std::vector<Tile*> debtor_assets = debtor->get_assets();
    for (Tile* asset : debtor_assets) {
        Property* property = dynamic_cast<Property*>(asset);
        if (!property) {
            continue;
        }
        // buildings always go back in the bank's box before the deed changes hands
        Street* street = dynamic_cast<Street*>(property);
        if (street && street->get_num_houses() > 0) {
            unsigned int refund = street->get_num_houses() * street->get_sell_price();
            if (street->get_num_houses() == 5) {
                hotels_available++;
            } else {
                houses_available += street->get_num_houses();
            }
            street->reset_buildings();
            if (creditor) {
                creditor->add_balance(refund);
            }
        }
        debtor->remove_asset(property);
        if (creditor) {
            bool full_set = false;
            creditor->add_asset(property, full_set);
            property->set_owner(creditor);
        } else {
            property->set_owner(nullptr);
            // the bank sells a seized estate by auction, and the loan stays on the deed
            pending_bank_auctions.push_back(property);
        }
        update_rents_for_color_set(property->get_color_set());
    }
    while (debtor->has_jail_card(Deck_Type::CHANCE)) {
        debtor->consume_jail_card(Deck_Type::CHANCE);
        if (creditor) {
            creditor->add_jail_card(Deck_Type::CHANCE);
        } else {
            return_jail_card(Deck_Type::CHANCE);
        }
    }
    while (debtor->has_jail_card(Deck_Type::COMMUNITY_CHEST)) {
        debtor->consume_jail_card(Deck_Type::COMMUNITY_CHEST);
        if (creditor) {
            creditor->add_jail_card(Deck_Type::COMMUNITY_CHEST);
        } else {
            return_jail_card(Deck_Type::COMMUNITY_CHEST);
        }
    }
    unsigned int remaining = debtor->get_balance();
    if (remaining > 0) {
        debtor->withdraw_balance(remaining);
        if (creditor) {
            creditor->add_balance(remaining);
        } else {
            pay_bank(remaining);
        }
    }
    unsigned int location = get_player_location(debtor);
    get_tiles()[location]->exit_path(debtor->get_pawn());
    if (debtor->get_in_jail()) {
        Hybrid* jail = dynamic_cast<Hybrid*>(get_tiles()[jail_index]);
        if (jail) {
            jail->release_from_jail(debtor->get_pawn());
        }
        debtor->set_in_jail(false);
    }
    debtor->set_active(false);
}

void GameContext::update_rents_for_color_set(const std::string& color_set) {
    std::vector<Street*> streets = board->get_color_set_assets(color_set);
    if (streets.empty()) {
        // railroads and utilities scale off the owner's count, nothing to cache
        return;
    }
    Player* owner = streets[0]->get_owner();
    bool single_owner = owner != nullptr;
    for (Street* street : streets) {
        if (street->get_owner() != owner || street->get_owner() == nullptr) {
            single_owner = false;
            break;
        }
    }
    for (Street* street : streets) {
        street->set_full_set(single_owner);
    }
}

std::vector<Player*> GameContext::get_active_players() const {
    std::vector<Player*> active_players;
    for (Player* player : players) {
        if (player->is_active()) {
            active_players.push_back(player);
        }
    }
    return active_players;
}

std::vector<Player*> GameContext::get_opponents(Player* player) const {
    std::vector<Player*> opponents;
    for (Player* other : players) {
        if (other != player && other->is_active()) {
            opponents.push_back(other);
        }
    }
    return opponents;
}

void GameContext::charge(Player* debtor, Player* creditor, unsigned int amount, bool to_all) {
    pending_debtor = debtor;
    pending_creditor = creditor;
    pending_debt = amount;
    pending_debt_to_all = to_all;
}

bool GameContext::has_pending_debt() const {
    return pending_debt > 0 && pending_debtor != nullptr;
}

unsigned int GameContext::get_pending_debt() const {
    return pending_debt;
}

Player* GameContext::get_pending_debtor() const {
    return pending_debtor;
}

Player* GameContext::get_pending_creditor() const {
    return pending_creditor;
}

void GameContext::clear_pending_debt() {
    pending_debt = 0;
    pending_debtor = nullptr;
    pending_creditor = nullptr;
    pending_debt_to_all = false;
}

Payment_State GameContext::settle_pending_debt(bool allow_raise) {
    if (!has_pending_debt()) {
        clear_pending_debt();
        return Payment_State::PAID;
    }
    Player* debtor = pending_debtor;
    if (debtor->withdraw_balance(pending_debt)) {
        distribute_payment(pending_debt);
        clear_pending_debt();
        return Payment_State::PAID;
    }
    if (allow_raise && raisable_cash(debtor) >= pending_debt) {
        // the player can still cover this by selling or mortgaging
        return Payment_State::SHORT;
    }
    unsigned int remaining = debtor->get_balance();
    if (remaining > 0) {
        debtor->withdraw_balance(remaining);
        distribute_payment(remaining);
    }
    Player* creditor = (pending_debt_to_all) ? nullptr : pending_creditor;
    Payment_State result = (creditor) ? Payment_State::BANKRUPT_TO_CREDITOR : Payment_State::BANKRUPT_TO_BANK;
    clear_pending_debt();
    declare_bankrupt(debtor, creditor);
    return result;
}

void GameContext::distribute_payment(unsigned int amount) {
    if (amount == 0) {
        return;
    }
    if (pending_debt_to_all) {
        std::vector<Player*> opponents = get_opponents(pending_debtor);
        if (opponents.empty()) {
            pay_bank(amount);
            return;
        }
        unsigned int share = amount / opponents.size();
        unsigned int given = 0;
        for (Player* opponent : opponents) {
            opponent->add_balance(share);
            given += share;
        }
        pay_bank(amount - given);
        return;
    }
    if (pending_creditor) {
        pending_creditor->add_balance(amount);
        return;
    }
    pay_bank(amount);
}

void GameContext::pay_bank(unsigned int amount) {
    // the bank is bottomless, the money only reappears under the free parking house rule
    if (free_parking) {
        free_parking->add_to_pot(static_cast<int>(amount));
    }
}

unsigned int GameContext::raisable_cash(Player* player) const {
    if (!player) {
        return 0;
    }
    unsigned int total = player->get_balance();
    unsigned int bank_houses = houses_available;

    // a colour set can only be stripped as a whole. Every hotel on it has to come down to
    // four houses before the even-selling rule lets any house go, and no deed on it can be
    // mortgaged until it is bare, so the set is worth nothing until the bank can hand back
    // four houses for each of its hotels at once.
    std::unordered_map<std::string, std::vector<Street*>> street_sets;
    for (Tile* asset : player->get_assets()) {
        Property* property = dynamic_cast<Property*>(asset);
        if (!property) {
            continue;
        }
        Street* street = dynamic_cast<Street*>(asset);
        if (street) {
            street_sets[street->get_color_set()].push_back(street);
        } else if (!property->get_mortgaged()) {
            // railroads and utilities never carry buildings, their loan is always available
            total += property->get_mortgage_value();
        }
    }

    // take the sets that need the fewest houses out of the box first, which is the order a
    // player selling under pressure would use
    std::vector<std::pair<unsigned int, std::string>> order;
    for (const auto& entry : street_sets) {
        unsigned int hotels = 0;
        for (Street* street : entry.second) {
            hotels += street->get_num_hotels();
        }
        order.push_back(std::make_pair(hotels, entry.first));
    }
    std::sort(order.begin(), order.end());

    for (const std::pair<unsigned int, std::string>& entry : order) {
        unsigned int hotels = entry.first;
        const std::vector<Street*>& streets = street_sets[entry.second];
        if (hotels > 0 && bank_houses < 4 * hotels) {
            // only some of the hotels can be broken up, and nothing else on the set moves
            unsigned int breakable = bank_houses / 4;
            std::vector<unsigned int> hotel_prices;
            for (Street* street : streets) {
                if (street->get_num_houses() == 5) {
                    hotel_prices.push_back(street->get_sell_price());
                }
            }
            std::sort(hotel_prices.rbegin(), hotel_prices.rend());
            for (unsigned int i = 0; i < breakable && i < hotel_prices.size(); i++) {
                total += hotel_prices[i];
            }
            bank_houses -= breakable * 4;
            continue;
        }
        for (Street* street : streets) {
            total += street->get_num_houses() * street->get_sell_price();
            if (street->get_num_houses() < 5) {
                bank_houses += street->get_num_houses();
            }
            if (!street->get_mortgaged()) {
                total += street->get_mortgage_value();
            }
        }
    }
    return total;
}

unsigned int GameContext::transfer_interest(Property* property) const {
    if (!property || !property->get_mortgaged()) {
        return 0;
    }
    return property->get_unmortgage_value() - property->get_mortgage_value();
}

Property* GameContext::next_bank_auction() {
    while (!pending_bank_auctions.empty()) {
        Property* property = pending_bank_auctions.front();
        pending_bank_auctions.erase(pending_bank_auctions.begin());
        if (property->get_owner() == nullptr) {
            return property;
        }
    }
    return nullptr;
}

bool GameContext::has_bank_auctions() const {
    return !pending_bank_auctions.empty();
}

void GameContext::abandon_deed(Property* property) {
    if (!property || property->get_owner() != nullptr) {
        return;
    }
    // nobody bid, so the bank writes the loan off and the deed goes back on the market clean
    property->set_mortgaged(false);
    update_rents_for_color_set(property->get_color_set());
}

void GameContext::charge_opponents(Player* collector, unsigned int amount) {
    pending_collections.clear();
    pending_collector = collector;
    pending_collection_amount = amount;
    if (amount == 0) {
        return;
    }
    pending_collections = get_opponents(collector);
}

bool GameContext::has_pending_collections() const {
    return !pending_collections.empty();
}

bool GameContext::next_collection() {
    while (!pending_collections.empty()) {
        Player* opponent = pending_collections.front();
        pending_collections.erase(pending_collections.begin());
        if (opponent->is_active()) {
            charge(opponent, pending_collector, pending_collection_amount);
            return true;
        }
    }
    return false;
}

unsigned int GameContext::get_pending_rent() const {
    return pending_rent;
}

void GameContext::clear_pending_rent() {
    pending_rent = 0;
}

unsigned int GameContext::rent_due(Tile* tile, Player* player) const {
    if (pending_rent > 0) {
        return pending_rent;
    }
    Street* street = dynamic_cast<Street*>(tile);
    if (street) {
        return street->get_rent();
    }
    Railroad* railroad = dynamic_cast<Railroad*>(tile);
    if (railroad && railroad->get_owner()) {
        return railroad->get_rent(railroad->get_owner()->get_num_railroads());
    }
    Utility* utility = dynamic_cast<Utility*>(tile);
    if (utility && utility->get_owner()) {
        return utility->get_rent(utility->get_owner()->get_num_utilities(), player->get_roll());
    }
    return 0;
}

std::vector<Street*> GameContext::color_set_of(Property* property) const {
    if (!property) {
        return std::vector<Street*>();
    }
    return board->get_color_set_assets(property->get_color_set());
}

Manage_Assets_State GameContext::build_house(Street* street) {
    if (!street || street->get_owner() == nullptr) {
        return Manage_Assets_State::NULL_STATE;
    }
    Player* owner = street->get_owner();
    std::vector<Street*> set_streets = color_set_of(street);
    for (Street* sibling : set_streets) {
        if (sibling->get_owner() != owner) {
            return Manage_Assets_State::NULL_STATE;
        }
        if (sibling->get_mortgaged()) {
            return Manage_Assets_State::BUILD_FAIL_MORTGAGED;
        }
    }
    if (street->get_num_houses() >= 5) {
        return Manage_Assets_State::BUILD_FAIL_MAX_BUILDINGS;
    }
    unsigned int min_houses = street->get_num_houses();
    for (Street* sibling : set_streets) {
        min_houses = std::min(min_houses, sibling->get_num_houses());
    }
    if (street->get_num_houses() != min_houses) {
        return Manage_Assets_State::BUILD_FAIL_NOT_EQUAL_BUILDINGS;
    }
    bool upgrading_to_hotel = street->get_num_houses() == 4;
    if (upgrading_to_hotel) {
        if (hotels_available == 0) {
            return Manage_Assets_State::BUILD_FAIL_NO_HOTELS_LEFT;
        }
    } else if (houses_available == 0) {
        return Manage_Assets_State::BUILD_FAIL_NO_HOUSES_LEFT;
    }
    if (!owner->withdraw_balance(street->get_build_cost())) {
        return Manage_Assets_State::BUILD_FAIL_NO_MONEY;
    }
    if (upgrading_to_hotel) {
        hotels_available--;
        houses_available += 4;
    } else {
        houses_available--;
    }
    street->build();
    return Manage_Assets_State::BUILD_SUCCESS;
}

Manage_Assets_State GameContext::sell_house(Street* street) {
    if (!street || street->get_owner() == nullptr) {
        return Manage_Assets_State::NULL_STATE;
    }
    if (street->get_num_houses() == 0) {
        return Manage_Assets_State::SELL_FAIL_NO_BUILDINGS;
    }
    unsigned int max_houses = 0;
    for (Street* sibling : color_set_of(street)) {
        max_houses = std::max(max_houses, sibling->get_num_houses());
    }
    if (street->get_num_houses() != max_houses) {
        return Manage_Assets_State::SELL_FAIL_NOT_EQUAL_BUILDINGS;
    }
    if (street->get_num_houses() == 5) {
        // a hotel comes down as four houses, the bank has to have them in stock
        if (houses_available < 4) {
            return Manage_Assets_State::SELL_FAIL_NO_HOUSES_LEFT;
        }
        houses_available -= 4;
        hotels_available++;
    } else {
        houses_available++;
    }
    street->sell();
    street->get_owner()->add_balance(street->get_sell_price());
    return Manage_Assets_State::SELL_SUCCESS;
}

Manage_Assets_State GameContext::mortgage_property(Property* property) {
    if (!property || property->get_owner() == nullptr || property->get_mortgaged()) {
        return Manage_Assets_State::NULL_STATE;
    }
    for (Street* sibling : color_set_of(property)) {
        if (sibling->get_num_houses() > 0) {
            return Manage_Assets_State::MORTGAGE_FAIL_HAS_BUILDINGS;
        }
    }
    property->set_mortgaged(true);
    property->get_owner()->add_balance(property->get_mortgage_value());
    update_rents_for_color_set(property->get_color_set());
    return Manage_Assets_State::MORTGAGE_SUCCESS;
}

Manage_Assets_State GameContext::unmortgage_property(Property* property) {
    if (!property || property->get_owner() == nullptr || !property->get_mortgaged()) {
        return Manage_Assets_State::NULL_STATE;
    }
    if (!property->get_owner()->withdraw_balance(property->get_unmortgage_value())) {
        return Manage_Assets_State::UNMORTGAGE_FAIL_NO_MONEY;
    }
    property->set_mortgaged(false);
    update_rents_for_color_set(property->get_color_set());
    return Manage_Assets_State::UNMORTGAGE_SUCCESS;
}

unsigned int GameContext::get_houses_available() const {
    return houses_available;
}

unsigned int GameContext::get_hotels_available() const {
    return hotels_available;
}

unsigned int GameContext::mortgage_interest(const std::vector<Property*>& properties) const {
    unsigned int interest = 0;
    for (Property* property : properties) {
        if (property && property->get_mortgaged()) {
            interest += property->get_unmortgage_value() - property->get_mortgage_value();
        }
    }
    return interest;
}

std::string GameContext::describe_trade(const Trade& trade) const {
    std::string description = trade.proposer->get_name() + " gives: ";
    if (trade.cash_offered > 0) {
        description += "$" + std::to_string(trade.cash_offered) + " ";
    }
    for (Property* property : trade.offered) {
        description += property->get_name() + " ";
    }
    if (trade.jail_cards_offered > 0) {
        description += std::to_string(trade.jail_cards_offered) + "x jail card ";
    }
    if (trade.cash_offered == 0 && trade.offered.empty() && trade.jail_cards_offered == 0) {
        description += "nothing ";
    }
    description += "\n" + trade.partner->get_name() + " gives: ";
    if (trade.cash_requested > 0) {
        description += "$" + std::to_string(trade.cash_requested) + " ";
    }
    for (Property* property : trade.requested) {
        description += property->get_name() + " ";
    }
    if (trade.jail_cards_requested > 0) {
        description += std::to_string(trade.jail_cards_requested) + "x jail card ";
    }
    if (trade.cash_requested == 0 && trade.requested.empty() && trade.jail_cards_requested == 0) {
        description += "nothing ";
    }
    unsigned int proposer_interest = mortgage_interest(trade.requested);
    unsigned int partner_interest = mortgage_interest(trade.offered);
    if (proposer_interest > 0 || partner_interest > 0) {
        description += "\nMortgage interest owed to the bank on transfer: "
                    + trade.proposer->get_name() + " $" + std::to_string(proposer_interest)
                    + ", " + trade.partner->get_name() + " $" + std::to_string(partner_interest);
    }
    return description;
}

Trade_State GameContext::execute_trade(const Trade& trade) {
    if (!trade.proposer || !trade.partner) {
        return Trade_State::CANCELLED;
    }
    if (trade.offered.empty() && trade.requested.empty() && trade.cash_offered == 0 && trade.cash_requested == 0
        && trade.jail_cards_offered == 0 && trade.jail_cards_requested == 0) {
        return Trade_State::FAIL_NOTHING_OFFERED;
    }
    // validate everything before a single deed or dollar moves
    for (Property* property : trade.offered) {
        if (!property || property->get_owner() != trade.proposer) {
            return Trade_State::CANCELLED;
        }
        for (Street* sibling : color_set_of(property)) {
            if (sibling->get_num_houses() > 0) {
                return Trade_State::FAIL_HAS_BUILDINGS;
            }
        }
    }
    for (Property* property : trade.requested) {
        if (!property || property->get_owner() != trade.partner) {
            return Trade_State::CANCELLED;
        }
        for (Street* sibling : color_set_of(property)) {
            if (sibling->get_num_houses() > 0) {
                return Trade_State::FAIL_HAS_BUILDINGS;
            }
        }
    }
    // whoever takes on a mortgaged deed owes the bank its interest straight away
    unsigned int proposer_interest = mortgage_interest(trade.requested);
    unsigned int partner_interest = mortgage_interest(trade.offered);
    if (trade.proposer->get_balance() + trade.cash_requested < trade.cash_offered + proposer_interest
        || trade.partner->get_balance() + trade.cash_offered < trade.cash_requested + partner_interest) {
        return Trade_State::FAIL_NO_FUNDS;
    }
    if (trade.jail_cards_offered > trade.proposer->get_jail_cards() || trade.jail_cards_requested > trade.partner->get_jail_cards()) {
        return Trade_State::CANCELLED;
    }

    // credit both sides before debiting them, the swap happens at the same moment
    trade.proposer->add_balance(trade.cash_requested);
    trade.partner->add_balance(trade.cash_offered);
    trade.proposer->withdraw_balance(trade.cash_offered + proposer_interest);
    trade.partner->withdraw_balance(trade.cash_requested + partner_interest);
    pay_bank(proposer_interest + partner_interest);

    for (Property* property : trade.offered) {
        trade.proposer->remove_asset(property);
        bool full_set = false;
        trade.partner->add_asset(property, full_set);
        property->set_owner(trade.partner);
    }
    for (Property* property : trade.requested) {
        trade.partner->remove_asset(property);
        bool full_set = false;
        trade.proposer->add_asset(property, full_set);
        property->set_owner(trade.proposer);
    }
    for (unsigned int i = 0; i < trade.jail_cards_offered; i++) {
        Deck_Type deck = trade.proposer->has_jail_card(Deck_Type::CHANCE) ? Deck_Type::CHANCE : Deck_Type::COMMUNITY_CHEST;
        if (trade.proposer->consume_jail_card(deck)) {
            trade.partner->add_jail_card(deck);
        }
    }
    for (unsigned int i = 0; i < trade.jail_cards_requested; i++) {
        Deck_Type deck = trade.partner->has_jail_card(Deck_Type::CHANCE) ? Deck_Type::CHANCE : Deck_Type::COMMUNITY_CHEST;
        if (trade.partner->consume_jail_card(deck)) {
            trade.proposer->add_jail_card(deck);
        }
    }
    for (Property* property : trade.offered) {
        update_rents_for_color_set(property->get_color_set());
    }
    for (Property* property : trade.requested) {
        update_rents_for_color_set(property->get_color_set());
    }
    return Trade_State::SUCCESS;
}

void GameContext::take_to_jail() {
    unsigned int location = get_player_location(current_player);
    get_tiles()[location]->exit_path(current_player->get_pawn());
    current_player->set_in_jail(true);
    current_player->reset_rounds_in_jail();
    current_player->reset_consecutive_doubles();
    current_player->set_rolled_double(false);
    Hybrid* jail = dynamic_cast<Hybrid*>(get_tiles()[jail_index]);
    if (jail) {
        jail->send_to_jail(current_player->get_pawn());
    }
    players_locations[current_player] = jail_index;
    set_current_tile(get_tiles()[jail_index]);
}

void GameContext::release_from_jail() {
    current_player->set_in_jail(false);
    current_player->reset_rounds_in_jail();
    players_locations[current_player] = jail_index;
    Hybrid* jail = dynamic_cast<Hybrid*>(get_tiles()[jail_index]);
    if (jail) {
        jail->release_from_jail(current_player->get_pawn());
    }
    get_tiles()[jail_index]->enter_path(current_player->get_pawn());
    set_current_tile(get_tiles()[jail_index]);
}

bool GameContext::use_jail_card(Player* player) {
    if (!player) {
        return false;
    }
    if (player->consume_jail_card(Deck_Type::CHANCE)) {
        return_jail_card(Deck_Type::CHANCE);
        return true;
    }
    if (player->consume_jail_card(Deck_Type::COMMUNITY_CHEST)) {
        return_jail_card(Deck_Type::COMMUNITY_CHEST);
        return true;
    }
    return false;
}

std::string GameContext::draw_chance() {
    return draw_card(chance_deck, chance_index);
}

std::string GameContext::draw_community_chest() {
    return draw_card(community_chest_deck, community_chest_index);
}

std::string GameContext::draw_card(std::vector<Card>& deck, unsigned int& index) {
    if (deck.empty()) {
        return "The deck is empty.";
    }
    for (unsigned int offset = 0; offset < deck.size(); offset++) {
        Card& card = deck[(index + offset) % deck.size()];
        // a jail card sitting in somebody's hand is out of the pile
        if (card.jail_card && card.held) {
            continue;
        }
        index = (index + offset + 1) % deck.size();
        return card.action();
    }
    return "The deck is empty.";
}

void GameContext::return_jail_card(Deck_Type deck) {
    std::vector<Card>& cards = (deck == Deck_Type::CHANCE) ? chance_deck : community_chest_deck;
    for (Card& card : cards) {
        if (card.jail_card) {
            card.held = false;
            return;
        }
    }
}

const bool GameContext::get_advanced() const {
    return advanced;
}

void GameContext::create_chance_deck() {
    chance_deck = {
        { [&]() { return advanceToTile(get_tiles()[5], 5, "Take a trip to Reading Railroad. If you pass GO, collect $200.", true, go_bonus); }, false, false },
        { [&]() { return advanceToNearestUtility(true, go_bonus, "Advance to the nearest utility. If unowned, you may buy it from the bank. If owned, throw dice and pay the owner 10 times the amount thrown."); }, false, false },
        { [&]() { return collect(150, "Your building loan matures. Collect $150."); }, false, false },
        { [&]() { return collect(50, "Bank dividend. Collect $50."); }, false, false },
        { [&]() { return advanceToNearestRailroad(true, go_bonus, "Advance to the nearest railroad. If unowned, you may buy it from the bank. If owned, pay the owner twice the rental to which they are otherwise entitled."); }, false, false },
        { [&]() { return advanceToTile(get_tiles()[39], 39, "Advance to Boardwalk.", false, 0); }, false, false },
        { [&]() { return advanceToTile(get_tiles()[11], 11, "Advance to St. Charles Place. If you pass GO, collect $200.", true, go_bonus); }, false, false },
        { [&]() { return repairs(25, 100, "Make general repairs on all your property: For each house, pay $25. For each hotel, pay $100."); }, false, false },
        { [&]() { return goToJail("Go to jail. Go directly to jail. Do not pass GO. Do not collect $200."); }, false, false },
        { [&]() { return advanceToGo(go_bonus, "Advance to GO. Collect $200."); }, false, false },
        { [&]() { return advanceToTile(get_tiles()[24], 24, "Advance to Illinois Avenue. If you pass GO, collect $200.", true, go_bonus); }, false, false },
        { [&]() { return moveBack(3, "Go back 3 spaces."); }, false, false },
        { [&]() { return pay(15, "Speeding fine. Pay $15.", false); }, false, false },
        { [&]() { return pay(50, "You have been elected Chairman of the Board. Pay each player $50.", true); }, false, false },
        { [&]() { return advanceToNearestRailroad(true, go_bonus, "Advance to the nearest railroad. If unowned, you may buy it from the bank. If owned, pay the owner twice the rental to which they are otherwise entitled."); }, false, false },
        { [&]() { return getOutOfJailFree(Deck_Type::CHANCE, "Get out of jail free. This card may be kept until needed or sold."); }, true, false }
    };
    std::shuffle(chance_deck.begin(), chance_deck.end(), std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
}

void GameContext::create_community_chest_deck() {
    community_chest_deck = {
        { [&]() { return collect(20, "You help your neighbor bring in her groceries. She makes you lunch to say thanks! Collect $20."); }, false, false },
        { [&]() { return repairs(40, 115, "You should have volunteered for that home improvement project - you would have learned valuable skills! For each house you own, pay $40. For each hotel, pay $115."); }, false, false },
        { [&]() { return pay(50, "You buy a few bags of cookies from the school bake sale. Yum! Pay $50.", false); }, false, false },
        { [&]() { return collect(10, "You volunteer at a blood drive. There are free cookies! Collect $10."); }, false, false },
        { [&]() { return collect(200, "You help your neighbors clean up their yards after a storm. Collect $200."); }, false, false },
        { [&]() { return getOutOfJailFree(Deck_Type::COMMUNITY_CHEST, "You rescue a puppy - and you feel rescued, too! Get out of jail free."); }, true, false },
        { [&]() { return collect(25, "You organize a bake sale for your local school. Collect $25."); }, false, false },
        { [&]() { return pay(100, "You go to the local school's car wash fundraiser - but you forget to close your windows! Pay $100.", false); }, false, false },
        { [&]() { return collect(100, "You help build a new school playground - then you get to test the slide! Collect $100."); }, false, false },
        { [&]() { return pay(50, "Your fuzzy friends at the animal shelter will be thankful for your donation. Pay $50.", false); }, false, false },
        { [&]() { return collect(100, "You set aside time every week to hang out with your elderly neighbor - you've heard some amazing stories! Collect $100."); }, false, false },
        { [&]() { return goToJail("Blasting music late at night? Your neighbors do not approve. Go to jail."); }, false, false },
        { [&]() { return collect(100, "You spend the day playing games with kids at a local children's hospital. Collect $100."); }, false, false },
        { [&]() { return advanceToGo(go_bonus, "Just when you think you can't go another step, you finish that foot race - and raise money for your local hospital! Advance to GO. Collect $200."); }, false, false },
        { [&]() { return collect(50, "You organize a group to clean up your town's walking path. Collect $50."); }, false, false },
        { [&]() { return collect_from_each(10, "You organize a block party so people on your street can get to know each other. Collect $10 from each player."); }, false, false }
    };
    std::shuffle(community_chest_deck.begin(), community_chest_deck.end(), std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count()));
}

unsigned int GameContext::get_turn() {
    return turn;
}

void GameContext::next_player() {
    unsigned int total_players = players.size();
    for (unsigned int i = 0; i < total_players; i++) {
        turn = (turn + 1) % total_players;
        if (players[turn]->is_active()) {
            current_player = players[turn];
            start_turn();
            return;
        }
    }
}

void GameContext::start_turn() {
    clear_pending_rent();
    clear_pending_debt();
    pending_collections.clear();
    if (get_active_players().size() < 2) {
        pending_bank_auctions.clear();
    }
    advanced = false;
    set_current_tile(get_tiles()[get_player_location(current_player)]);
}


std::string GameContext::collect(unsigned int amount, const std::string& message) {
    advanced = false;
    current_player->add_balance(amount);
    return message;
}

std::string GameContext::collect_from_each(unsigned int amount, const std::string& message) {
    advanced = false;
    charge_opponents(current_player, amount);
    return message;
}

std::string GameContext::pay(unsigned int amount, const std::string& message, bool all_players) {
    advanced = false;
    if (all_players) {
        unsigned int total_due = amount * get_opponents(current_player).size();
        charge(current_player, nullptr, total_due, true);
    } else {
        charge(current_player, nullptr, amount);
    }
    return message;
}

std::string GameContext::repairs(unsigned int per_house, unsigned int per_hotel, const std::string& message) {
    advanced = false;
    unsigned int total = current_player->get_num_houses() * per_house + current_player->get_num_hotels() * per_hotel;
    if (total > 0) {
        charge(current_player, nullptr, total);
    }
    return message;
}

std::string GameContext::advanceToTile(Tile* tile, unsigned int tile_index, const std::string& message, bool passGo, unsigned int goBonus) {
    advanced = true;
    move_player_to_tile(tile, tile_index, passGo, goBonus);
    return message;
}

std::string GameContext::advanceToNearestRailroad(bool passGo, unsigned int goBonus, const std::string& message) {
    advanced = true;
    unsigned int player_location = get_player_location(current_player);
    unsigned int target = 5;
    if (player_location >= 5 && player_location < 15){
        target = 15;
    } else if (player_location >= 15 && player_location < 25){
        target = 25;
    } else if (player_location >= 25 && player_location < 35){
        target = 35;
    }
    move_player_to_tile(get_tiles()[target], target, passGo, goBonus);
    Railroad* railroad = dynamic_cast<Railroad*>(get_tiles()[target]);
    if (railroad && railroad->get_owner() && railroad->get_owner() != current_player && !railroad->get_mortgaged()) {
        // this card charges double the rent the owner would normally collect
        pending_rent = 2 * railroad->get_rent(railroad->get_owner()->get_num_railroads());
    }
    return message;
}

std::string GameContext::advanceToNearestUtility(bool passGo, unsigned int goBonus, const std::string& message) {
    advanced = true;
    unsigned int player_location = get_player_location(current_player);
    unsigned int target = (player_location >= 12 && player_location < 28) ? 28 : 12;
    move_player_to_tile(get_tiles()[target], target, passGo, goBonus);
    Utility* utility = dynamic_cast<Utility*>(get_tiles()[target]);
    if (utility && utility->get_owner() && utility->get_owner() != current_player && !utility->get_mortgaged()) {
        // the card asks for a fresh throw, worth ten times whatever comes up
        dies[0]->roll();
        dies[1]->roll();
        pending_rent = 10 * (dies[0]->get_rolled() + dies[1]->get_rolled());
    }
    return message;
}

std::string GameContext::advanceToGo(unsigned int goBonus, const std::string& message) {
    advanced = true;
    move_player_to_tile(get_tiles()[0], 0, true, goBonus);
    return message;
}

std::string GameContext::goToJail(const std::string& message) {
    advanced = false;
    take_to_jail();
    return message;
}

std::string GameContext::moveBack(unsigned int spaces, const std::string& message) {
    advanced = true;
    unsigned int player_location = get_player_location(current_player);
    unsigned int steps = spaces % Board::board_size;
    unsigned int new_location = (player_location + Board::board_size - steps) % Board::board_size;
    get_tiles()[player_location]->exit_path(current_player->get_pawn());
    players_locations[current_player] = new_location;
    get_tiles()[new_location]->enter_path(current_player->get_pawn());
    set_current_tile(get_tiles()[new_location]);
    return message;
}

std::string GameContext::getOutOfJailFree(Deck_Type deck, const std::string& message) {
    advanced = false;
    std::vector<Card>& cards = (deck == Deck_Type::CHANCE) ? chance_deck : community_chest_deck;
    for (Card& card : cards) {
        if (card.jail_card) {
            card.held = true;
            break;
        }
    }
    current_player->add_jail_card(deck);
    return message;
}
