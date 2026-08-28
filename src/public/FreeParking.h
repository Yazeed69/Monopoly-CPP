#ifndef FREEPARKING_H
#define FREEPARKING_H

#include "Tile.h"

class FreeParking : public Tile {
    public:
        FreeParking(bool using_pot = false);
        FreeParking(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color, bool using_pot = false);
    
        int get_pot();
        int peek_pot() const;
        void add_to_pot(int amount);

        bool get_using_pot() const;
        void set_using_pot(bool using_pot);

        Tile_State Action() override;
        Tile_State Action(Player* player) override;

    private:
        bool using_pot;
        int pot;
};

#endif 
