#ifndef HYBRID_H
#define HYBRID_H

#include "Tile.h"

class Hybrid : public Tile {
    public:
        Hybrid();
        Hybrid(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color);
        
        void send_to_jail(char pawn);
        void release_from_jail(char pawn);

        Tile_State Action() override;
        Tile_State Action(Player* player) override;

    private:
        void update_jail();
        std::string pawns_in_jail;
        int players_in_jail;
        const int max_players_in_jail = 4;
};

#endif