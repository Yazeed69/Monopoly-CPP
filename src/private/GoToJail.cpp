#include "GoToJail.h"


GoToJail::GoToJail() : Tile() {
    
}


GoToJail::GoToJail(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color) : Tile(name, abbreviation, desc, UI_color) {
    
}

Tile_State GoToJail::Action() {
    return Tile_State::TO_JAIL;
}

Tile_State GoToJail::Action(Player* player) {
    return Tile_State::TO_JAIL;
}