#pragma once
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

class technical
{
public:

    time_t seed;
    bool show_mase;
    bool command_changed;
    bool treasure_picked;
    int turns_1;
    int turns_2;
    bool sucsess_river;
    bool sucsess_bum;
    bool sucsess_add_swamp;
    int count_of_land;
    string command;
    string action;
    int count_of_path;
    long tries;
    long min_path_index;
    int repets;
    point* arr_of_land_points;
    river_path* paths;
    point* river;
    technical();
    ~technical();
};

