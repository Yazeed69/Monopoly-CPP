#ifndef DICE_H
#define DICE_H

#include <random>
#include <chrono>

class Dice {
    public:
        Dice();
        
       unsigned int get_rolled() const;
        void roll();

    private:
        unsigned int faces;
        unsigned int rolled;
        std::default_random_engine engine;
};

#endif
