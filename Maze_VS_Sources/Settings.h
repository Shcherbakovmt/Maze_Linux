#pragma once
#include <iostream>
using std::cout;
class settings
{
public:
    bool respawn_new_place;
    int k_along_bord;
    int k_r_around;
    int k_touch_walls;
    int quadr_dist_between_pits;
    int add_swamp_max;
    int add_swamp_min;
    int number_of_river;
    int Chance_For_Treasure_In_Boom;
    settings();
};
