#include "Bookkeeper.h"
#include <iostream>

Bookkeeper::Bookkeeper() {
    max_properties = {
        {"Brown", 2},
        {"Light Blue", 3},
        {"Pink", 3},
        {"Orange", 3},
        {"Red", 3},
        {"Yellow", 3},
        {"Green", 3},
        {"Dark Blue", 2},
        {"Railroad", 4},
        {"Utility", 2}
    };
    properties = {
        {"Brown", 0},
        {"Light Blue", 0},
        {"Pink", 0},
        {"Orange", 0},
        {"Red", 0},
        {"Yellow", 0},
        {"Green", 0},
        {"Dark Blue", 0},
        {"Railroad", 0},
        {"Utility", 0}
    };
    color_sets = {
        "Brown",
        "Light Blue",
        "Pink",
        "Orange",
        "Red",
        "Yellow",
        "Green",
        "Dark Blue"
    };
    special_sets = {
        "Railroad",
        "Utility"
    };
}

void Bookkeeper::add_property(const std::string& color) {
    if (!is_tracked(color)) {
        return;
    }
    properties[color]++;
}

void Bookkeeper::remove_property(const std::string& color) {
    if (!is_tracked(color) || properties[color] == 0) {
        return;
    }
    properties[color]--;
}

std::vector<std::string> Bookkeeper::full_sets() const {
    std::vector<std::string> full_sets;
    for (const std::string& color : color_sets) {
        if (properties.at(color) == max_properties.at(color)) {
            full_sets.push_back(color);
        }
    }
    return full_sets;
}

bool Bookkeeper::is_full_set(const std::string &color) const {
    if (!is_tracked(color)) {
        return false;
    }
    return properties.at(color) == max_properties.at(color);
}

bool Bookkeeper::is_tracked(const std::string &color) const {
    return max_properties.find(color) != max_properties.end();
}

int Bookkeeper::get_count(const std::string &color) const {
    if (!is_tracked(color)) {
        return 0;
    }
    return properties.at(color);
}
