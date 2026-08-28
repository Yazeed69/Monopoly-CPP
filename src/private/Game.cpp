#include "Game.h"

#include <algorithm>

Game::Game(const std::vector<std::string>& names, bool free_parking_pot) {
    this->context = new GameContext(names, free_parking_pot);
    this->debt_success_state = End_Turn_State::CONTINUE;
}

Game::~Game() {
    delete context;
}

GameContext* Game::get_context() const {
    return context;
}

bool Game::is_ready() const {
    return context->get_board()->is_loaded() && !context->get_players().empty();
}

void Game::award_go_bonus(Game_State& game_state) {
    if (game_state == Game_State::PASSED_GO_REWARD || game_state == Game_State::LANDED_ON_GO_REWARD) {
        context->get_current_player()->add_balance(GameContext::go_bonus);
    }
}

void Game::process_turn(State state, Game_State& game_state, unsigned int option, bool& valid_option) {
    Player* player = context->get_current_player();
    switch (state) {
        case State::NORMAL:
            switch (option) {
                case 1:
                    context->clear_pending_rent();
                    context->roll_dice(player);
                    if (player->get_rolled_double()) {
                        player->increment_consecutive_doubles();
                        if (player->get_consecutive_doubles() >= 3) {
                            // three doubles in a row is a trip to jail, not a third move
                            context->take_to_jail();
                            game_state = Game_State::THIRD_DOUBLE_TO_JAIL;
                            valid_option = true;
                            break;
                        }
                    } else {
                        player->reset_consecutive_doubles();
                    }
                    context->set_current_tile(context->move_player(player->get_roll(), game_state));
                    award_go_bonus(game_state);
                    valid_option = true;
                    break;
                case 2:
                    game_state = Game_State::MANAGE_ASSETS;
                    break;
                case 3:
                    game_state = Game_State::TRADE;
                    break;
            }
            break;
        case State::JAIL:
        case State::JAIL_LAST_ROUND:
            switch (option) {
                case 1:
                    context->clear_pending_rent();
                    context->roll_dice(player);
                    if (player->get_rolled_double()){
                        context->release_from_jail();
                        context->set_current_tile(context->move_player(player->get_roll(), game_state));
                        award_go_bonus(game_state);
                        // leaving on a double does not also earn another roll
                        player->set_rolled_double(false);
                        player->reset_consecutive_doubles();
                    } else if (state == State::JAIL_LAST_ROUND) {
                        // third failed attempt, the fine stops being optional
                        context->charge(player, nullptr, GameContext::jail_fine);
                        debt_success_state = End_Turn_State::CONTINUE;
                        game_state = Game_State::FORCED_JAIL_FINE;
                    } else {
                        player->increment_rounds_in_jail();
                        game_state = Game_State::STAYED_IN_JAIL;
                    }
                    valid_option = true;
                    break;
                case 2:
                    // paying is optional here, so do not let it be the thing that ends their game
                    if (context->raisable_cash(player) >= GameContext::jail_fine) {
                        context->charge(player, nullptr, GameContext::jail_fine);
                        debt_success_state = End_Turn_State::CONTINUE;
                        game_state = Game_State::PAID_JAIL_FINE;
                        valid_option = true;
                    }
                    break;
                case 3:
                    if (context->use_jail_card(player)) {
                        debt_success_state = End_Turn_State::CONTINUE;
                        game_state = Game_State::PAID_JAIL_FINE;
                        valid_option = true;
                    }
                    break;
                case 4:
                    game_state = Game_State::MANAGE_ASSETS;
                    break;
                case 5:
                    game_state = Game_State::TRADE;
                    break;
            }
            break;
    }
}

void Game::complete_jail_exit(Game_State& game_state, bool move_now) {
    Player* player = context->get_current_player();
    context->release_from_jail();
    if (!move_now) {
        return;
    }
    // the compulsory fine still leaves the player the roll they already made
    context->set_current_tile(context->move_player(player->get_roll(), game_state));
    award_go_bonus(game_state);
    player->set_rolled_double(false);
    player->reset_consecutive_doubles();
}

Manage_Assets_State Game::process_manage_assets(Manage_Assets_Menu menu, Property* target) {
    if (!target) {
        return Manage_Assets_State::NULL_STATE;
    }
    switch (menu) {
        case Manage_Assets_Menu::BUILD:
            return context->build_house(dynamic_cast<Street*>(target));
        case Manage_Assets_Menu::SELL:
            return context->sell_house(dynamic_cast<Street*>(target));
        case Manage_Assets_Menu::MORTGAGE:
            return context->mortgage_property(target);
        case Manage_Assets_Menu::UNMORTGAGE:
            return context->unmortgage_property(target);
        default:
            return Manage_Assets_State::NULL_STATE;
    }
}

