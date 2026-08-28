#include "Tile.h"

Tile::Tile() {
    name = "";
    abbreviation = "";
    desc = "";
    path = "    ";
    state = Tile_State::FREE;
    this->UI_color = "";
}

Tile::Tile(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color) {
    this->name = UI_color + name + "\033[0m";
    this->abbreviation = UI_color + abbreviation + "\033[0m";
    this->desc = desc;
    path = "    ";
    state = Tile_State::FREE;
    this->UI_color = UI_color;
}

Tile::~Tile() {

}

const std::string& Tile::get_name() const {
    return name;
}

const std::string& Tile::get_abbreviation() const {
    return abbreviation;
}

const std::string& Tile::get_desc() const {
    return desc;
}

void Tile::set_name(const std::string& name) {
    this->name = name;
}

void Tile::set_abbreviation(const std::string& abbreviation) {
    this->abbreviation = abbreviation;
}

void Tile::set_desc(const std::string& desc) {
    this->desc = desc;
}

const std::string& Tile::get_path() const {
    return path;
}

void Tile::enter_path(char pawn) {
    for (std::string::size_type i = 0; i < path.size(); i++) {
        if (path[i] == pawn) {
            return;
        }
    }
    for (std::string::size_type i = 0; i < path.size(); i++) {
        if (path[i] == ' ') {
            path[i] = pawn;
            return;
        }
    }
}

void Tile::exit_path(char pawn){
    for (std::string::size_type i = 0; i < path.size(); i++){
        if (path[i] == pawn){
            path[i] = ' ';
            return;
        }
    }
}

Tile_State Tile::get_state() const {
    return state;
}

void Tile::set_state(Tile_State state) {
    this->state = state;
}

const std::string& Tile::get_UI_color() const {
    return UI_color;
}
