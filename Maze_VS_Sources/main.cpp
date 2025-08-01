#include <iostream>
#include <time.h>
#include <locale.h>
#include <iso646.h> // Чтобы можно было писать and и or
#include <string>
#include <thread>  // Required for std::this_thread::sleep_for
#include <chrono>
#include "Settings.h"
#include "technical.h"
#include "structures.cpp"
#include "elements.h"
#include "Maze.h"

using namespace sf;


using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::to_string;

int main()
{
    sf::Font ft;
    ft.openFromFile("Fonts/Helvetica.ttf");
    
    sf::RenderWindow window(sf::VideoMode({ 1280, 960 }), "The maze");


    sf::Text text(ft);
    sf::Text EnteredText(ft);
    EnteredText.setPosition({ 0,600 });
    
    Maze labirint(window, text, EnteredText);
    cout << "Sucsess created Maze" << endl;

    while (window.isOpen())
    {
        labirint.turn(text, window);

    }
}
