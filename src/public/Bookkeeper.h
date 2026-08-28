#ifndef BOOKKEEPER_H
#define BOOKKEEPER_H

#include <unordered_map>
#include <vector>
#include <string>

class Bookkeeper {
    public:
        Bookkeeper();
        void add_property(const std::string& color);
        void remove_property(const std::string& color);
        // return all full sets
        std::vector<std::string> full_sets() const;
        bool is_full_set(const std::string& color) const;
        bool is_tracked(const std::string& color) const;
        int get_count(const std::string& color) const;
        
    private:
        std::unordered_map<std::string, int> max_properties;
        std::unordered_map<std::string, int> properties;
        std::vector<std::string> color_sets;
        std::vector<std::string> special_sets;
};

#endif
