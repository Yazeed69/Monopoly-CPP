#include "Railroad.h"
#include "Player.h"

Railroad::Railroad() : Property() {
    rent[0] = 25;
    rent[1] = 50;
    rent[2] = 100;
    rent[3] = 200;
}

Railroad::Railroad(const std::string& name, const std::string abbreviation, const std::string color_set, const std::string desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, const std::string& UI_color) : Property(name, abbreviation, color_set, desc, purchase_price, mortgage_value, unmortgage_value, UI_color) {
    rent[0] = 25;
    rent[1] = 50;
    rent[2] = 100;
    rent[3] = 200;
}

unsigned int Railroad::get_rent(unsigned int num_railroads) const {
    if (get_mortgaged() || num_railroads == 0) {
        return 0;
    }
    if (num_railroads > 4) {
        num_railroads = 4;
    }
    return rent[num_railroads - 1];
}

void Railroad::set_rent(unsigned int rent, unsigned int num_railroads) {
    if (num_railroads == 0 || num_railroads > 4) {
        return;
    }
    this->rent[num_railroads - 1] = rent;
}

Tile_State Railroad::Action() {
    return ((get_owner()) == nullptr) ? Tile_State::UNOWNED : Tile_State::OWNED_RAILROAD;
}

Tile_State Railroad::Action(Player* player) {
    if (get_owner() != nullptr){
        if (player == get_owner()){
            return Tile_State::OWN_PROPERTY;
        } else if (get_mortgaged()) {
            return Tile_State::MORTGAGED;
        } else {
            return Tile_State::OWNED_RAILROAD;
        }
    } else {
        return Tile_State::UNOWNED;
    }
}
