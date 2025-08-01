#include "Settings.h"
#include <iostream>
using std::cout;
using std::endl;

settings::settings()
{
    respawn_new_place = 0; // если == 1, то крокодил перерождается каждый раз в рандомном месте
    k_along_bord = 1; // штраф за прилегание реки к стенам лабиринта
    k_r_around = 1; // штраф за то что река слишком много контактирует сама с собой
    k_touch_walls = -4; // поощрение за касание рекой одной из 4 стен лабиринта
    quadr_dist_between_pits = 9; // квадрат минимального расстояния между ямами
    add_swamp_max = 5; // диапозон добавочного болота
    add_swamp_min = 2;
    number_of_river = 26; // количество рек + устье
    Chance_For_Treasure_In_Boom = 5;
    cout << "The default settings are set" << endl;
}