const std::string Game::game_process_card(Tile_State& tile_state) {
    std::string card_text;
    if (tile_state == Tile_State::DRAW_CC) {
        card_text = context->draw_community_chest();
    } else if (tile_state == Tile_State::DRAW_CHANCE) {
        card_text = context->draw_chance();
    }
    if (context->has_pending_debt() || context->has_pending_collections()) {
        debt_success_state = End_Turn_State::CONTINUE;
    }
    return card_text;
}

End_Turn_State Game::resolve_debt(bool allow_raise) {
    switch (context->settle_pending_debt(allow_raise)) {
        case Payment_State::PAID:
            return debt_success_state;
        case Payment_State::SHORT:
            return End_Turn_State::SHORT_ON_CASH;
        default:
            return End_Turn_State::BANKRUPT;
    }
}

End_Turn_State Game::handle_rent_payment(unsigned int rent) {
    Property* property = dynamic_cast<Property*>(context->get_current_tile());
    Player* owner = (property) ? property->get_owner() : nullptr;
    Player* payer = context->get_current_player();

    if (!owner || owner == payer || rent == 0) {
        return End_Turn_State::CONTINUE;
    }
    payer->set_last_paid_rent(rent);
    context->charge(payer, owner, rent);
    debt_success_state = End_Turn_State::PAYED_RENT;
    return resolve_debt(true);
}


End_Turn_State Game::process_turn(Tile_State state, unsigned int option) {
    Tile* tile = context->get_current_tile();  // Retrieve current tile once
    Player* current_player = context->get_current_player();

    switch (state) {
        case Tile_State::UNOWNED: {
            Property* property = dynamic_cast<Property*>(tile);
            if (!property) {
                return End_Turn_State::CONTINUE;
            }
            if (option == 1) {
                if (current_player->get_balance() < property->get_purchase_price()) {
                    return End_Turn_State::NO_MONEY_TO_BUY;
                }
                return (context->buy_property()) ? End_Turn_State::BOUGHT_PROPERTY : End_Turn_State::NO_MONEY_TO_BUY;
            }
            return End_Turn_State::PASSED_ON_PROPERTY;
        }

        case Tile_State::OWN_PROPERTY:
            return End_Turn_State::CONTINUE;

        case Tile_State::OWNED_STREET:
        case Tile_State::OWNED_RAILROAD:
        case Tile_State::OWNED_UTILITY:
            return handle_rent_payment(context->rent_due(tile, current_player));

        case Tile_State::MORTGAGED:
        case Tile_State::DRAW_CC:
        case Tile_State::DRAW_CHANCE:
        case Tile_State::HYBRID:
        case Tile_State::GO:
        case Tile_State::FREE:
            return End_Turn_State::CONTINUE;

        case Tile_State::TAX: {
            Tax* tax_tile = dynamic_cast<Tax*>(tile);
            if (!tax_tile) {
                return End_Turn_State::CONTINUE;
            }
            unsigned int tax = tax_tile->get_tax();
            current_player->set_last_tax_paid(tax);
            context->charge(current_player, nullptr, tax);
            debt_success_state = End_Turn_State::PAYED_TAX;
            return resolve_debt(true);
        }

        case Tile_State::FREE_POT: {
            FreeParking* parking_tile = dynamic_cast<FreeParking*>(tile);
            if (!parking_tile) {
                return End_Turn_State::CONTINUE;
            }
            int pot = parking_tile->get_pot();
            if (pot > 0) {
                current_player->add_balance(pot);
                return End_Turn_State::FREE_POT;
            } else {
                return End_Turn_State::CONTINUE;
            }
        }

        case Tile_State::TO_JAIL:
            context->take_to_jail();
            return End_Turn_State::TO_JAIL;

        default:
            return End_Turn_State::CONTINUE;
    }
}

void Game::end_turn(){
    context->next_player();
}

bool Game::is_over() const {
    return context->get_active_players().size() <= 1;
}

Player* Game::get_winner() const {
    std::vector<Player*> active_players = context->get_active_players();
    if (active_players.empty()) {
        return nullptr;
    }
    if (active_players.size() == 1) {
        return active_players.front();
    }
    // nobody has been knocked out yet, the richest player is ahead
    Player* leader = active_players.front();
    for (Player* player : active_players) {
        if (player->net_worth() > leader->net_worth()) {
            leader = player;
        }
    }
    return leader;
}
