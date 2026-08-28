#include "Player.h"
#include "Railroad.h"
#include "Utility.h"
#include "Street.h"

#include <memory>

unsigned int Player::active_players = 0;

Player::Player() {
    active_players++;
    name = "";
    pawn = ' ';
    balance = 0;
    in_jail = false;
    roll = 0;
    assets = std::vector<Tile*>();
    last_paid_rent = 0;
    last_tax_paid = 0;
    last_roll = 0;
    rolled_double = false;
    consecutive_doubles = 0;
    bookkeeper = std::make_unique<Bookkeeper>();
    num_railroads = 0;
    num_utilities = 0;
    rounds_in_jail = 0;
    chance_jail_cards = 0;
    community_jail_cards = 0;
    active = true;
}

Player::Player(const std::string& name, const char pawn, const unsigned int balance) {
    active_players++;
    this->name = name;
    this->pawn = pawn;
    this->balance = balance;
    in_jail = false;
    roll = 0;
    assets = std::vector<Tile*>();
    last_paid_rent = 0;
    last_tax_paid = 0;
    last_roll = 0;
    rolled_double = false;
    consecutive_doubles = 0;
    bookkeeper = std::make_unique<Bookkeeper>();
    num_railroads = 0;
    num_utilities = 0;
    rounds_in_jail = 0;
    chance_jail_cards = 0;
    community_jail_cards = 0;
    active = true;
}

Player::Player(const Player& other) {
    name = other.name;
    pawn = other.pawn;
    balance = other.balance;
    in_jail = other.in_jail;
    roll = other.roll;
    assets = other.assets;
    last_paid_rent = other.last_paid_rent;
    last_tax_paid = other.last_tax_paid;
    last_roll = other.last_roll;
    rolled_double = other.rolled_double;
    consecutive_doubles = other.consecutive_doubles;
    num_railroads = other.num_railroads;
    num_utilities = other.num_utilities;
    rounds_in_jail = other.rounds_in_jail;
    chance_jail_cards = other.chance_jail_cards;
    community_jail_cards = other.community_jail_cards;
    active = other.active;
    bookkeeper = std::make_unique<Bookkeeper>(*other.bookkeeper);
    if (active) {
        active_players++;
    }
}

Player& Player::operator=(const Player& other) {
    if (this != &other) {
        name = other.name;
        pawn = other.pawn;
        balance = other.balance;
        in_jail = other.in_jail;
        roll = other.roll;
        assets = other.assets;
        last_paid_rent = other.last_paid_rent;
        last_tax_paid = other.last_tax_paid;
        last_roll = other.last_roll;
        rolled_double = other.rolled_double;
        consecutive_doubles = other.consecutive_doubles;
        num_railroads = other.num_railroads;
        num_utilities = other.num_utilities;
        rounds_in_jail = other.rounds_in_jail;
        chance_jail_cards = other.chance_jail_cards;
        community_jail_cards = other.community_jail_cards;
        set_active(other.active);
        bookkeeper = std::make_unique<Bookkeeper>(*other.bookkeeper);
    }
    return *this;
}

Player::Player(Player&& other) noexcept {
    name = other.name;
    pawn = other.pawn;
    balance = other.balance;
    in_jail = other.in_jail;
    roll = other.roll;
    assets = other.assets;
    last_paid_rent = other.last_paid_rent;
    last_tax_paid = other.last_tax_paid;
    last_roll = other.last_roll;
    rolled_double = other.rolled_double;
    consecutive_doubles = other.consecutive_doubles;
    num_railroads = other.num_railroads;
    num_utilities = other.num_utilities;
    rounds_in_jail = other.rounds_in_jail;
    chance_jail_cards = other.chance_jail_cards;
    community_jail_cards = other.community_jail_cards;
    active = other.active;
    bookkeeper = std::move(other.bookkeeper);
    // the moved-from player no longer holds a seat at the table
    other.active = false;
    other.assets.clear();
}

