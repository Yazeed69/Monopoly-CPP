#include "Dice.h"


Dice::Dice() : faces(6), engine(std::chrono::system_clock::now().time_since_epoch().count()){
    roll();
}

unsigned int Dice::get_rolled() const {
    return rolled;
}

void Dice::roll(){
    std::uniform_int_distribution<unsigned int> uniform_dist(1, faces);
    this->rolled = uniform_dist(engine);
}