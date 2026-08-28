#include "Go.h"

Go::Go(){

}

Go::Go(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color) : Tile(name, abbreviation, desc, UI_color){

}

Tile_State Go::Action() {
    return Tile_State::GO;
}

Tile_State Go::Action(Player* player) {
    return Tile_State::GO;
}