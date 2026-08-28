#ifndef PLAYER_H
#define PLAYER_H



#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <memory>

#include "Bookkeeper.h"
#include "State.h"

class Tile;
class Property;
class Railroad;
class Utility;
class Street;


class Player {
    public:
        Player();
        Player(const std::string& name, const char pawn, const unsigned int balance);
        Player(const Player& other);
        Player& operator=(const Player& other);
        Player(Player&& other) noexcept;
        Player& operator=(Player&& other) noexcept;
        ~Player();

        std::string get_name() const;
        char get_pawn() const;
        unsigned int get_balance() const;

        void set_name(const std::string& name);
        void set_pawn(const char pawn);

        void set_balance(const unsigned int balance);
        void add_balance(const unsigned int balance);
        bool withdraw_balance(const unsigned int balance);

        Bookkeeper* get_bookkeeper() const;

        std::vector<Tile*> get_assets() const;
        bool get_in_jail() const;
        unsigned int get_roll() const;

        void set_in_jail(const bool in_jail);
        void set_roll(const unsigned int roll);

        void add_asset(Tile* asset);
        void add_asset(Tile* asset, bool& full_set);
        void remove_asset(const Tile* asset);
        void clear_assets();
        std::string assets_string() const;

        bool get_rolled_double() const;
        void set_rolled_double(const bool rolled_double);

        unsigned int get_consecutive_doubles() const;
        void increment_consecutive_doubles();
        void reset_consecutive_doubles();

        unsigned int get_rounds_in_jail() const;
        void increment_rounds_in_jail();
        void reset_rounds_in_jail();

        unsigned int get_last_paid_rent() const;
        unsigned int get_last_tax_paid() const;
        unsigned int get_last_roll() const;
        void set_last_paid_rent(const unsigned int last_paid_rent);
        void set_last_tax_paid(const unsigned int last_tax_paid);
        void set_last_roll(const unsigned int last_roll);
        
        void set_draw_card(const std::string &card_text);
        
        std::string get_draw_card() const;

        unsigned int get_num_railroads() const;
        unsigned int get_num_utilities() const;

        // buildings across every street the player holds, used by the repair cards
        unsigned int get_num_houses() const;
        unsigned int get_num_hotels() const;

        unsigned int net_worth() const;

        void add_jail_card(const Deck_Type deck);
        bool consume_jail_card(const Deck_Type deck);
        bool has_jail_card(const Deck_Type deck) const;
        bool has_any_jail_card() const;
        unsigned int get_jail_cards() const;

        bool is_active() const;
        void set_active(bool is_active);


        static unsigned int get_active_players();
        static bool comparePlayersByRoll(const Player* a, const Player* b);

    private:
        static unsigned int active_players;
        std::string name;
        char pawn;
        unsigned int balance;

        std::unique_ptr<Bookkeeper> bookkeeper;

        std::vector<Tile*> assets;
        std::unordered_map<std::string, unsigned int> color_set_tracker;
        bool in_jail;
        bool rolled_double;
        unsigned int consecutive_doubles;
        unsigned int roll;
        unsigned int last_paid_rent;
        unsigned int last_tax_paid;  
        unsigned int last_roll;

        unsigned int num_railroads;
        unsigned int num_utilities;

        std::string card_text;

        unsigned int rounds_in_jail;
        unsigned int chance_jail_cards;
        unsigned int community_jail_cards;
        bool active;


};

#endif
