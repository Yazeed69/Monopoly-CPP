#include "Street.h"
#include "Player.h"


Street::Street() : Property() {
    this->hosues = "  ";
    this->num_houses = 0;
    this->build_cost = 0;
    this->base_rent = 0;
    this->full_set_rent = 0;
    this->rent_1_house = 0;
    this->rent_2_houses = 0;
    this->rent_3_houses = 0;
    this->rent_4_houses = 0;
    this->rent_hotel = 0;
    this->rent = 0;
    this->full_set = false;
}

Street::Street(const std::string& name, const std::string& abbreviation, const std::string& color_set, const std::string& desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, unsigned int build_cost, unsigned int base_rent, unsigned int full_set_rent, unsigned int rent_1_house, unsigned int rent_2_houses, unsigned int rent_3_houses, unsigned int rent_4_houses, unsigned int rent_hotel, const std::string& UI_color) : Property(name, abbreviation, color_set, desc, purchase_price, mortgage_value, unmortgage_value, UI_color) {
    this->hosues = "  ";
    this->num_houses = 0; 
    this->build_cost = build_cost;
    this->base_rent = base_rent;
    this->full_set_rent = full_set_rent;
    this->rent_1_house = rent_1_house;
    this->rent_2_houses = rent_2_houses;
    this->rent_3_houses = rent_3_houses;
    this->rent_4_houses = rent_4_houses;
    this->rent_hotel = rent_hotel;
    this->rent = base_rent;
    this->full_set = false;
}

unsigned int Street::get_num_houses() const {
    return num_houses;
}

unsigned int Street::get_num_hotels() const {
    return num_houses / 5;
}

unsigned int Street::get_build_cost() const {
    return build_cost;
}

unsigned int Street::get_sell_price() const {
    return build_cost / 2;
}

unsigned int Street::get_rent() const {
    return rent;
}

unsigned int Street::get_base_rent() const {
    return base_rent;
}

unsigned int Street::get_full_set_rent() {
    return full_set_rent;
}

const std::string &Street::get_houses() const {
    return hosues;
}

bool Street::get_full_set() const {
    return full_set;
}

void Street::set_full_set(const bool full_set) {
    this->full_set = full_set;
    refresh_rent();
}

void Street::refresh_rent() {
    refresh_houses();
    if (get_mortgaged()) {
        rent = 0;
        return;
    }
    switch (num_houses) {
        case 0:
            rent = (full_set) ? full_set_rent : base_rent;
            break;
        case 1:
            rent = rent_1_house;
            break;
        case 2:
            rent = rent_2_houses;
            break;
        case 3:
            rent = rent_3_houses;
            break;
        case 4:
            rent = rent_4_houses;
            break;
        default:
            rent = rent_hotel;
            break;
    }
}

void Street::refresh_houses() {
    switch (num_houses) {
        // houses green color text
        // hotels red color text
        case 1:
            hosues = "\033[32m. \033[0m";
            break;
        case 2:
            hosues = "\033[32m..\033[0m";
            break;
        case 3:
            hosues = "\033[32m:.\033[0m";
            break;
        case 4:
            hosues = "\033[32m::\033[0m";
            break;
        case 5:
            hosues = "\033[31mHH\033[0m";
            break;
        default:
            hosues = (get_mortgaged()) ? "\033[33mMM\033[0m" : "  ";
            break;
    }
}

void Street::build() {
    if (num_houses >= 5) {
        return;
    }
    num_houses++;
    refresh_rent();
}

void Street::sell() {
    if (num_houses == 0) {
        return;
    }
    num_houses--;
    refresh_rent();
}

void Street::reset_buildings() {
    num_houses = 0;
    refresh_rent();
}

Tile_State Street::Action() {
    return ((get_owner()) == nullptr) ? Tile_State::UNOWNED : Tile_State::OWNED_STREET;
}

void Street::set_rent(unsigned int rent) {
    this->rent = rent;
}


Tile_State Street::Action(Player* player) {
    if (get_owner() != nullptr){
        if (player == get_owner()){
            return Tile_State::OWN_PROPERTY;
        } else if (get_mortgaged()) {
            return Tile_State::MORTGAGED;
        } else {
            return Tile_State::OWNED_STREET;
        }
    } else {
        return Tile_State::UNOWNED;
    }
}
