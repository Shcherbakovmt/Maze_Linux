#pragma once
#include "structures.cpp"
class elements
{
public:
    point center_of_swamp;
    point treasure;
    point medbat;
    point arsenal;
    point u_mouth;
    point pit1, pit2, pit3, bum1, bum2, bum3; // ïî óìîë÷àíèþ äâà áóìà
    point* complete_river; // ôèíàëüíàÿ ðåêà
    char** lines;
    crocodile croc;
    elements();
    ~elements();
};
