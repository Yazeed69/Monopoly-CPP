#ifndef STREET_H
#define STREET_H

#include "Property.h"

class Street : public Property {
    public:
        Street();
        Street(const std::string& name, const std::string& abbreviation, const std::string& color_set, const std::string& desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, unsigned int build_cost, unsigned int base_rent, unsigned int full_set_rent, unsigned int rent_1_house, unsigned int rent_2_houses, unsigned int rent_3_houses, unsigned int rent_4_houses, unsigned int rent_hotel, const std::string& UI_color);
    
        unsigned int get_num_houses() const;
        unsigned int get_num_hotels() const;
        unsigned int get_build_cost() const;
        unsigned int get_sell_price() const;
        unsigned int get_rent() const;
        unsigned int get_base_rent() const;
        unsigned int get_full_set_rent();
        const std::string& get_houses() const;
        void set_rent(unsigned int rent);
        void reset_buildings();

        bool get_full_set() const;
        void set_full_set(const bool full_set);
        // rent always follows the buildings, the colour set and the mortgage flag
        void refresh_rent();

        void build();
        void sell();

        Tile_State Action() override;
        Tile_State Action(Player* player) override;


    private:

        void refresh_houses();

        std::string hosues;
        unsigned int rent;
        unsigned int num_houses;
        unsigned int build_cost;
        unsigned int base_rent;
        unsigned int full_set_rent;
        unsigned int rent_1_house;
        unsigned int rent_2_houses;
        unsigned int rent_3_houses;
        unsigned int rent_4_houses;
        unsigned int rent_hotel;
        bool full_set;

};

#endif 
