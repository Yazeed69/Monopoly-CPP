#include "Property.h"
#include "Player.h"

Property::Property() : Tile() {
    owner = nullptr;
    color_set = "";
    purchase_price = 0;
    mortgage_value = 0;
    unmortgage_value = 0;  
    mortgaged = false;
}

Property::Property(const std::string& name, const std::string& abbreviation, const std::string& color_set, const std::string& desc, const unsigned int purchase_price, const unsigned int mortgage_value, const unsigned int unmortgage_value, const std::string& UI_color) : Tile(name, abbreviation, desc, UI_color) {
    owner = nullptr;
    this->color_set = color_set;
    this->purchase_price = purchase_price;
    this->mortgage_value = mortgage_value;
    this->unmortgage_value = unmortgage_value;
    mortgaged = false;
}

Player* Property::get_owner() const {
    return owner;
}

std::string Property::get_color_set() const {
    return color_set;
}

unsigned int Property::get_purchase_price() const {
    return purchase_price;
}

unsigned int Property::get_mortgage_value() const {
    return mortgage_value;
}

unsigned int Property::get_unmortgage_value() const {
    return unmortgage_value;
}

void Property::set_owner(Player* owner) {
    this->owner = owner;
}

void Property::set_color_set(const std::string& color_set) {
    this->color_set = color_set;
}

void Property::set_purchase_price(const unsigned int purchase_price) {
    this->purchase_price = purchase_price;
}

bool Property::get_mortgaged() const {
    return mortgaged;
}

void Property::set_mortgaged(const bool mortgaged) {
    this->mortgaged = mortgaged;
}
