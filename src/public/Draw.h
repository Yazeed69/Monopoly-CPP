#ifndef DRAW_H
#define DRAW_H

#include "Tile.h"
#include <vector>

class Draw : public Tile {
    public:
        Draw();
        Draw(const std::string& name, const std::string& abbreviation, const std::string& desc, bool type, const std::string& UI_color);

        Tile_State Action() override;
        Tile_State Action(Player* player) override;

    private:
        // 0 = chance, 1 = community chest
        bool type;
};

#endif