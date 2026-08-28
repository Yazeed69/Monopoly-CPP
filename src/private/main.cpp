#include "Game.h"
#include "UIController.h"
#include "State.h"
#include "Property.h"
#include "Player.h"

#include <algorithm>
#include <vector>


//GAME STATE
//TILE STATE

// the deeds a player could act on in each branch of the asset menu
static std::vector<Property*> assets_for_menu(Player* player, Manage_Assets_Menu menu) {
    std::vector<Property*> targets;
    for (Tile* asset : player->get_assets()) {
        Property* property = dynamic_cast<Property*>(asset);
        if (!property) {
            continue;
        }
        Street* street = dynamic_cast<Street*>(property);
        switch (menu) {
            case Manage_Assets_Menu::BUILD:
                if (street && player->get_bookkeeper()->is_full_set(street->get_color_set()) && street->get_num_houses() < 5) {
                    targets.push_back(property);
                }
                break;
            case Manage_Assets_Menu::SELL:
                if (street && street->get_num_houses() > 0) {
                    targets.push_back(property);
                }
                break;
            case Manage_Assets_Menu::MORTGAGE:
                if (!property->get_mortgaged()) {
                    targets.push_back(property);
                }
                break;
            case Manage_Assets_Menu::UNMORTGAGE:
                if (property->get_mortgaged()) {
                    targets.push_back(property);
                }
                break;
            default:
                break;
        }
    }
    return targets;
}

static std::string price_for_menu(Property* property, Manage_Assets_Menu menu) {
    Street* street = dynamic_cast<Street*>(property);
    switch (menu) {
        case Manage_Assets_Menu::BUILD:
            return (street) ? std::to_string(street->get_build_cost()) : "0";
        case Manage_Assets_Menu::SELL:
            return (street) ? std::to_string(street->get_sell_price()) : "0";
        case Manage_Assets_Menu::MORTGAGE:
            return std::to_string(property->get_mortgage_value());
        default:
            return std::to_string(property->get_unmortgage_value());
    }
}

static void run_manage_assets(Game* game, UIController& ui, Player* player, bool raising_funds) {
    while (!ui.input_closed()) {
        ui.process_manage_assets(Manage_Assets_Menu::ROOT, player, raising_funds, {}, {});
        unsigned int back = (raising_funds) ? 3 : 5;
        unsigned int option = ui.read_option(back);
        if (option == 0 || option == back) {
            return;
        }
        Manage_Assets_Menu menu = Manage_Assets_Menu::BUILD;
        if (raising_funds) {
            menu = (option == 1) ? Manage_Assets_Menu::SELL : Manage_Assets_Menu::MORTGAGE;
        } else if (option == 2) {
            menu = Manage_Assets_Menu::SELL;
        } else if (option == 3) {
            menu = Manage_Assets_Menu::MORTGAGE;
        } else if (option == 4) {
            menu = Manage_Assets_Menu::UNMORTGAGE;
        }

        std::vector<Property*> targets = assets_for_menu(player, menu);
        std::vector<std::string> names;
        std::vector<std::string> prices;
        for (Property* property : targets) {
            names.push_back(property->get_name());
            prices.push_back(price_for_menu(property, menu));
        }
        if (!names.empty()) {
            names.push_back("Back");
        }
        ui.process_manage_assets(menu, player, raising_funds, names, prices);
        if (targets.empty()) {
            ui.reaction("Press Enter to continue.");
            ui.wait();
            continue;
        }
        unsigned int choice = ui.read_option(names.size());
        if (choice == 0 || choice == names.size()) {
            continue;
        }
        Property* target = targets[choice - 1];
        Manage_Assets_State result = game->process_manage_assets(menu, target);
        ui.process_manage_assets(result, target->get_name());
        ui.reaction("Press Enter to continue.");
        ui.wait();
    }
}

static void select_properties(UIController& ui, Player* owner, std::vector<Property*>& chosen, const std::string& message) {
    while (!ui.input_closed()) {
        std::vector<Property*> available;
        std::vector<std::string> names;
        for (Tile* asset : owner->get_assets()) {
            Property* property = dynamic_cast<Property*>(asset);
            if (!property || std::find(chosen.begin(), chosen.end(), property) != chosen.end()) {
                continue;
            }
            available.push_back(property);
            names.push_back(property->get_name());
        }
        if (available.empty()) {
            // nothing left to pick, do not make them answer a one-option menu
            if (chosen.empty()) {
                ui.reaction(owner->get_name() + " has no properties to put on the table.");
            }
            return;
        }
        names.push_back("Done");
        ui.prompt(message + " (" + std::to_string(chosen.size()) + " picked so far)", names);
        unsigned int option = ui.read_option(names.size());
        if (option == 0 || option == names.size()) {
            return;
        }
        chosen.push_back(available[option - 1]);
    }
}

