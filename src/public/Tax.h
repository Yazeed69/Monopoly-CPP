#ifndef TAX_H
#define TAX_H

#include "Tile.h"

class Tax : public Tile {
    public:
        Tax();
        Tax(const std::string& name, const std::string& abbreviation, const std::string& desc, int tax, const std::string& UI_color);

        unsigned int get_tax() const;

        Tile_State Action() override;
        Tile_State Action(Player* player) override;
    private:
        unsigned int tax;
};


#endif