Player& Player::operator=(Player&& other) noexcept {
    if (this != &other) {
        name = other.name;
        pawn = other.pawn;
        balance = other.balance;
        in_jail = other.in_jail;
        roll = other.roll;
        assets = other.assets;
        last_paid_rent = other.last_paid_rent;
        last_tax_paid = other.last_tax_paid;
        last_roll = other.last_roll;
        rolled_double = other.rolled_double;
        consecutive_doubles = other.consecutive_doubles;
        num_railroads = other.num_railroads;
        num_utilities = other.num_utilities;
        rounds_in_jail = other.rounds_in_jail;
        chance_jail_cards = other.chance_jail_cards;
        community_jail_cards = other.community_jail_cards;
        set_active(other.active);
        bookkeeper = std::move(other.bookkeeper);
        other.active = false;
        other.assets.clear();
    }
    return *this;
}

Player::~Player() {
    clear_assets();
    if (active && active_players > 0) {
        active_players--;
    }
}

std::string Player::get_name() const {
    return name;
}

char Player::get_pawn() const {
    return pawn;
}

unsigned int Player::get_balance() const {
    return balance;
}

void Player::set_name(const std::string& name) {
    this->name = name;
}

void Player::set_pawn(const char pawn) {
    this->pawn = pawn;
}

void Player::set_balance(const unsigned int balance) {
    this->balance = balance;
}

void Player::add_balance(const unsigned int balance) {
    this->balance += balance;
}

bool Player::withdraw_balance(const unsigned int balance) {
    if (this->balance < balance) {
        return false;
    }
    this->balance -= balance;
    return true;
}

Bookkeeper* Player::get_bookkeeper() const {
    return bookkeeper.get();
}

std::vector<Tile*> Player::get_assets() const {
    return assets;
}

bool Player::get_in_jail() const {
    return in_jail;
}

unsigned int Player::get_roll() const {
    return roll;
}

void Player::set_in_jail(const bool in_jail) {
    this->in_jail = in_jail;
}

void Player::set_roll(const unsigned int roll) {
    last_roll = this->roll;
    this->roll = roll;
}

void Player::add_asset(Tile* asset) {
    bool full_set = false;
    add_asset(asset, full_set);
}

void Player::add_asset(Tile* asset, bool& full_set) {
    Property* property = dynamic_cast<Property*>(asset);
    if (!property) {
        return;
    }
    if (std::find(assets.begin(), assets.end(), asset) != assets.end()) {
        return;
    }
    std::string color_set = property->get_color_set();
    if (color_set == "Railroad") {
        num_railroads++;
    } else if (color_set == "Utility") {
        num_utilities++;
    }
    bookkeeper->add_property(color_set);
    if (bookkeeper->is_full_set(color_set)) {
        full_set = true;
    }
    assets.push_back(asset);
}


void Player::remove_asset(const Tile* asset) {
    auto iterator = std::find(assets.begin(), assets.end(), asset);
    if (iterator != assets.end()) {
        Property* property = dynamic_cast<Property*>(*iterator);
        if (property) {
            const std::string color_set = property->get_color_set();
            if (color_set == "Railroad" && num_railroads > 0) {
                num_railroads--;
            } else if (color_set == "Utility" && num_utilities > 0) {
                num_utilities--;
            }
            bookkeeper->remove_property(color_set);
        }
        assets.erase(iterator);
    }
}

void Player::clear_assets() {
    assets.clear();
}

std::string Player::assets_string() const {
    std::ostringstream oss;
    for (auto asset : assets) {
        const Property* property = dynamic_cast<const Property*>(asset);
        oss << asset->get_abbreviation();
        // a trailing star marks a deed that is currently mortgaged
        if (property && property->get_mortgaged()) {
            oss << "\033[33m*\033[0m";
        }
        oss << " ";
    }
    return oss.str();
}

bool Player::get_rolled_double() const {
    return rolled_double;
}

void Player::set_rolled_double(const bool rolled_double) {
    this->rolled_double = rolled_double;
}