static void run_trade(Game* game, UIController& ui) {
    GameContext* context = game->get_context();
    Player* proposer = context->get_current_player();
    std::vector<Player*> opponents = context->get_opponents(proposer);
    if (opponents.empty()) {
        ui.reaction("No available players to trade with.");
        ui.reaction("Press Enter to continue.");
        ui.wait();
        return;
    }
    std::vector<std::string> candidate_names;
    for (Player* opponent : opponents) {
        candidate_names.push_back(opponent->get_name());
    }
    candidate_names.push_back("Cancel");
    ui.prompt("Choose a player to trade with:", candidate_names);
    unsigned int option = ui.read_option(candidate_names.size());
    if (option == 0 || option == candidate_names.size()) {
        return;
    }

    Trade trade;
    trade.proposer = proposer;
    trade.partner = opponents[option - 1];
    trade.cash_offered = 0;
    trade.cash_requested = 0;
    trade.jail_cards_offered = 0;
    trade.jail_cards_requested = 0;

    select_properties(ui, trade.proposer, trade.offered, "Which of your properties are you giving away?");
    select_properties(ui, trade.partner, trade.requested, "Which of " + trade.partner->get_name() + "'s properties do you want?");

    ui.reaction("How much cash are you adding? (0 to " + std::to_string(proposer->get_balance()) + ")");
    trade.cash_offered = ui.read_amount(proposer->get_balance());
    ui.reaction("How much cash are you asking for? (0 to " + std::to_string(trade.partner->get_balance()) + ")");
    trade.cash_requested = ui.read_amount(trade.partner->get_balance());
    if (proposer->get_jail_cards() > 0) {
        ui.reaction("How many jail cards are you adding? (0 to " + std::to_string(proposer->get_jail_cards()) + ")");
        trade.jail_cards_offered = ui.read_amount(proposer->get_jail_cards());
    }
    if (trade.partner->get_jail_cards() > 0) {
        ui.reaction("How many jail cards are you asking for? (0 to " + std::to_string(trade.partner->get_jail_cards()) + ")");
        trade.jail_cards_requested = ui.read_amount(trade.partner->get_jail_cards());
    }

    ui.reaction(context->describe_trade(trade));
    ui.prompt(trade.partner->get_name() + ", do you accept this trade?", {"Yes", "No"});
    if (ui.read_option(2) != 1) {
        ui.reaction("Trade declined.");
        ui.reaction("Press Enter to continue.");
        ui.wait();
        return;
    }
    switch (context->execute_trade(trade)) {
        case Trade_State::SUCCESS:
            ui.reaction("Trade completed successfully.");
            break;
        case Trade_State::FAIL_NO_FUNDS:
            ui.reaction("Trade failed, somebody does not have that much cash.");
            break;
        case Trade_State::FAIL_HAS_BUILDINGS:
            ui.reaction("Trade failed, sell the buildings on those colour sets first.");
            break;
        case Trade_State::FAIL_NOTHING_OFFERED:
            ui.reaction("Trade failed, nothing was actually being swapped.");
            break;
        default:
            ui.reaction("Trade cancelled.");
            break;
    }
    ui.reaction("Press Enter to continue.");
    ui.wait();
}

// one sealed round of bids, the highest bid takes the deed
static End_Turn_State run_auction(Game* game, UIController& ui, Property* property) {
    GameContext* context = game->get_context();
    unsigned int interest = context->transfer_interest(property);
    unsigned int highest = 0;
    Player* leader = nullptr;
    ui.reaction(property->get_name() + " goes to auction. List price $" + std::to_string(property->get_purchase_price()) + ".");
    if (interest > 0) {
        ui.reaction("It is mortgaged, so the winner also owes the bank $" + std::to_string(interest) + " in interest.");
    }
    for (Player* bidder : context->get_active_players()) {
        if (ui.input_closed()) {
            break;
        }
        // the winner has to be able to cover the loan interest on top of their bid
        unsigned int ceiling = (bidder->get_balance() > interest) ? bidder->get_balance() - interest : 0;
        ui.reaction(bidder->get_name() + ", enter your bid (0 to pass, you can go up to $" + std::to_string(ceiling) + "):");
        unsigned int bid = ui.read_amount(ceiling);
        if (bid > highest) {
            highest = bid;
            leader = bidder;
        }
    }
    if (leader && highest > 0 && context->award_auction(leader, property, highest)) {
        ui.reaction(leader->get_name() + " won " + property->get_name() + " for $" + std::to_string(highest) + ".");
        ui.reaction("Press Enter to continue.");
        ui.wait();
        return End_Turn_State::AUCTION_WON;
    }
    ui.reaction("Nobody bid on " + property->get_name() + ".");
    ui.reaction("Press Enter to continue.");
    ui.wait();
    return End_Turn_State::AUCTION_NO_BIDS;
}

