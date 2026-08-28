#ifndef GAME_H
#define GAME_H

#include <random>
#include <chrono>
#include <string>
#include <vector>

#include "GameContext.h"
#include "State.h"


class Game {
    public:
        Game(const std::vector<std::string>& names, bool free_parking_pot);
        ~Game();

        GameContext* get_context() const;
        bool is_ready() const;

        End_Turn_State handle_rent_payment(unsigned int rent);
        // every outstanding debt comes back through here until it is paid or the player is out
        End_Turn_State resolve_debt(bool allow_raise);

        void process_turn(State state, Game_State& game_state, unsigned int option, bool& valid_option);
        void complete_jail_exit(Game_State& game_state, bool move_now);
        Manage_Assets_State process_manage_assets(Manage_Assets_Menu menu, Property* target);
        const std::string game_process_card(Tile_State& tile_state);
        End_Turn_State process_turn(Tile_State state, unsigned int option);
        void end_turn();

        bool is_over() const;
        Player* get_winner() const;

    private:
        void award_go_bonus(Game_State& game_state);

        GameContext* context;
        End_Turn_State debt_success_state;
};

#endif
