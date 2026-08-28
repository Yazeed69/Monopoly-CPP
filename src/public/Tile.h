#ifndef TILE_H
#define TILE_H

#include <string>
#include "State.h"

class Player;

class Tile {
    public:
        Tile();
        Tile(const std::string& name, const std::string& abbreviation, const std::string& desc, const std::string& UI_color);
        virtual ~Tile();

        const std::string& get_name() const;
        const std::string& get_abbreviation() const;
        const std::string& get_desc() const;

        void set_name(const std::string& name);
        void set_abbreviation(const std::string& abbreviation);
        void set_desc(const std::string& desc);

        const std::string& get_path() const;

        void enter_path(char pawn);
        void exit_path(char pawn);

        Tile_State get_state() const;
        void set_state(Tile_State state);

        virtual Tile_State Action() = 0;
        virtual Tile_State Action(Player* player) = 0;

    private:
        const std::string& get_UI_color() const;
        std::string name;
        std::string abbreviation;
        std::string desc;
        std::string path;
        Tile_State state;
    
    protected:
        std::string UI_color;
};

#endif