unsigned int Player::get_consecutive_doubles() const {
    return consecutive_doubles;
}

void Player::increment_consecutive_doubles() {
    consecutive_doubles++;
}

void Player::reset_consecutive_doubles() {
    consecutive_doubles = 0;
}

unsigned int Player::get_rounds_in_jail() const {
    return rounds_in_jail;
}

void Player::increment_rounds_in_jail() {
    rounds_in_jail++;
}

void Player::reset_rounds_in_jail() {
    rounds_in_jail = 0;
}

unsigned int Player::get_last_paid_rent() const
{
    return last_paid_rent;
}

unsigned int Player::get_last_tax_paid() const {
    return last_tax_paid;
}

unsigned int Player::get_last_roll() const {
    return last_roll;
}

void Player::set_last_paid_rent(const unsigned int last_paid_rent)
{
    this->last_paid_rent = last_paid_rent;
}

void Player::set_last_tax_paid(const unsigned int last_tax_paid) {
    this->last_tax_paid = last_tax_paid;
}

void Player::set_last_roll(const unsigned int last_roll) {
    this->last_roll = last_roll;
}

void Player::set_draw_card(const std::string &card_text) {
    this->card_text = card_text;
}

std::string Player::get_draw_card() const {
    return card_text;
}

unsigned int Player::get_num_railroads() const
{
    return num_railroads;
}

unsigned int Player::get_num_utilities() const {
    return num_utilities;
}

unsigned int Player::get_num_houses() const {
    unsigned int houses = 0;
    for (Tile* asset : assets) {
        Street* street = dynamic_cast<Street*>(asset);
        // a street sitting on five buildings is a hotel, not five houses
        if (street && street->get_num_houses() < 5) {
            houses += street->get_num_houses();
        }
    }
    return houses;
}

unsigned int Player::get_num_hotels() const {
    unsigned int hotels = 0;
    for (Tile* asset : assets) {
        Street* street = dynamic_cast<Street*>(asset);
        if (street) {
            hotels += street->get_num_hotels();
        }
    }
    return hotels;
}

unsigned int Player::net_worth() const {
    unsigned int value = balance;
    for (Tile* asset : assets) {
        Property* property = dynamic_cast<Property*>(asset);
        if (!property) {
            continue;
        }
        value += (property->get_mortgaged()) ? property->get_mortgage_value() : property->get_purchase_price();
        Street* street = dynamic_cast<Street*>(asset);
        if (street) {
            value += street->get_num_houses() * street->get_build_cost();
        }
    }
    return value;
}

void Player::add_jail_card(const Deck_Type deck) {
    if (deck == Deck_Type::CHANCE) {
        chance_jail_cards++;
    } else {
        community_jail_cards++;
    }
}

bool Player::consume_jail_card(const Deck_Type deck) {
    if (deck == Deck_Type::CHANCE) {
        if (chance_jail_cards == 0) {
            return false;
        }
        chance_jail_cards--;
        return true;
    }
    if (community_jail_cards == 0) {
        return false;
    }
    community_jail_cards--;
    return true;
}

bool Player::has_jail_card(const Deck_Type deck) const {
    return (deck == Deck_Type::CHANCE) ? chance_jail_cards > 0 : community_jail_cards > 0;
}

bool Player::has_any_jail_card() const {
    return chance_jail_cards > 0 || community_jail_cards > 0;
}

unsigned int Player::get_jail_cards() const {
    return chance_jail_cards + community_jail_cards;
}

bool Player::is_active() const {
    return active;
}

void Player::set_active(bool is_active) {
    if (active && !is_active && active_players > 0) {
        active_players--;
    } else if (!active && is_active) {
        active_players++;
    }
    active = is_active;
}

unsigned int Player::get_active_players() {
    return active_players;
}

bool Player::comparePlayersByRoll(const Player* a, const Player* b) {
    return a->get_roll() < b->get_roll();
}
