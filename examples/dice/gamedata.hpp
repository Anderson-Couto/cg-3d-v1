#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Roll };

struct GameData {
    std::bitset<1> m_input;  // [ roll ]
};

#endif