#include "UIController.h"
#include "Board.h"

#include <limits>


UIController::UIController(GameContext* context) {
    this->context = context;
    this->closed_input = false;
}

void UIController::attach(GameContext* context) {
    this->context = context;
}

void UIController::clear_console() {
    std::cout << "\033[2J\033[1;1H";
}

void UIController::print_board() {
    clear_console();
    std::cout << get_status_line() << "\n"
          << "       " << get_path(20) << "  " << get_path(21) << get_path(22) << get_path(23) << get_path(24) << get_path(25) << get_path(26) << get_path(27) << get_path(28) << get_path(29) << "  " << get_path(30) << "    \n"
          << "        " << get_abbreviation(20) << "    " << get_abbreviation(21) << "  " << get_abbreviation(22) << "  " << get_abbreviation(23) << "  " << get_abbreviation(24) << "  " << get_abbreviation(25) << "  " << get_abbreviation(26) << "  " << get_abbreviation(27) << "  " << get_abbreviation(28) << "  " << get_abbreviation(29) << "    " << get_abbreviation(30) << "     \t\t" << get_player_name(0) << "\n"
          << "   " << get_path(19) << " " << get_abbreviation(19) << " " << get_houses(19) << " " << get_houses(21) << "  " << "  " << "  " << get_houses(23) << "  " << get_houses(24) << "  " << "  " << "  " << get_houses(26) << "  " << get_houses(27) << "  " << "  " << "  " << get_houses(29) << " " << get_houses(31) << " " << get_abbreviation(31) << " " << get_path(31) << "\t\t" << get_player_assets(0) << "\n"
          << "   " << get_path(18) << " " << get_abbreviation(18) << " " << get_houses(18) << "                                    " << get_houses(32) << " " << get_abbreviation(32) << " " << get_path(32) << "\n"
          << "   " << get_path(17) << " " << get_abbreviation(17) << "                                          "  << get_abbreviation(33) << " " << get_path(33) << "\t\t" << get_player_name(1) << "\n"
          << "   " << get_path(16) << " " << get_abbreviation(16) << " " << get_houses(16) << "                 " << get_dice_roll(0) << " C                " << get_houses(34) << " " << get_abbreviation(34) << " " << get_path(34) << "\t\t" << get_player_assets(1) << "\n"
          << "   " << get_path(15) << " " << get_abbreviation(15) << "                  ? " << get_dice_roll(1) << "                     " << get_abbreviation(35) << " " << get_path(35) << "\n"
          << "   " << get_path(14) << " " << get_abbreviation(14) << " " << get_houses(14) << "                                       " << get_abbreviation(36) << " " << get_path(36) << "\t\t" << get_player_name(2) << "\n"
          << "   " << get_path(13) << " " << get_abbreviation(13) << " " << get_houses(13) << "                                    " << get_houses(37) << " " << get_abbreviation(37) << " " << get_path(37) << "\t\t" << get_player_assets(2) << "\n"
          << "   " << get_path(12) << " " << get_abbreviation(12) << "                                          "  << get_abbreviation(38) << " " << get_path(38) << "\n"
          << "   " << get_path(11) << " " << get_abbreviation(11) << " " << get_houses(11) << " " << get_houses(9) << "  " << get_houses(8) << "      " << get_houses(6) << "          " << get_houses(3) << "      " << get_houses(1) << " " << get_houses(39) << " " << get_abbreviation(39) << " " << get_path(39) << "\t\t" << get_player_name(3) << "\n"
          << "    " << get_abbreviation(10) << "    " << get_abbreviation(9) << "  " << get_abbreviation(8) << "  " << get_abbreviation(7) << "  " << get_abbreviation(6) << "  " << get_abbreviation(5) << "  " << get_abbreviation(4) << "  " << get_abbreviation(3) << "  " << get_abbreviation(2) << "  " << get_abbreviation(1) << "    " << get_abbreviation(0) <<  "     \t\t" << get_player_assets(3) << "\n"
          << "       " << get_path(10) << "  " << get_path(9) << get_path(8) << get_path(7) << get_path(6) << get_path(5) << get_path(4) << get_path(3) << get_path(2) << get_path(1) << "  " << get_path(0) << "    \n";
}

