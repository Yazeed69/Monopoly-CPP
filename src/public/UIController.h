#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "GameContext.h"
#include "State.h"


class UIController {
    public:
        UIController(GameContext* context);
        void attach(GameContext* context);

        void clear_console();
        void print_board();

        const std::string& get_path(unsigned int i) const;
        std::string get_houses(unsigned int i) const;
        const std::string& get_abbreviation(unsigned int i) const;
        std::string get_player_name(unsigned int i) const;
        std::string get_player_assets(unsigned int i) const;
        std::string get_dice_roll(unsigned int i) const;
        std::string get_status_line() const;

        // setup runs before there is a context to draw
        void title_screen();
        unsigned int prompt_player_count();
        std::vector<std::string> prompt_player_names(unsigned int count);
        bool prompt_free_parking_rule();

        void process_turn(State& state);
        void process_turn(Tile_State& state, Game_State& game_state);
        void process_turn(End_Turn_State& state);

        void process_card(std::string& card_text);

        void process_manage_assets(Manage_Assets_Menu menu, Player* player, bool raising_funds, const std::vector<std::string>& options, const std::vector<std::string>& prices);
        void process_manage_assets(Manage_Assets_State state, const std::string& property_name);

        void announce_game_over(const std::vector<Player*>& standings);


        void whose_turn();
        void prompt(std::string& message, std::vector<std::string>& options);
        void prompt(std::string message, std::vector<std::string> options);
        void prompt(std::string message, std::vector<std::string> options, std::vector<std::string> prices);
        void reaction(std::string& message);
        void reaction(std::string message);

        // every read goes through here so a stray word or a closed pipe cannot spin the loop
        unsigned int read_option(unsigned int max);
        unsigned int read_amount(unsigned int max);
        std::string read_text(const std::string& message);
        bool read_yes(const std::string& message);
        void wait();
        bool input_closed() const;


    private:
        GameContext* context;
        bool closed_input;
        std::ostringstream board;
        std::ostringstream player_stream;
        std::ostringstream prompt_stream;
        std::ostringstream reaction_stream;
};

#endif
