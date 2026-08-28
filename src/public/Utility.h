#ifndef UTILITY_H
#define UTILITY_H

#include "Property.h"

class Utility : public Property {
    public:
        Utility();
        Utility(const std::string& name, const std::string& abbreviation, const std::string& color_set, const std::string& desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, const std::string& UI_color);
    
        unsigned int get_rent(unsigned int num_utilities, unsigned int roll) const;
        unsigned int get_multiplier(unsigned int num_utilities) const;
        Tile_State Action() override;
        Tile_State Action(Player* player) override;
        
    private:
        unsigned int rent_multiplier[2];
};

#endif