// a debt keeps coming back until it is paid off or the player is out of the game
static End_Turn_State run_debt(Game* game, UIController& ui, End_Turn_State state) {
    GameContext* context = game->get_context();
    while (state == End_Turn_State::SHORT_ON_CASH && !ui.input_closed()) {
        Player* debtor = context->get_pending_debtor();
        ui.print_board();
        ui.prompt(debtor->get_name() + " owes $" + std::to_string(context->get_pending_debt()) + " and only holds $" + std::to_string(debtor->get_balance()) + ".",
                  {"Raise funds by selling or mortgaging", "Declare bankruptcy"});
        unsigned int option = ui.read_option(2);
        if (option == 1) {
            run_manage_assets(game, ui, debtor, true);
            state = game->resolve_debt(true);
        } else {
            state = game->resolve_debt(false);
        }
    }
    if (state == End_Turn_State::SHORT_ON_CASH) {
        state = game->resolve_debt(false);
    }
    return state;
}

static unsigned int tile_option_count(GameContext* context, Tile_State tile_state) {
    switch (tile_state) {
        case Tile_State::UNOWNED:
            return (dynamic_cast<Property*>(context->get_current_tile())) ? 2 : 0;
        case Tile_State::OWNED_STREET:
        case Tile_State::OWNED_RAILROAD:
        case Tile_State::OWNED_UTILITY:
        case Tile_State::TAX:
            return 1;
        default:
            return 0;
    }
}

static std::vector<Player*> final_standings(GameContext* context) {
    std::vector<Player*> standings = context->get_players();
    std::sort(standings.begin(), standings.end(), [](const Player* a, const Player* b) {
        if (a->is_active() != b->is_active()) {
            return a->is_active();
        }
        return a->net_worth() > b->net_worth();
    });
    return standings;
}

