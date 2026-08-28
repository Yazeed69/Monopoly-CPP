#include "Tax.h"


Tax::Tax() : Tile() {
    tax = 0;
}

Tax::Tax(const std::string& name, const std::string& abbreviation, const std::string& desc, int tax, const std::string& UI_color) : Tile(name, abbreviation, desc, UI_color) {
    this->tax = tax;
}

unsigned int Tax::get_tax() const {
    return tax;
}

Tile_State Tax::Action() {
    return Tile_State::TAX;
}

Tile_State Tax::Action(Player* player) {
    return Tile_State::TAX;
}