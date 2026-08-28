#include "Board.h"
#include "Player.h"



#include <iostream>
#include <limits>

Board::Board(){
    tiles = nullptr;
    loaded_tiles = 0;
    load_error = "";
    read_board();
}

Board::~Board(){
    if (tiles) {
        for (unsigned int i = 0; i < loaded_tiles; i++) {
            delete tiles[i];
        }
        delete[] tiles;
    }
}

Tile* Board::get_tile(unsigned int i) const {
    return tiles[i % board_size];
}

Tile** Board::get_tiles() const {
    return tiles;
}

bool Board::is_loaded() const {
    return tiles != nullptr && loaded_tiles == board_size;
}

const std::string& Board::get_load_error() const {
    return load_error;
}

std::vector<Street *> Board::get_color_set_assets(const std::string &color) const {
    auto it = color_sets.find(color);
    if (it == color_sets.end()) {
        return std::vector<Street*>();
    }
    return it->second;
}

std::vector<Property *> Board::get_set_assets(const std::string &color) const {
    auto it = set_assets.find(color);
    if (it == set_assets.end()) {
        return std::vector<Property*>();
    }
    return it->second;
}

void Board::place_pawn(unsigned int i, char pawn) {
    tiles[i % board_size]->enter_path(pawn);
}

void Board::step_pawn(unsigned int i, char pawn) {
    tiles[i % board_size]->exit_path(pawn);
    tiles[(i + 1) % board_size]->enter_path(pawn);
}

void Board::move_pawn(unsigned int i, char pawn) {
    for (unsigned int tile = 0; tile < loaded_tiles; tile++) {
        tiles[tile]->exit_path(pawn);
    }
    tiles[i % board_size]->enter_path(pawn);
}

void Board::remove_pawn(unsigned int i, char pawn) {
    tiles[i % board_size]->exit_path(pawn);
}

std::vector<Street *> Board::get_color_sets(const std::string &color_set) const {
    return get_color_set_assets(color_set);
}

std::ifstream Board::open_board_file() {
    // the game can be launched from the repo root or from src, so try both
    const std::string paths[] = {
        "src/assets/default-board.txt",
        "assets/default-board.txt",
        "../src/assets/default-board.txt"
    };
    for (const std::string& path : paths) {
        std::ifstream file(path);
        if (file) {
            return file;
        }
    }
    return std::ifstream();
}

void Board::read_board() {
    std::ifstream file = open_board_file();
    // a default constructed stream is not in a failed state, so ask whether it opened
    if (!file.is_open()) {
        load_error = "could not open src/assets/default-board.txt";
        return;
    }
    tiles = new Tile*[board_size];
    for (unsigned int slot = 0; slot < board_size; slot++) {
        tiles[slot] = nullptr;
    }
    char type;
    std::string name, abbreviation, color_set, UI_color;
    unsigned int purchase_price, mortgage_value, unmortgage_value;
    unsigned int i = 0;
    bool built = false;
    while (i < board_size && file >> type >> name >> abbreviation) {
        name = clean_name(name);
        built = true;
        switch (type){
            case 's': {
                unsigned int build_cost, base_rent, full_set_rent, rent_1_house, rent_2_houses, rent_3_houses, rent_4_houses, rent_hotel;
                file >> color_set >> purchase_price >> mortgage_value >> unmortgage_value >> build_cost
                     >> base_rent >> full_set_rent >> rent_1_house >> rent_2_houses >> rent_3_houses
                     >> rent_4_houses >> rent_hotel >> UI_color;
                color_set = clean_name(color_set);
                tiles[i++] = new Street(name, abbreviation, color_set, "...", purchase_price, mortgage_value,
                                           unmortgage_value, build_cost, base_rent, full_set_rent, rent_1_house,
                                           rent_2_houses, rent_3_houses, rent_4_houses, rent_hotel, UI_color);
                color_sets[color_set].push_back(dynamic_cast<Street*>(tiles[i - 1]));
                set_assets[color_set].push_back(dynamic_cast<Street*>(tiles[i - 1]));

                break;
            }
            case 'r':
                file >> color_set >> purchase_price >> mortgage_value >> unmortgage_value >> UI_color;
                color_set = clean_name(color_set);
                tiles[i++] = new Railroad(name, abbreviation, color_set, "...", purchase_price, mortgage_value, unmortgage_value, UI_color);
                set_assets[color_set].push_back(dynamic_cast<Railroad*>(tiles[i - 1]));
                break;
            case 'd': {
                bool draw_type;
                file >> draw_type >> UI_color;
                tiles[i++] = new Draw(name, abbreviation, "...", draw_type, UI_color);
                break;
            }
            case 'u':
                file >> color_set >> purchase_price >> mortgage_value >> unmortgage_value >> UI_color;
                color_set = clean_name(color_set);
                tiles[i++] = new Utility(name, abbreviation, color_set, "...", purchase_price, mortgage_value, unmortgage_value, UI_color);
                set_assets[color_set].push_back(dynamic_cast<Utility*>(tiles[i - 1]));
                break;
            case 't': {
                unsigned int tax;
                file >> tax >> UI_color;
                tiles[i++] = new Tax(name, abbreviation, "...", tax, UI_color);
                break;
            }
            case 'h':
                file >> UI_color;
                tiles[i++] = new Hybrid(name, abbreviation, "...", UI_color);
                break;
            case 'j':
                file >> UI_color;
                tiles[i++] = new GoToJail(name, abbreviation, "...", UI_color);
                break;
            case 'f':
                file >> UI_color;
                tiles[i++] = new FreeParking(name, abbreviation, "...", UI_color);
                break;
            case 'g':
                file >> UI_color;
                tiles[i++] = new Go(name, abbreviation, "...", UI_color);
                break;
            default:
                // unknown tile letter, drop the rest of the line so the next read stays in step
                built = false;
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
        }
        if (!file) {
            // the line ran out halfway through, so the tile just built cannot be trusted
            if (built && i > 0) {
                delete tiles[--i];
                tiles[i] = nullptr;
            }
            break;
        }
    }
    loaded_tiles = i;
    if (loaded_tiles != board_size) {
        load_error = "board file only described " + std::to_string(loaded_tiles) + " of " + std::to_string(board_size) + " tiles";
    }
}

std::string Board::clean_name(const std::string& input) const {
    std::string result = input;

    // Remove leading and trailing quotes
    if (!result.empty() && result.front() == '"') {
        result.erase(0, 1); // Remove leading quote
    }
    if (!result.empty() && result.back() == '"') {
        result.pop_back(); // Remove trailing quote
    }

    // Replace underscores with spaces
    std::replace(result.begin(), result.end(), '_', ' ');

    return result;
}