int main() {
    // Set up game and UI
    UIController ui(nullptr);
    ui.title_screen();
    unsigned int player_count = ui.prompt_player_count();
    if (ui.input_closed()) {
        return 0;
    }
    std::vector<std::string> names = ui.prompt_player_names(player_count);
    if (ui.input_closed()) {
        return 0;
    }
    bool free_parking_pot = ui.prompt_free_parking_rule();
    if (ui.input_closed()) {
        return 0;
    }

    Game* game = new Game(names, free_parking_pot);
    if (!game->is_ready()) {
        std::cout << "Could not start the game: " << game->get_context()->get_board()->get_load_error() << std::endl;
        std::cout << "Run the game from the project root so src/assets/default-board.txt can be found." << std::endl;
        delete game;
        return 1;
    }
    GameContext* context = game->get_context();
    ui.attach(context);

    State state;
    Game_State game_state;
    Tile_State tile_state;
    End_Turn_State end_turn_state;
    std::string card_text;
    unsigned int option;
    bool valid_option = false;

    // Game Loop
    while (!ui.input_closed()){
        context->start_turn();
        Player* player = context->get_current_player();

        game_state = Game_State::NO_REWARD;
        tile_state = Tile_State::UNOWNED;
        end_turn_state = End_Turn_State::CONTINUE;
        valid_option = false;
        bool rolled = false;
        bool turn_over = false;

        // menu phase, the player can manage assets and trade until they commit to a move
        while (!rolled && !turn_over && !ui.input_closed()) {
            state = (player->get_in_jail()) ? State::JAIL : State::NORMAL;
            if (state == State::JAIL && player->get_rounds_in_jail() + 1 >= GameContext::max_rounds_in_jail) {
                state = State::JAIL_LAST_ROUND;
            }
            valid_option = false;
            game_state = Game_State::NO_REWARD;

            ui.process_turn(state);
            option = ui.read_option((state == State::NORMAL) ? 3 : 5);
            if (option == 0) {
                break;
            }
            game->process_turn(state, game_state, option, valid_option);

            if (game_state == Game_State::MANAGE_ASSETS) {
                run_manage_assets(game, ui, player, false);
                continue;
            }
            if (game_state == Game_State::TRADE) {
                run_trade(game, ui);
                continue;
            }
            if (game_state == Game_State::THIRD_DOUBLE_TO_JAIL) {
                ui.print_board();
                ui.whose_turn();
                ui.reaction("Three doubles in a row. Go directly to jail.");
                ui.reaction("Press Enter to continue.");
                ui.wait();
                turn_over = true;
                continue;
            }
            if (game_state == Game_State::STAYED_IN_JAIL) {
                ui.print_board();
                ui.whose_turn();
                ui.reaction("No doubles, you stay in jail.");
                ui.reaction("Press Enter to continue.");
                ui.wait();
                turn_over = true;
                continue;
            }
            if (game_state == Game_State::PAID_JAIL_FINE || game_state == Game_State::FORCED_JAIL_FINE) {
                bool forced = game_state == Game_State::FORCED_JAIL_FINE;
                end_turn_state = run_debt(game, ui, game->resolve_debt(true));
                if (end_turn_state == End_Turn_State::BANKRUPT) {
                    ui.process_turn(end_turn_state);
                    ui.wait();
                    turn_over = true;
                    continue;
                }
                game_state = Game_State::NO_REWARD;
                game->complete_jail_exit(game_state, forced);
                if (forced) {
                    ui.reaction("No doubles on your last attempt, so the $" + std::to_string(GameContext::jail_fine) + " fine was compulsory. You paid it and left jail.");
                } else if (option == 3) {
                    ui.reaction("You used a Get Out of Jail Free card and are out of jail.");
                } else {
                    ui.reaction("You paid the $" + std::to_string(GameContext::jail_fine) + " fine and are out of jail.");
                }
                ui.reaction("Press Enter to continue.");
                ui.wait();
                rolled = forced;
                continue;
            }
            if (!valid_option) {
                ui.reaction("That option is not available right now.");
                ui.reaction("Press Enter to continue.");
                ui.wait();
                continue;
            }
            rolled = true;
        }

        // tile phase, whatever the player landed on now gets its say
        if (rolled && !ui.input_closed()) {
            tile_state = context->get_current_tile()->Action(player);
            ui.process_turn(tile_state, game_state);
            // the GO bonus has been announced, do not let a later redraw claim it again
            game_state = Game_State::NO_REWARD;

            unsigned int draws = 0;
            while ((tile_state == Tile_State::DRAW_CC || tile_state == Tile_State::DRAW_CHANCE) && draws < 3 && !ui.input_closed()) {
                draws++;
                card_text = game->game_process_card(tile_state);
                ui.process_card(card_text);
                ui.wait();
                if (context->has_pending_debt()) {
                    end_turn_state = run_debt(game, ui, game->resolve_debt(true));
                    if (end_turn_state == End_Turn_State::BANKRUPT) {
                        break;
                    }
                }
                while (context->next_collection() && !ui.input_closed()) {
                    Player* payer = context->get_pending_debtor();
                    if (run_debt(game, ui, game->resolve_debt(true)) == End_Turn_State::BANKRUPT) {
                        ui.reaction(payer->get_name() + " could not pay and is out of the game.");
                        ui.reaction("Press Enter to continue.");
                        ui.wait();
                    }
                }
                if (!context->get_advanced()) {
                    break;
                }
                tile_state = context->get_current_tile()->Action(player);
                ui.process_turn(tile_state, game_state);
            }

            if (player->is_active() && !ui.input_closed()) {
                unsigned int tile_options = tile_option_count(context, tile_state);
                if (tile_options > 0) {
                    option = ui.read_option(tile_options);
                } else {
                    option = 1;
                    ui.reaction("Press Enter to continue.");
                    ui.wait();
                }
                end_turn_state = game->process_turn(tile_state, option);
                if (end_turn_state == End_Turn_State::SHORT_ON_CASH) {
                    end_turn_state = run_debt(game, ui, end_turn_state);
                }
                if (end_turn_state == End_Turn_State::PASSED_ON_PROPERTY || end_turn_state == End_Turn_State::NO_MONEY_TO_BUY) {
                    Property* property = dynamic_cast<Property*>(context->get_current_tile());
                    if (property && property->get_owner() == nullptr) {
                        end_turn_state = run_auction(game, ui, property);
                    }
                }
            }
            // ending turn
            ui.process_turn(end_turn_state);
            ui.wait();
        }

        while (context->has_bank_auctions() && !ui.input_closed() && context->get_active_players().size() > 1) {
            Property* seized = context->next_bank_auction();
            if (!seized) {
                break;
            }
            ui.print_board();
            ui.reaction("The bank is selling off what it seized.");
            if (run_auction(game, ui, seized) == End_Turn_State::AUCTION_NO_BIDS) {
                context->abandon_deed(seized);
            }
        }

        if (game->is_over()) {
            ui.announce_game_over(final_standings(context));
            break;
        }

        // a double earns another go, unless it landed the player in jail
        bool extra_turn = rolled && !turn_over && player->is_active() && player->get_rolled_double() && !player->get_in_jail();
        if (!extra_turn) {
            game->end_turn();
        } else {
            ui.reaction("You rolled a double, take another turn.");
            ui.reaction("Press Enter to continue.");
            ui.wait();
        }
    }

    if (!game->is_over()) {
        ui.announce_game_over(final_standings(context));
    }

    delete game;
    return 0;
}
