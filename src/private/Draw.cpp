#include "Draw.h"


Draw::Draw() {
    type = false;
}

Draw::Draw(const std::string& name, const std::string& abbreviation, const std::string& desc, bool type, const std::string& UI_color) : Tile(name, abbreviation, desc, UI_color) {
    this->type = type;
}

Tile_State Draw::Action() {
    return (type)? Tile_State::DRAW_CC : Tile_State::DRAW_CHANCE;
}

Tile_State Draw::Action(Player *player) {
    return (type)? Tile_State::DRAW_CC : Tile_State::DRAW_CHANCE;
}
