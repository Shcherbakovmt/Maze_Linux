#include "technical.h"
#include <iostream>
#include <time.h>
#include <locale.h>
#include <iso646.h> // ����� ����� ���� ������ and � or
#include <string>
#include "structures.cpp"
using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::to_string;

technical::technical()
{
    seed = time(0);
    show_mase = 0;
    command_changed = 0;
    treasure_picked = 0;
    turns_1 = 0;
    turns_2 = 0;
    sucsess_river = 0;
    sucsess_bum = 0;
    sucsess_add_swamp = 0;
    count_of_land = 0;
    command = "";
    action = "";
    count_of_path = 0;
    tries = 0;
    min_path_index = 0;
    repets = 0;
    arr_of_land_points = new point[64];
    paths = new river_path[2000000];
    river = new point[64];
}

technical::~technical()
{

    for (int k = 0; k < count_of_path; k++)
    {
        delete[] paths[k].river;
    }
    delete[] river;
    delete[] arr_of_land_points;
    delete[] paths;
}
