#include "Settings.h"
#include <iostream>
using std::cout;
using std::endl;

settings::settings()
{
    respawn_new_place = 0; // ���� == 1, �� �������� ������������� ������ ��� � ��������� �����
    k_along_bord = 1; // ����� �� ���������� ���� � ������ ���������
    k_r_around = 1; // ����� �� �� ��� ���� ������� ����� ������������ ���� � �����
    k_touch_walls = -4; // ��������� �� ������� ����� ����� �� 4 ���� ���������
    quadr_dist_between_pits = 9; // ������� ������������ ���������� ����� �����
    add_swamp_max = 5; // �������� ����������� ������
    add_swamp_min = 2;
    number_of_river = 26; // ���������� ��� + �����
    Chance_For_Treasure_In_Boom = 5;
    cout << "The default settings are set" << endl;
}
