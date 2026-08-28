#include "FreeParking.h"

FreeParking::FreeParking(bool using_pot) : Tile() {
    this->using_pot = using_pot;
    pot = 0;
}

FreeParking::FreeParking(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color, bool using_pot) : Tile(name, abbreviation, desc, UI_color) {
    this->using_pot = using_pot;
    pot = 0;
}

int FreeParking::get_pot() {
    int temp = pot;
    pot = 0;
    return temp;
}

int FreeParking::peek_pot() const {
    return pot;
}

void FreeParking::add_to_pot(int amount) {
    if (using_pot && amount > 0) {
        pot += amount;
    }
}

bool FreeParking::get_using_pot() const {
    return using_pot;
}

void FreeParking::set_using_pot(bool using_pot) {
    this->using_pot = using_pot;
}

Tile_State FreeParking::Action() {
    return (using_pot)? Tile_State::FREE_POT : Tile_State::FREE;
}

Tile_State FreeParking::Action(Player* player) {
    return (using_pot)? Tile_State::FREE_POT : Tile_State::FREE;
}
