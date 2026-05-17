#include "Settings.h"
#include <iostream>
using std::cout;
using std::endl;

settings::settings()
{
    respawn_new_place = 0; // åñëè == 1, òî êðîêîäèë ïåðåðîæäàåòñÿ êàæäûé ðàç â ðàíäîìíîì ìåñòå
    k_along_bord = 1; // øòðàô çà ïðèëåãàíèå ðåêè ê ñòåíàì ëàáèðèíòà
    k_r_around = 1; // øòðàô çà òî ÷òî ðåêà ñëèøêîì ìíîãî êîíòàêòèðóåò ñàìà ñ ñîáîé
    k_touch_walls = -4; // ïîîùðåíèå çà êàñàíèå ðåêîé îäíîé èç 4 ñòåí ëàáèðèíòà
    quadr_dist_between_pits = 9; // êâàäðàò ìèíèìàëüíîãî ðàññòîÿíèÿ ìåæäó ÿìàìè
    add_swamp_max = 5; // äèàïîçîí äîáàâî÷íîãî áîëîòà
    add_swamp_min = 2;
    number_of_river = 26; // êîëè÷åñòâî ðåê + óñòüå
    Chance_For_Treasure_In_Boom = 5;
    cout << "The default settings are set" << endl;
}
