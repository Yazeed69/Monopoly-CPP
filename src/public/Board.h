#ifndef BOARD_H
#define BOARD_H

#include <fstream>
#include <string>
#include <unordered_map>


#include "Tile.h"
#include "Street.h"
#include "Railroad.h"
#include "Draw.h"
#include "Utility.h"
#include "Tax.h"
#include "GoToJail.h"
#include "Hybrid.h"
#include "FreeParking.h"
#include "GoToJail.h"
#include "Go.h"

class Board {
    public:
        Board();
        ~Board();

        static constexpr unsigned int board_size = 40;

        Tile* get_tile(unsigned int i) const;
        Tile** get_tiles() const;
        // false when the board file could not be found or was short
        bool is_loaded() const;
        const std::string& get_load_error() const;

        std::vector<Street*> get_color_set_assets(const std::string& color) const;
        // every deed in a set, streets plus the railroad and utility groups
        std::vector<Property*> get_set_assets(const std::string& color) const;

        void read_board();

        void place_pawn(unsigned int i, char pawn);
        void step_pawn(unsigned int i, char pawn);
        void move_pawn(unsigned int i, char pawn);
        void remove_pawn(unsigned int i, char pawn);

        std::vector<Street*> get_color_sets(const std::string& color_set) const;


    private:
        std::string clean_name(const std::string& input) const;
        std::ifstream open_board_file();
        Tile** tiles;
        unsigned int loaded_tiles;
        std::string load_error;
        std::unordered_map<std::string, std::vector<Street*>> color_sets;
        std::unordered_map<std::string, std::vector<Property*>> set_assets;

};

#endif
