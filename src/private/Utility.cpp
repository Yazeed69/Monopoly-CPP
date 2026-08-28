#include "Utility.h"
#include "Player.h"


Utility::Utility() : Property() {
    rent_multiplier[0] = 4;
    rent_multiplier[1] = 10;
}

Utility::Utility(const std::string& name, const std::string& abbreviation, const std::string& color_set, const std::string& desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, const std::string& UI_color) : Property(name, abbreviation, color_set, desc, purchase_price, mortgage_value, unmortgage_value, UI_color) {
    rent_multiplier[0] = 4;
    rent_multiplier[1] = 10;
}

unsigned int Utility::get_rent(unsigned int num_utilities, unsigned int roll) const {
    return get_multiplier(num_utilities) * roll;
}

unsigned int Utility::get_multiplier(unsigned int num_utilities) const {
    if (get_mortgaged() || num_utilities == 0) {
        return 0;
    }
    if (num_utilities > 2) {
        num_utilities = 2;
    }
    return rent_multiplier[num_utilities - 1];
}

Tile_State Utility::Action() {
    return ((get_owner()) == nullptr) ? Tile_State::UNOWNED : Tile_State::OWNED_UTILITY;
}

Tile_State Utility::Action(Player* player) {
    if (get_owner() != nullptr){
        if (player == get_owner()){
            return Tile_State::OWN_PROPERTY;
        } else if (get_mortgaged()) {
            return Tile_State::MORTGAGED;
        } else {
            return Tile_State::OWNED_UTILITY;
        }
    } else {
        return Tile_State::UNOWNED;
    }
}