std::string UIController::get_status_line() const {
    std::ostringstream oss;
    oss << "houses " << context->get_houses_available() << "  hotels " << context->get_hotels_available();
    FreeParking* free_parking = context->get_free_parking();
    if (free_parking && free_parking->get_using_pot()) {
        oss << "  pot $" << free_parking->peek_pot();
    }
    return oss.str();
}

const std::string& UIController::get_path(unsigned int i) const {
    return context->get_board()->get_tile(i)->get_path();
}

std::string UIController::get_houses(unsigned int i) const {
    Street* street = dynamic_cast<Street*>(context->get_board()->get_tile(i));
    // only streets carry buildings, everything else keeps the slot blank
    return (street) ? street->get_houses() : "  ";
}

const std::string& UIController::get_abbreviation(unsigned int i) const {
    return context->get_board()->get_tile(i)->get_abbreviation();
}

std::string UIController::get_player_name(unsigned int i) const {
    if (i >= context->get_players().size()) {
        return "";
    }
    Player* player = context->get_players()[i];
    std::ostringstream oss;
    oss << player->get_name() << " (" << player->get_pawn() << ")";
    if (!player->is_active()) {
        oss << " - bankrupt";
    } else if (player == context->get_current_player()) {
        oss << " <-";
    }
    return oss.str();
}

std::string UIController::get_player_assets(unsigned int i) const {
    if (i >= context->get_players().size()) {
        return "";
    }
    Player* player = context->get_players()[i];
    std::ostringstream oss;
    oss << "$" <<  player->get_balance() << "  " << player->assets_string();
    if (player->get_jail_cards() > 0) {
        oss << "[jail x" << player->get_jail_cards() << "] ";
    }
    return oss.str();
}

std::string UIController::get_dice_roll(unsigned int i) const {
    std::ostringstream oss;
    oss << context->get_die(i)->get_rolled();
    return oss.str();
}

void UIController::title_screen() {
    clear_console();
    std::cout << "  MONOPOLY\n"
              << "  a terminal rules engine\n\n";
}

unsigned int UIController::prompt_player_count() {
    prompt("How many players?", {"2 players", "3 players", "4 players"});
    unsigned int option = read_option(3);
    return (option == 0) ? 2 : option + 1;
}

std::vector<std::string> UIController::prompt_player_names(unsigned int count) {
    std::vector<std::string> names;
    for (unsigned int i = 0; i < count; i++) {
        std::string fallback = "Player " + std::to_string(i + 1);
        std::string name = read_text("Name for player " + std::to_string(i + 1) + " (pawn " + static_cast<char>('A' + i) + "), blank for " + fallback + ":");
        names.push_back((name.empty()) ? fallback : name);
    }
    return names;
}

bool UIController::prompt_free_parking_rule() {
    return read_yes("House rule: should taxes and fines pile up under Free Parking?");
}

void UIController::process_turn(State& state) {
    print_board();
    whose_turn();
    Player* player = context->get_current_player();
    std::string jail_card_option = "Use Get Out of Jail Free card (" + std::to_string(player->get_jail_cards()) + " held)";
    switch (state) {
        case State::NORMAL:
            prompt("What would you like to do?", {"Roll", "Manage Assets", "Trade"});
            break;
        case State::JAIL:
            prompt("You are in jail, attempt " + std::to_string(player->get_rounds_in_jail() + 1) + " of " + std::to_string(GameContext::max_rounds_in_jail) + ". What would you like to do?",
                   {"Roll for doubles", "Pay $" + std::to_string(GameContext::jail_fine), jail_card_option, "Manage Assets", "Trade"});
            break;
        case State::JAIL_LAST_ROUND:
            prompt("Last attempt in jail. Fail this roll and the $" + std::to_string(GameContext::jail_fine) + " fine is compulsory. What would you like to do?",
                   {"Roll for doubles", "Pay $" + std::to_string(GameContext::jail_fine), jail_card_option, "Manage Assets", "Trade"});
            break;
    }
}

