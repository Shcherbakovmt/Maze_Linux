#pragma once
#include <iostream>
#include <time.h>
#include <locale.h>
#include <iso646.h> // ����� ����� ���� ������ and � or
#include <string>
#include "./SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/include/SFML/Graphics.hpp" // to avoid conflict between SFML-3.0.0 and SFML-2.6.1

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::to_string;

    struct point
    {
        int i;
        int j;
    };
    struct river_path
    {
        point* river;
        int count_of_r_around_summ = 0;
        int count_river_along_bord = 0;
        int* toched_walls = 0;
        int numb_toched_walls = 0;
        int Count_3r_around = 0;
        int Count_4r_around = 0;

    };
    struct player
    {
        sf::Texture texture;
        string name;
        int id = 0;
        point coord;
        bool with_treasure = 0;
        int hp = 2;
        int bullets = 0;
        int granade = 0;
        int TimeInSwamp = 0;
        int turns = 0;
    };
    struct crocodile
    {
        bool alive;
        int time_to_res;
        point coord;
        sf::Texture alive_text;
        sf::Texture dead_text;
    };






