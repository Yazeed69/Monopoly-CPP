#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

#include "Board.h"
#include "Player.h"
#include "Dice.h"

class Property;

// one entry in a deck, the text is produced by the action itself so the card
// stays next to the behaviour it describes
struct Card {
    std::function<std::string()> action;
    bool jail_card;
    bool held;
};

// everything one player is handing to another in a single swap
struct Trade {
    Player* proposer;
    Player* partner;
    std::vector<Property*> offered;
    std::vector<Property*> requested;
    unsigned int cash_offered;
    unsigned int cash_requested;
    unsigned int jail_cards_offered;
    unsigned int jail_cards_requested;
};

class GameContext {
    public:
        GameContext(const std::vector<std::string>& names, bool free_parking_pot);
        ~GameContext();

        static constexpr unsigned int go_bonus = 200;
        static constexpr unsigned int jail_fine = 50;
        static constexpr unsigned int starting_balance = 1500;
        static constexpr unsigned int jail_index = 10;
        static constexpr unsigned int max_rounds_in_jail = 3;
        static constexpr unsigned int house_stock = 32;
        static constexpr unsigned int hotel_stock = 12;

        std::vector<Player*> get_players() const;
        Player* get_current_player() const;
        void set_current_tile(Tile* tile);
        Tile* get_current_tile() const;
        Tile** get_tiles() const;
        std::unordered_map<Player*, unsigned int> get_players_locations() const;
        unsigned int get_player_location(Player* player) const;
        Board* get_board() const;
        Dice* get_die(unsigned int i) const;
        FreeParking* get_free_parking() const;

        Tile* move_player(unsigned int roll, Game_State& reward);
        void move_player_to_tile(Tile* tile, unsigned int tile_index, bool passGo, unsigned int goBonus);

        unsigned int get_turn();
        void next_player();
        void start_turn();

        void roll_dice(Player* player);

        bool buy_property();
        bool award_auction(Player* winner, Property* property, unsigned int price);
        bool transfer_property(Player* from, Player* to, Property* property, unsigned int price);
        void declare_bankrupt(Player* debtor, Player* creditor);
        void update_rents_for_color_set(const std::string& color_set);
        std::vector<Player*> get_active_players() const;
        std::vector<Player*> get_opponents(Player* player) const;

        // every debt in the game funnels through this pair so bankruptcy is
        // decided in exactly one place
        void charge(Player* debtor, Player* creditor, unsigned int amount, bool to_all = false);
        bool has_pending_debt() const;
        unsigned int get_pending_debt() const;
        Player* get_pending_debtor() const;
        Player* get_pending_creditor() const;
        Payment_State settle_pending_debt(bool allow_raise);
        void clear_pending_debt();

        // cash the player could actually put their hands on, given what the bank still has in its box
        unsigned int raisable_cash(Player* player) const;
        // what the bank charges somebody taking on a mortgaged deed
        unsigned int transfer_interest(Property* property) const;

        // the bank sells a bankrupt estate by auction, one deed at a time
        Property* next_bank_auction();
        bool has_bank_auctions() const;
        void abandon_deed(Property* property);

        // a "collect from each player" card lines the opponents up so each debt is
        // settled through the same funnel as any other
        void charge_opponents(Player* collector, unsigned int amount);
        bool has_pending_collections() const;
        bool next_collection();

        // rent the card system has already fixed for this landing, 0 when the
        // normal deed rent applies
        unsigned int get_pending_rent() const;
        void clear_pending_rent();
        unsigned int rent_due(Tile* tile, Player* player) const;

        Manage_Assets_State build_house(Street* street);
        Manage_Assets_State sell_house(Street* street);
        Manage_Assets_State mortgage_property(Property* property);
        Manage_Assets_State unmortgage_property(Property* property);
        unsigned int get_houses_available() const;
        unsigned int get_hotels_available() const;

        Trade_State execute_trade(const Trade& trade);
        std::string describe_trade(const Trade& trade) const;

        void take_to_jail();
        void release_from_jail();
        bool use_jail_card(Player* player);

        std::string draw_chance();
        std::string draw_community_chest();

        const bool get_advanced() const;


    private:


        std::string collect(unsigned int amount, const std::string& message);
        std::string collect_from_each(unsigned int amount, const std::string& message);
        std::string pay(unsigned int amount, const std::string& message, bool all_players);
        std::string repairs(unsigned int per_house, unsigned int per_hotel, const std::string& message);
        std::string advanceToTile(Tile* tile, unsigned int tile_index, const std::string& message, bool passGo, unsigned int goBonus);
        std::string advanceToNearestRailroad(bool passGo, unsigned int goBonus, const std::string& message);
        std::string advanceToNearestUtility(bool passGo, unsigned int goBonus, const std::string& message);
        std::string advanceToGo(unsigned int goBonus, const std::string& message);
        std::string goToJail(const std::string& message);
        std::string moveBack(unsigned int spaces, const std::string& message);
        std::string getOutOfJailFree(Deck_Type deck, const std::string& message);

        std::string draw_card(std::vector<Card>& deck, unsigned int& index);
        void return_jail_card(Deck_Type deck);
        void pay_bank(unsigned int amount);
        void distribute_payment(unsigned int amount);
        unsigned int mortgage_interest(const std::vector<Property*>& properties) const;
        std::vector<Street*> color_set_of(Property* property) const;

        void create_chance_deck();
        void create_community_chest_deck();

        unsigned int turn;
        Board* board;
        std::vector<Player*> players;
        std::unordered_map<Player*, unsigned int> players_locations;

        Player* current_player;
        Tile* current_tile;
        FreeParking* free_parking;
        std::array<Dice*, 2> dies;
        std::vector<Card> community_chest_deck;
        std::vector<Card> chance_deck;

        unsigned int chance_index;
        unsigned int community_chest_index;

        unsigned int houses_available;
        unsigned int hotels_available;

        unsigned int pending_debt;
        Player* pending_debtor;
        Player* pending_creditor;
        bool pending_debt_to_all;
        unsigned int pending_rent;

        std::vector<Player*> pending_collections;
        Player* pending_collector;
        unsigned int pending_collection_amount;
        std::vector<Property*> pending_bank_auctions;

        bool advanced;

};

#endif
