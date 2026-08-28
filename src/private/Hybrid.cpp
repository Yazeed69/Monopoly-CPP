#include "Hybrid.h"


Hybrid::Hybrid() {
    players_in_jail = 0;
    update_jail();
}

Hybrid::Hybrid(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color) : Tile(name, abbreviation, desc, UI_color) {
    players_in_jail = 0;
    update_jail();
}

void Hybrid::send_to_jail(char pawn) {
    if (pawns_in_jail.find(pawn) != std::string::npos) {
        return;
    }
    if (players_in_jail < max_players_in_jail) {
        pawns_in_jail += pawn;
        players_in_jail++;
        update_jail();
    }
}

void Hybrid::release_from_jail(char pawn) {
    std::string::size_type position = pawns_in_jail.find(pawn);
    if (position == std::string::npos) {
        return;
    }
    pawns_in_jail.erase(position, 1);
    players_in_jail--;
    update_jail();
}

void Hybrid::update_jail(){
    std::string gap(max_players_in_jail - players_in_jail, ' ');
    set_abbreviation(gap + "{" + pawns_in_jail + "}");
}

Tile_State Hybrid::Action() {
    return Tile_State::HYBRID;
}

Tile_State Hybrid::Action(Player* player) {
    return Tile_State::HYBRID;
}
