#ifndef RAILROAD_H
#define RAILROAD_H

#include "Property.h"

class Railroad : public Property {
    public:
        Railroad();
        Railroad(const std::string& name, const std::string abbreviation, const std::string color_set, const std::string desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, const std::string& UI_color);

        unsigned int get_rent(unsigned int num_railroads) const;
        void set_rent(unsigned int rent, unsigned int num_railroads);
        Tile_State Action() override;
        Tile_State Action(Player* player) override;
    private:
        unsigned int rent[4];
}; 

#endif