void UIController::process_turn(Tile_State& state, Game_State& game_state) {
    print_board();
    whose_turn();
    Tile* tile = context->get_current_tile();
    Player* player = context->get_current_player();
    if (game_state == Game_State::PASSED_GO_REWARD) {
        reaction("You passed go and collected $" + std::to_string(GameContext::go_bonus) + ".");
    } else if (game_state == Game_State::LANDED_ON_GO_REWARD) {
        reaction("You landed on go and collected $" + std::to_string(GameContext::go_bonus) + ".");
    }
    switch (state) {
        case Tile_State::UNOWNED: {
            Property* property = dynamic_cast<Property*>(tile);
            if (!property) {
                reaction("You landed on " + tile->get_name());
                break;
            }
            prompt("You landed on " + tile->get_name() + ".\nIt is unowned and costs $" + std::to_string(property->get_purchase_price()) + ".\nWhat would you like to do?", {"Buy", "Pass (goes to auction)"});
            break;
        }
        case Tile_State::OWN_PROPERTY:
            reaction("You landed on " + tile->get_name() + ".\n" + "You own this property.\n");
            break;
        case Tile_State::OWNED_STREET:
        case Tile_State::OWNED_RAILROAD:
        case Tile_State::OWNED_UTILITY: {
            Property* property = dynamic_cast<Property*>(tile);
            std::string owner_name = (property && property->get_owner()) ? property->get_owner()->get_name() : "the bank";
            prompt("You landed on " + tile->get_name() + ".\n" + owner_name + " owns this property.\nYou owe $" + std::to_string(context->rent_due(tile, player)) + ".\nWhat would you like to do?", {"Pay Rent"});
            break;
        }
        case Tile_State::MORTGAGED:
            reaction("You landed on " + tile->get_name() + ".\n" + "This property is mortgaged, so no rent is due.\n");
            break;
        case Tile_State::DRAW_CC:
        case Tile_State::DRAW_CHANCE:
            reaction("You landed on " + tile->get_name() + ".\n" + "You drew a card.\n");
            break;
        case Tile_State::TAX: {
            Tax* tax_tile = dynamic_cast<Tax*>(tile);
            unsigned int tax = (tax_tile) ? tax_tile->get_tax() : 0;
            prompt("You landed on " + tile->get_name() + ".\n" + "You owe $" + std::to_string(tax) + ".\nWhat would you like to do?", {"Pay Tax"});
            break;
        }
        case Tile_State::FREE:
            reaction("You landed on " + tile->get_name() + ".\n" + "You get to take a break.\n");
            break;
        case Tile_State::FREE_POT: {
            FreeParking* parking_tile = dynamic_cast<FreeParking*>(tile);
            int pot = (parking_tile) ? parking_tile->peek_pot() : 0;
            if (pot > 0) {
                reaction("You landed on " + tile->get_name() + ".\n" + "You get $" + std::to_string(pot) + " from the pot.\n");
            } else {
                reaction("You landed on " + tile->get_name() + ".\n" + "The pot is empty.\n");
            }
            break;
        }
        case Tile_State::HYBRID:
            reaction("You landed on " + tile->get_name() + ".\n" + "Just visiting.\n");
            break;
        case Tile_State::GO:
            reaction("You landed on " + tile->get_name() + ".\n");
            break;
        case Tile_State::TO_JAIL:
            reaction("You landed on " + tile->get_name() + ".\n" + "You are going to jail.\n");
            break;
    }
}

void UIController::process_turn(End_Turn_State& state) {
    print_board();
    whose_turn();
    switch (state) {
        case End_Turn_State::BOUGHT_PROPERTY:
            reaction("You bought " + context->get_current_tile()->get_name());
            break;
        case End_Turn_State::PASSED_ON_PROPERTY:
            reaction("You passed on " + context->get_current_tile()->get_name());
            break;
        case End_Turn_State::NO_MONEY_TO_BUY:
            reaction("You cannot afford " + context->get_current_tile()->get_name() + ".");
            break;
        case End_Turn_State::AUCTION_WON:
            reaction("The auction closed.");
            break;
        case End_Turn_State::AUCTION_NO_BIDS:
            reaction("Nobody bid, " + context->get_current_tile()->get_name() + " stays with the bank.");
            break;
        case End_Turn_State::PAYED_RENT:
            reaction("You payed " + context->get_current_tile()->get_name() + " owner $" + std::to_string(context->get_current_player()->get_last_paid_rent()) + " for rent.");
            break;
        case End_Turn_State::PAYED_TAX:
            reaction("You payed $" + std::to_string(context->get_current_player()->get_last_tax_paid()) + " in taxes.");
            break;
        case End_Turn_State::FREE_POT:
            reaction("You collected the free parking pot.");
            break;
        case End_Turn_State::TO_JAIL:
            reaction("You are going to jail.");
            break;
        case End_Turn_State::BANKRUPT:
            reaction("You went bankrupt and are out of the game.");
            break;
        case End_Turn_State::SHORT_ON_CASH:
            reaction("You still owe money.");
            break;
        case End_Turn_State::CONTINUE:
            break;
    }
    reaction("Your turn is over. Press Enter to continue.");
}

