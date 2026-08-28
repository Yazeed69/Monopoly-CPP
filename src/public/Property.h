#ifndef PROPERTY_H
#define PROPERTY_H

#include "Tile.h"

class Player;


class Property : public Tile {
    public:
        Property();
        Property(const std::string& name, const std::string& abbreviation, const std::string& color_set, const std::string& desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, const std::string& UI_color);
        Player* get_owner() const;
        std::string get_color_set() const;
        unsigned int get_purchase_price() const;
        unsigned int get_mortgage_value() const;
        unsigned int get_unmortgage_value() const;

        void set_owner(Player* owner);
        void set_color_set(const std::string& color_set);
        void set_purchase_price(const unsigned int purchase_price);

        bool get_mortgaged() const;
        void set_mortgaged(const bool mortgaged);

        
    private:
        Player* owner;
        std::string color_set;
        unsigned int purchase_price;
        unsigned int mortgage_value;
        unsigned int unmortgage_value;
        bool mortgaged;
};

#endif


/*
Color Sets

Brown 2
Light Blue 3
Pink 3
Orange 3
Red 3
Yellow 3
Green 3
Dark Blue 2
Railroad 4
Utility 2

*/
