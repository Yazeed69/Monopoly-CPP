#ifndef GOTOJAIL_H
#define GOTOJAIL_H

#include "Tile.h"

class GoToJail : public Tile {
    public:
        GoToJail();
        GoToJail(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color);
        
        Tile_State Action() override;
        Tile_State Action(Player* player) override;

    private:
};

#endif