void UIController::process_card(std::string &card_text) {
    print_board();
    whose_turn();
    reaction("You drew a card.\n" + card_text);
    reaction("Press Enter to continue.");
}

void UIController::process_manage_assets(Manage_Assets_Menu menu, Player* player, bool raising_funds, const std::vector<std::string>& options, const std::vector<std::string>& prices) {
    print_board();
    whose_turn();
    // a "collect from each player" card can put somebody other than the current player in the chair
    std::string who = (player) ? player->get_name() + ", " : "";
    switch (menu) {
        case Manage_Assets_Menu::ROOT:
            if (raising_funds) {
                // while a debt is outstanding the only useful moves are the ones that bring cash in
                prompt(who + "raise cash to cover what you owe.",
                       {"Sell a house or hotel", "Mortgage a property", "Back"});
            } else {
                prompt("Manage your assets. Bank stock: " + std::to_string(context->get_houses_available()) + " houses, " + std::to_string(context->get_hotels_available()) + " hotels.",
                       {"Build a house or hotel", "Sell a house or hotel", "Mortgage a property", "Lift a mortgage", "Back"});
            }
            break;
        case Manage_Assets_Menu::BUILD:
            if (options.empty()) {
                reaction(who + "you have no full colour set that can take another building.");
            } else {
                prompt(who + "where would you like to build?", options, prices);
            }
            break;
        case Manage_Assets_Menu::SELL:
            if (options.empty()) {
                reaction(who + "you have no buildings to sell.");
            } else {
                prompt(who + "which building would you like to sell back to the bank?", options, prices);
            }
            break;
        case Manage_Assets_Menu::MORTGAGE:
            if (options.empty()) {
                reaction(who + "you have nothing left to mortgage.");
            } else {
                prompt(who + "which property would you like to mortgage?", options, prices);
            }
            break;
        case Manage_Assets_Menu::UNMORTGAGE:
            if (options.empty()) {
                reaction(who + "you have no mortgaged properties.");
            } else {
                prompt(who + "which mortgage would you like to lift?", options, prices);
            }
            break;
        case Manage_Assets_Menu::EXIT:
            break;
    }
}

void UIController::process_manage_assets(Manage_Assets_State state, const std::string& property_name) {
    switch (state) {
        case Manage_Assets_State::BUILD_SUCCESS:
            reaction("You successfully built on " + property_name);
            break;
        case Manage_Assets_State::BUILD_FAIL_NO_MONEY:
            reaction("You do not have enough money to build on " + property_name);
            break;
        case Manage_Assets_State::BUILD_FAIL_NOT_EQUAL_BUILDINGS:
            reaction("You must build an equal number of buildings on each property in a set.");
            break;
        case Manage_Assets_State::BUILD_FAIL_MAX_BUILDINGS:
            reaction(property_name + " already has a hotel, nothing more can go up there.");
            break;
        case Manage_Assets_State::BUILD_FAIL_MORTGAGED:
            reaction("You cannot build while any property in that set is mortgaged.");
            break;
        case Manage_Assets_State::BUILD_FAIL_NO_HOUSES_LEFT:
            reaction("The bank is out of houses.");
            break;
        case Manage_Assets_State::BUILD_FAIL_NO_HOTELS_LEFT:
            reaction("The bank is out of hotels.");
            break;
        case Manage_Assets_State::SELL_SUCCESS:
            reaction("You sold a building on " + property_name + " back to the bank.");
            break;
        case Manage_Assets_State::SELL_FAIL_NOT_EQUAL_BUILDINGS:
            reaction("You must sell buildings evenly across a set.");
            break;
        case Manage_Assets_State::SELL_FAIL_NO_BUILDINGS:
            reaction("There is nothing built on " + property_name + ".");
            break;
        case Manage_Assets_State::SELL_FAIL_NO_HOUSES_LEFT:
            reaction("The bank does not have four houses to break that hotel into.");
            break;
        case Manage_Assets_State::MORTGAGE_SUCCESS:
            reaction("You mortgaged " + property_name + ".");
            break;
        case Manage_Assets_State::MORTGAGE_FAIL_HAS_BUILDINGS:
            reaction("Sell the buildings on that colour set before mortgaging.");
            break;
        case Manage_Assets_State::UNMORTGAGE_SUCCESS:
            reaction("You lifted the mortgage on " + property_name + ".");
            break;
        case Manage_Assets_State::UNMORTGAGE_FAIL_NO_MONEY:
            reaction("You cannot afford to lift that mortgage.");
            break;
        case Manage_Assets_State::NULL_STATE:
            break;
    }
}

