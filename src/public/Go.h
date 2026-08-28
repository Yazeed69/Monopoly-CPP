#ifndef GO_H
#define GO_H

#include "Tile.h"

class Go : public Tile {
    public:
        Go();
        Go(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color);

        Tile_State Action() override;
        Tile_State Action(Player* player) override;

    private:
};

#endif