void UIController::announce_game_over(const std::vector<Player*>& standings) {
    print_board();
    reaction("Game over.");
    unsigned int place = 1;
    for (Player* player : standings) {
        reaction(std::to_string(place++) + ". " + player->get_name() + " - $" + std::to_string(player->get_balance()) + " cash, $" + std::to_string(player->net_worth()) + " net worth" + ((player->is_active()) ? "" : " (bankrupt)"));
    }
}

void UIController::whose_turn() {
    player_stream << "It is " << context->get_current_player()->get_name() << "'s turn. ($" << context->get_current_player()->get_balance() << ")" << std::endl;
    std::cout << player_stream.str();
    player_stream.str("");
    player_stream.clear();
}

void UIController::prompt(std::string message, std::vector<std::string> options) {
    prompt_stream << message << std::endl;
    unsigned int i = 1;
    for (std::string& option : options){
        prompt_stream << i++ << ". " << option << std::endl;
    }
    std::cout << prompt_stream.str();
    prompt_stream.str("");
    prompt_stream.clear();
}

void UIController::prompt(std::string message, std::vector<std::string> options, std::vector<std::string> prices) {
    prompt_stream << message << std::endl;
    for (std::vector<std::string>::size_type i = 0; i < options.size(); i++){
        prompt_stream << (i + 1) << ". " << options[i];
        if (i < prices.size()) {
            prompt_stream << " $" << prices[i];
        }
        prompt_stream << std::endl;
    }
    std::cout << prompt_stream.str();
    prompt_stream.str("");
    prompt_stream.clear();
}

void UIController::prompt(std::string &message, std::vector<std::string> &options) {
    prompt_stream << message << std::endl;
    unsigned int i = 1;
    for (std::string& option : options){
        prompt_stream << i++ << ". " << option << std::endl;
    }
    std::cout << prompt_stream.str();
    prompt_stream.str("");
    prompt_stream.clear();
}

void UIController::reaction(std::string& message){
    reaction_stream << message << std::endl;
    std::cout << reaction_stream.str();
    reaction_stream.str("");
    reaction_stream.clear();
}

void UIController::reaction(std::string message){
    reaction_stream << message << std::endl;
    std::cout << reaction_stream.str();
    reaction_stream.str("");
    reaction_stream.clear();
}

unsigned int UIController::read_option(unsigned int max) {
    if (max == 0) {
        return 0;
    }
    while (!closed_input) {
        unsigned int option = 0;
        if (std::cin >> option) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (option >= 1 && option <= max) {
                return option;
            }
        } else if (std::cin.eof()) {
            closed_input = true;
            break;
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        reaction("Please enter a number between 1 and " + std::to_string(max) + ".");
    }
    return 0;
}

unsigned int UIController::read_amount(unsigned int max) {
    while (!closed_input) {
        unsigned int amount = 0;
        if (std::cin >> amount) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (amount <= max) {
                return amount;
            }
        } else if (std::cin.eof()) {
            closed_input = true;
            break;
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        reaction("Please enter an amount between 0 and " + std::to_string(max) + ".");
    }
    return 0;
}

std::string UIController::read_text(const std::string& message) {
    if (closed_input) {
        return "";
    }
    reaction(message);
    std::string line;
    if (!std::getline(std::cin, line)) {
        closed_input = true;
        return "";
    }
    // trim the carriage return a windows pipe leaves behind
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
        line.pop_back();
    }
    return line;
}

bool UIController::read_yes(const std::string& message) {
    if (closed_input) {
        return false;
    }
    prompt(message, {"Yes", "No"});
    return read_option(2) == 1;
}

void UIController::wait() {
    if (closed_input) {
        return;
    }
    std::string line;
    if (!std::getline(std::cin, line)) {
        closed_input = true;
    }
}

bool UIController::input_closed() const {
    return closed_input;
}
