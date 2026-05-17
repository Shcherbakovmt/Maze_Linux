#pragma once
#include <iostream>
#include <time.h>
#include <locale.h>
#include <iso646.h> // ����� ����� ���� ������ and � or
#include <string>
#include "./SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/include/SFML/Graphics.hpp" // to avoid conflict between SFML-3.0.0 and SFML-2.6.1
#include <thread>  // Required for std::this_thread::sleep_for
#include <chrono>  // Required for time units like std::chrono::seconds
#include "Settings.h"
#include "technical.h"
#include "structures.cpp"
#include "elements.h"
#include <vector>

using namespace sf;


using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::to_string;
using std::vector;

class Maze : public elements, public settings, public technical
{
public:
    // --- Снапшот состояния игры для отмотки ходов назад ---
    struct GameSnapshot
    {
        vector<player> players;
        crocodile      croc;
        point          treasure;
        bool           treasure_picked;
        // Карта 10×10 (может меняться при взрыве стен)
        char           lines_snap[10][10];
        string         log_text; // текст лога на момент снапшота
    };

    vector<player> players;
    sf::Texture background;
    bool TexturesIsDone = 0;
    sf::Texture** textures;
    sf::Texture** unknown;
    char temp_3;
    int trash = 0;

    sf::Texture TreasureTexture;
    sf::Texture ButtonTexture;
    float scale = 0.3;
    sf::Font font;

    bool unknown_created = 0;
    int current_player_idx = -1; // индекс текущего ходящего игрока (-1 = никто)

    // История ходов (максимум 20 снапшотов)
    static const int MAX_HISTORY = 20;
    vector<GameSnapshot> history;

    // Сохранить снапшот перед ходом
    void save_snapshot(const sf::Text& text)
    {
        if (static_cast<int>(history.size()) >= MAX_HISTORY)
            history.erase(history.begin());
        GameSnapshot snap;
        snap.players        = players;
        snap.croc           = croc;
        snap.treasure       = treasure;
        snap.treasure_picked = treasure_picked;
        snap.log_text       = text.getString();
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++)
                snap.lines_snap[i][j] = lines[i][j];
        history.push_back(snap);
    }

    // Восстановить последний снапшот
    bool restore_snapshot(sf::Text& text)
    {
        if (history.empty()) return false;
        GameSnapshot& snap = history.back();
        players          = snap.players;
        croc             = snap.croc;
        treasure         = snap.treasure;
        treasure_picked  = snap.treasure_picked;
        text.setString(snap.log_text + "[Rewound 1 move]\n");
        for (int i = 0; i < 10; i++)
            for (int j = 0; j < 10; j++)
                lines[i][j] = snap.lines_snap[i][j];
        // Перестроить текстуры карты по восстановленным lines
        if (TexturesIsDone) textures = CreateTextures();
        history.pop_back();
        return true;
    }


    sf::Texture** CreateUnknown()
    {
        sf::Texture** unknown = new sf::Texture * [10];

        for (int i = 0; i < 10; i++)
        {
            unknown[i] = new sf::Texture[10];
        }
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                (void)unknown[i][j].loadFromFile("Textures/Unknown.png");
            }
        }
        for (int i = 0; i < 10; i++)
        {
            (void)unknown[0][i].loadFromFile("Textures/Wall.png");
            (void)unknown[9][i].loadFromFile("Textures/Wall.png");
            (void)unknown[i][0].loadFromFile("Textures/Wall.png");
            (void)unknown[i][9].loadFromFile("Textures/Wall.png");
        }
        unknown_created = 1;
        return unknown;
    }
    void draw_unknown_lab(sf::RenderWindow& window) // ���� �� ����� ����� ������� ��������, �������� ������� � ���������� ��� ���������� ��������� �������
    {
        if (not(unknown_created))
        {
            cout << "createUnknown\n";
            unknown = CreateUnknown();
        }
        for (int i = 0; i < 10; i++) // ���������� �� ��� y ����
        {
            for (int j = 0; j < 10; j++) // �� ��� x
            {
                sf::Sprite spr(unknown[i][j]);
                spr.setPosition({ window.getSize().x + scale * (j - 10) * 160.f, scale * i * 160.f }); // ����� = window.getSize().x - scale * 10 * 160
                spr.setScale({ scale * 1.f, scale * 1.f });
                window.draw(spr);
            }
        }
    }

    void Button_Pressed(sf::RenderWindow& window, sf::Text& text)
    {
        // Не вызываем pollEvent здесь — это "съедало" бы TextEntered/KeyPressed события
        // у вызывающих функций. Closed обрабатывается в их собственных циклах pollEvent.
        sf::Vector2i MousePosition = sf::Mouse::getPosition(window);
        // Кнопка теперь расположена правее лога, под картой лабиринта
        int btnX = static_cast<int>(window.getSize().x * 0.62f) + 10;
        int btnY = static_cast<int>(scale * 10 * 160.f) + 10;
        if (MousePosition.x > btnX and
            MousePosition.y > btnY and
            MousePosition.x < btnX + 290 and
            MousePosition.y < btnY + 100)
        {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            {
                if(show_mase == 1) show_mase = 0;
                else if (show_mase == 0) show_mase = 1;
                WindowClearDrawDisplay(window, text);
                while (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) { continue; }
                return;
            }
        }
    }

    point choose_point(sf::RenderWindow& window, sf::Text& text)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        point ans;
        bool trig = 0;
        while (trig == 0)
        {
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>())
                    window.close();
            }

            Button_Pressed(window, text); // ����� �� ������ �� ������������ �������
            sf::Vector2i MousePosition = sf::Mouse::getPosition(window);
            if (window.getSize().x - scale * 160.f > MousePosition.x and window.getSize().x - scale * 9 * 160.f < MousePosition.x and
                scale * 9 * 160.f > MousePosition.y and scale * 160.f < MousePosition.y)
            {
                int j = (int)((MousePosition.x - (window.getSize().x - scale * 9 * 160)) / (160 * scale)) + 1;
                int i = (int)(MousePosition.y - scale * 160) / (160 * scale) + 1;
                //cout << "i, j = " << i << " " << j << endl;

                sf::Texture highlighted("Textures/UnknownHighlighted.png");
                sf::Sprite HLSpr(highlighted);
                HLSpr.setScale({ scale * 1.f, scale * 1.f });
                HLSpr.setPosition({ window.getSize().x + scale * (j - 10) * 160.f, scale * i * 160.f });

                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    ans.i = i;
                    ans.j = j;
                    trig = 1;
                }
                WindowClearDrawDisplay(window, text, HLSpr);
            }
        }
        return ans;
    }

    void draw_lab_SFML(sf::RenderWindow& window)
    {

        float scale = 0.3;
        for (int i = 0; i < 10; i++) // ���������� �� ��� y ����
        {
            for (int j = 0; j < 10; j++) // �� ��� x
            {
                sf::Sprite spr(textures[i][j]);
                spr.setPosition({ window.getSize().x + scale * (j - 10) * 160.f, scale * i * 160.f }); // ����� = window.getSize().x - scale * 10 * 160
                spr.setScale({ scale * 1.f, scale * 1.f });
                window.draw(spr);
            }
        }
        sf::Sprite TreasureSprite(TreasureTexture);


        for (auto &player : players)
        {
            sf::Sprite PlayerSprite(player.texture);
            PlayerSprite.setPosition({ window.getSize().x + scale * (player.coord.j - 10) * 160.f + +scale * 40, scale * player.coord.i * 160.f });
            PlayerSprite.setScale({ scale / 2 * 1.f, scale / 2 * 1.f });
            window.draw(PlayerSprite);
        }




        sf::Sprite Croc_Sprite(croc.alive_text);
        if (not(croc.alive)) { Croc_Sprite.setTexture(croc.dead_text); }

        
        TreasureSprite.setPosition({ window.getSize().x + scale * (treasure.j - 10) * 160.f + 20 * scale, scale * treasure.i * 160.f + 20 * scale });
        TreasureSprite.setScale({ scale * 3 / 2 * 1.f, scale * 3 / 2 * 1.f });
        window.draw(TreasureSprite);


        Croc_Sprite.setPosition({ window.getSize().x + scale * (croc.coord.j - 10) * 160.f , scale * croc.coord.i * 160.f + scale * 60 });
        Croc_Sprite.setScale({ scale * 0.4f, scale * 0.5f });
        window.draw(Croc_Sprite);
    }

    // Вспомогательная функция: обрезает текст до последних max_lines строк
    void trim_text_lines(sf::Text& text, int max_lines)
    {
        string s = text.getString();
        int count_lines = 0;
        for (char c : s)
            if (c == '\n') count_lines++;

        if (count_lines > max_lines)
        {
            string s_new = "";
            int line_number = 0;
            for (char c : s)
            {
                if (c == '\n') line_number++;
                if (line_number >= count_lines - max_lines)
                    s_new += c;
            }
            text.setString(s_new);
        }
    }

    // Возвращает читаемое название клетки по её символу
    string tile_name(char c) const
    {
        switch (c) {
            case 'c': return "Land";
            case 'E': return "Exit";
            case 'r': return "River";
            case 's': return "Swamp";
            case 'u': return "Swamp mouth";
            case 'A': return "Arsenal";
            case 'M': return "Med unit";
            case 'w': return "Wall";
            case 'b': return "Boom wall";
            case '1': case '2': case '3': return "Pit";
            default:  return "?";
        }
    }

    // Рисует тёмный прямоугольник с заданной прозрачностью
    void draw_panel(sf::RenderWindow& window, float x, float y, float w, float h,
                    sf::Color fill = sf::Color(0, 0, 0, 160),
                    sf::Color outline = sf::Color(180, 140, 80, 200),
                    float outline_thickness = 2.f)
    {
        sf::RectangleShape panel({w, h});
        panel.setPosition({x, y});
        panel.setFillColor(fill);
        panel.setOutlineColor(outline);
        panel.setOutlineThickness(outline_thickness);
        window.draw(panel);
    }

    // Рисует панель статуса игроков внизу экрана
    void draw_hud(sf::RenderWindow& window)
    {
        if (players.empty()) return;

        const float WIN_W = static_cast<float>(window.getSize().x);
        const float WIN_H = static_cast<float>(window.getSize().y);

        // Высота HUD-панели и ширина карточки одного игрока
        const float HUD_H   = 130.f;  // увеличена для строки с клеткой
        const float HUD_Y   = WIN_H - HUD_H - 8.f;
        const float CARD_W  = std::min(300.f, (WIN_W * 0.62f) / static_cast<float>(players.size()));
        const float CARD_PAD = 8.f;

        // Общий фон HUD
        draw_panel(window, 4.f, HUD_Y, WIN_W * 0.62f - 4.f, HUD_H + 4.f,
                   sf::Color(20, 15, 10, 200), sf::Color(120, 90, 40, 220), 2.f);

        for (int idx = 0; idx < static_cast<int>(players.size()); idx++)
        {
            const player& pl = players[idx];
            float card_x = 4.f + idx * CARD_W + CARD_PAD;
            float card_y = HUD_Y + CARD_PAD;
            float card_inner_w = CARD_W - CARD_PAD * 2.f;
            float card_inner_h = HUD_H - CARD_PAD * 2.f;

            // Карточка игрока — подсвечиваем текущего
            bool is_current = (idx == current_player_idx);
            sf::Color card_fill  = is_current ? sf::Color(60, 40, 10, 210) : sf::Color(30, 20, 5, 180);
            sf::Color card_border = is_current ? sf::Color(255, 200, 50, 255) : sf::Color(100, 75, 30, 180);
            float border_w = is_current ? 3.f : 1.f;
            draw_panel(window, card_x, card_y, card_inner_w, card_inner_h, card_fill, card_border, border_w);

            float tx = card_x + 8.f;
            float ty = card_y + 6.f;

            // Имя игрока
            sf::Text name_text(font);
            name_text.setCharacterSize(15);
            string display_name = pl.name;
            if (display_name.size() > 12) display_name = display_name.substr(0, 11) + "…";
            if (is_current) display_name = "> " + display_name;
            name_text.setString(display_name);
            name_text.setFillColor(is_current ? sf::Color(255, 215, 50) : sf::Color(220, 200, 160));
            name_text.setPosition({tx, ty});
            window.draw(name_text);
            ty += 22.f;

            // HP (красные сердца: ♥ = полное, × = пустое)
            sf::Text hp_text(font);
            hp_text.setCharacterSize(14);
            string hp_str = "HP: ";
            for (int h = 0; h < 2; h++)
                hp_str += (h < pl.hp) ? "* " : "- ";
            hp_text.setString(hp_str);
            hp_text.setFillColor(pl.hp == 2 ? sf::Color(100, 220, 100) :
                                 pl.hp == 1 ? sf::Color(230, 170, 50)  :
                                              sf::Color(200, 60, 60));
            hp_text.setPosition({tx, ty});
            window.draw(hp_text);
            ty += 20.f;

            // Патроны
            sf::Text bul_text(font);
            bul_text.setCharacterSize(14);
            string bul_str = "Bullets: " + to_string(pl.bullets);
            bul_text.setString(bul_str);
            bul_text.setFillColor(pl.bullets > 0 ? sf::Color(180, 220, 255) : sf::Color(120, 120, 120));
            bul_text.setPosition({tx, ty});
            window.draw(bul_text);
            ty += 20.f;

            // Гранаты
            sf::Text gr_text(font);
            gr_text.setCharacterSize(14);
            string gr_str  = "Grenades: " + to_string(pl.granade);
            gr_text.setString(gr_str);
            gr_text.setFillColor(pl.granade > 0 ? sf::Color(255, 180, 100) : sf::Color(120, 120, 120));
            gr_text.setPosition({tx, ty});
            window.draw(gr_text);
            ty += 20.f;

            // Значок сокровища
            if (pl.with_treasure)
            {
                sf::Text tr_text(font);
                tr_text.setCharacterSize(13);
                tr_text.setString("[HAS TREASURE]");
                tr_text.setFillColor(sf::Color(255, 230, 0));
                tr_text.setPosition({tx, ty});
                window.draw(tr_text);
                ty += 18.f;
            }

            // Текущая клетка игрока
            if (TexturesIsDone && pl.turns > 0 && !player_in_exit(const_cast<player*>(&pl)))
            {
                char cell = lines[pl.coord.i][pl.coord.j];
                string cell_str = "At: " + tile_name(cell);
                // Цвет в зависимости от типа клетки
                sf::Color cell_color;
                switch (cell) {
                    case 's': case 'u': cell_color = sf::Color(100, 210, 120); break; // болото — зелёный
                    case 'r':           cell_color = sf::Color(100, 180, 255); break; // река — синий
                    case 'A':           cell_color = sf::Color(180, 220, 255); break; // арсенал — голубой
                    case 'M':           cell_color = sf::Color(100, 220, 100); break; // медпункт — зелёный
                    case 'b':           cell_color = sf::Color(255, 120, 60);  break; // стена-бум — оранжевый
                    case '1': case '2': case '3':
                                        cell_color = sf::Color(200, 100, 200); break; // яма — фиолетовый
                    case 'E':           cell_color = sf::Color(255, 255, 100); break; // выход — жёлтый
                    default:            cell_color = sf::Color(180, 165, 130); break; // земля — бежевый
                }
                sf::Text cell_text(font);
                cell_text.setCharacterSize(13);
                cell_text.setString(cell_str);
                cell_text.setFillColor(cell_color);
                cell_text.setPosition({tx, ty});
                window.draw(cell_text);
            }
        }

        // Подсказка управления справа от карточек (только во время хода)
        if (current_player_idx >= 0)
        {
            float hint_x = 4.f + players.size() * CARD_W + CARD_PAD * 2.f;
            float hint_y = HUD_Y + CARD_PAD;
            float hint_w = WIN_W * 0.62f - hint_x - 4.f;
            if (hint_w > 60.f)
            {
                draw_panel(window, hint_x, hint_y, hint_w, HUD_H - CARD_PAD * 2.f,
                           sf::Color(15, 15, 30, 180), sf::Color(80, 80, 120, 180), 1.f);
                sf::Text hint(font);
                hint.setCharacterSize(13);
                hint.setString("Arrows - move\nF - shoot\nE - grenade\nZ - undo move");
                hint.setFillColor(sf::Color(160, 160, 200));
                hint.setPosition({hint_x + 6.f, hint_y + 6.f});
                window.draw(hint);
            }
        }
    }

    // Рисует лог-панель (левая половина, над HUD)
    void draw_log(sf::RenderWindow& window, sf::Text& text)
    {
        const float WIN_W = static_cast<float>(window.getSize().x);
        const float WIN_H = static_cast<float>(window.getSize().y);
        const float HUD_H = 148.f; // HUD_H (130) + отступы
        const float LOG_W = WIN_W * 0.62f;
        const float LOG_H = WIN_H - HUD_H - 8.f;

        // Полупрозрачный тёмный фон как было изначально
        draw_panel(window, 4.f, 4.f, LOG_W - 8.f, LOG_H,
                   sf::Color(15, 10, 5, 175), sf::Color(100, 75, 30, 200), 2.f);

        text.setPosition({14.f, 10.f});
        text.setCharacterSize(16);
        text.setFillColor(sf::Color(235, 220, 185));
        window.draw(text);
    }

    void WindowClearDrawDisplay(sf::RenderWindow& window, sf::Text& text)
    {
        trim_text_lines(text, 15);

        // Фон
        sf::Sprite BackgroundSprite(background);
        BackgroundSprite.setScale({ window.getSize().x / 160.f, window.getSize().y / 160.f });

        // Кнопка "See the maze" — переезжает в правый нижний угол карты
        const float mazeX = window.getSize().x * 0.62f;
        const float mazeH = scale * 10 * 160.f;
        sf::Sprite ButtonSprite(ButtonTexture);
        ButtonSprite.setPosition({ mazeX + 10.f, mazeH + 10.f });
        sf::Text ButtonText(font);
        ButtonText.setCharacterSize(14);
        ButtonText.setString("Show/hide maze");
        ButtonText.setFillColor(sf::Color(40, 25, 10));
        ButtonText.setPosition({ mazeX + 20.f, mazeH + 18.f });

        window.clear(sf::Color(30, 20, 10));
        window.draw(BackgroundSprite);

        draw_log(window, text);
        if (TexturesIsDone and show_mase == 1) draw_lab_SFML(window);
        if (show_mase == 0) draw_unknown_lab(window);
        draw_hud(window);

        window.draw(ButtonSprite);
        window.draw(ButtonText);
        window.display();
    }

    void WindowClearDrawDisplay(sf::RenderWindow& window, sf::Text& text, sf::Sprite& spr)
    {
        trim_text_lines(text, 15);

        sf::Sprite BackgroundSprite(background);
        BackgroundSprite.setScale({ window.getSize().x / 160.f, window.getSize().y / 160.f });

        const float mazeX = window.getSize().x * 0.62f;
        const float mazeH = scale * 10 * 160.f;
        sf::Sprite ButtonSprite(ButtonTexture);
        ButtonSprite.setPosition({ mazeX + 10.f, mazeH + 10.f });
        sf::Text ButtonText(font);
        ButtonText.setCharacterSize(14);
        ButtonText.setString("Show/hide maze");
        ButtonText.setFillColor(sf::Color(40, 25, 10));
        ButtonText.setPosition({ mazeX + 20.f, mazeH + 18.f });

        window.clear(sf::Color(30, 20, 10));
        window.draw(BackgroundSprite);

        draw_log(window, text);
        if (TexturesIsDone and show_mase == 1) draw_lab_SFML(window);

        int temp = 1;
        for (auto &pl : players) temp *= pl.turns;
        if (unknown_created and show_mase == 0 and temp == 0) draw_unknown_lab(window);

        draw_hud(window);
        window.draw(ButtonSprite);
        window.draw(ButtonText);
        window.draw(spr);
        window.display();
    }

    sf::Texture** CreateTextures()
    {
        sf::Texture txt("Textures/Planks.png");
        sf::Texture** textures = new sf::Texture * [10];

        for (int i = 0; i < 10; i++)
        {
            textures[i] = new sf::Texture[10];
        }

        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {

                if (lines[i][j] == 'c' or lines[i][j] == 'E')
                {
                    (void)textures[i][j].loadFromFile("Textures/Planks.png");
                    continue;
                }
                if (lines[i][j] == 'r')
                {
                    (void)textures[i][j].loadFromFile("Textures/Water.png", false, sf::IntRect({ 0, 0 }, { 160, 160 }));
                    continue;

                }
                if (lines[i][j] == 's')
                {
                    (void)textures[i][j].loadFromFile("Textures/Moss.png");
                    continue;
                }
                if (lines[i][j] == 'A')
                {
                    (void)textures[i][j].loadFromFile("Textures/Arsenal.png");
                    continue;
                }
                if (lines[i][j] == 'M')
                {
                    (void)textures[i][j].loadFromFile("Textures/MedUnit.png");
                    continue;
                }
                if (lines[i][j] == 'w')
                {
                    (void)textures[i][j].loadFromFile("Textures/Wall.png");
                    continue;
                }
                if (lines[i][j] == 'b')
                {
                    (void)textures[i][j].loadFromFile("Textures/Boom.png");
                    continue;
                }
                if (lines[i][j] == '1')
                {
                    (void)textures[i][j].loadFromFile("Textures/Pit.png", false, sf::IntRect({ 0, 0 }, { 160, 160 }));
                    continue;
                }
                if (lines[i][j] == '2')
                {
                    (void)textures[i][j].loadFromFile("Textures/Pit.png", false, sf::IntRect({ 0, 160 }, { 160, 320 }));
                    continue;
                }
                if (lines[i][j] == '3')
                {
                    (void)textures[i][j].loadFromFile("Textures/Pit.png", false, sf::IntRect({ 0, 320 }, { 160, 480 }));
                    continue;
                }
                if (lines[i][j] == 'u')
                {
                    (void)textures[i][j].loadFromFile("Textures/Lily.png");
                    continue;
                }
                else
                {
                    (void)textures[i][j].loadFromFile("Textures/Dirt.png");
                    continue;
                }
            }
        } // 
        TexturesIsDone = 1;
        (void)TreasureTexture.loadFromFile("Textures/Treasure.png");
        return textures;
    }



    int rand_from_a_to_b(int a, int b)
    {
        seed = (seed * 73129 + 95121) % 10000;
        srand(seed);
        return rand() % (b - a + 1) + a;
    }
    void init_player(sf::RenderWindow& window, sf::Text& text, const std::filesystem::path& filename)
    {
        player pl;
        text.setString(text.getString() + "Enter the player's name\n");
        WindowClearDrawDisplay(window, text);

        (void)pl.texture.loadFromFile(filename);

        pl.name = getCommandString(window, text);
        pl.id = players.size();
        pl.with_treasure = 0;
        pl.hp = 2;
        pl.bullets = 0;
        pl.granade = 0;
        players.push_back(pl);
        return;
    }
    crocodile init_croc()
    {

        int count_of_swamp = 0;
        crocodile croc;
        (void)croc.dead_text.loadFromFile("Textures/DeadCrocodile.png");
        (void)croc.alive_text.loadFromFile("Textures/Crocodile.png");
        point* swamp_points = new point[64];
        for (int i = 1; i <= 8; i++) // ������ ������ ���� ����� ������, ����� ����� �������� ������� ���� �� ��� ��� ����� ��� ������� ��������
        {
            for (int j = 1; j <= 8; j++)
            {
                if (lines[i][j] == 's' or lines[i][j] == 'u')
                {
                    swamp_points[count_of_swamp].i = i;
                    swamp_points[count_of_swamp].j = j;
                    count_of_swamp += 1;
                }
            }
        }
        croc.coord = swamp_points[rand_from_a_to_b(0, count_of_swamp - 1)];
        croc.alive = 1;
        croc.time_to_res = 0;

        delete[] swamp_points;
        return croc;
    }
    bool player_in_exit(player* pl)
    {
        if (pl->coord.i == 9 or pl->coord.i == 0 or pl->coord.j == 9 or pl->coord.j == 0)
            return 1;
        else
            return 0;
    }
    void visual_lab(char** lines)
    {
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                cout << lines[i][j] << " ";
            }
            cout << endl;
        }

    }
    char** init_arr()
    {
        char** arr = new char* [10];

        for (int i = 0; i < 10; i++)
        {
            arr[i] = new char[10];
        }

        for (int i = 1; i < 9; i++)
        {
            for (int j = 1; j < 9; j++)
            {
                arr[i][j] = '.';
            }
        }
        for (int n = 0; n < 10; n++)
        {
            arr[0][n] = 'w';
            arr[n][0] = 'w';
            arr[9][n] = 'w';
            arr[n][9] = 'w';
        }
        return arr;
    }
    bool is_point_in_arr(point* arr, point p, int sise)
    {
        for (int k = 0; k < sise; k++)
        {
            if (arr[k].i == p.i and arr[k].j == p.j)
            {
                return 1;
            }
        }
        return 0;
    }
    bool is_there_path(char** lines)
    {
        bool trig = 0;

        if (u_mouth.j + 1 <= 8)
        {
            if (lines[u_mouth.i][u_mouth.j + 1] == '.') trig = 1;
        }
        if (u_mouth.j - 1 >= 1)
        {
            if (lines[u_mouth.i][u_mouth.j - 1] == '.') trig = 1;
        }
        if (u_mouth.i + 1 <= 8)
        {
            if (lines[u_mouth.i + 1][u_mouth.j] == '.') trig = 1;
        }
        if (u_mouth.i - 1 >= 1)
        {
            if (lines[u_mouth.i - 1][u_mouth.j] == '.') trig = 1;
        }
        return trig;
    }
    int is_along_border(char** /*lines*/, point p)
    {
        if ((p.j + 1 > 8 and p.i - 1 < 1) or (p.j + 1 > 8 and p.i + 1 > 8) or (p.j - 1 < 1 and p.i - 1 < 1) or (p.j - 1 < 1 and p.i + 1 > 8)) return 10; // ���� ����

        if (p.j + 1 > 8) return 1; // ������ �����
        if (p.i - 1 < 1) return 2; // ������
        if (p.j - 1 < 1) return 3; // �����
        if (p.i + 1 > 8) return 4; // �����
        return 0;
    }
    int count_of_swamp_around(char** lines, point p)
    {

        int count_of_s = 0;
        if (p.j + 1 <= 8)
        {
            if (lines[p.i][p.j + 1] == 's') count_of_s += 1; // ������
        }

        if (p.i + 1 <= 8)
        {
            if (lines[p.i + 1][p.j] == 's') count_of_s += 1; // �����
        }

        if (p.i - 1 >= 1)
        {
            if (lines[p.i - 1][p.j] == 's') count_of_s += 1; // ������
        }

        if (p.j - 1 >= 1)
        {
            if (lines[p.i][p.j - 1] == 's') count_of_s += 1; // �����
        }
        return count_of_s;
    }
    int count_of_r_around(char** lines, point p)
    {

        int count_of_r = 0;
        if (p.j + 1 <= 8)
        {
            if (lines[p.i][p.j + 1] == 'r') count_of_r += 1; // ������
        }

        if (p.i + 1 <= 8)
        {
            if (lines[p.i + 1][p.j] == 'r') count_of_r += 1; // �����
        }

        if (p.i - 1 >= 1)
        {
            if (lines[p.i - 1][p.j] == 'r') count_of_r += 1; // ������
        }

        if (p.j - 1 >= 1)
        {
            if (lines[p.i][p.j - 1] == 'r') count_of_r += 1; // �����
        }
        return count_of_r;
    }
    int dist_quadr(point a, point b)
    {
        return (b.i - a.i) * (b.i - a.i) + (b.j - a.j) * (b.j - a.j);
    }
    int number_of_land(char** lines, point* arr_of_p, int count_of_t)
    {
        int count_of_possible;
        //int next_point_index;

        point* possible_points = new point[4]; // ����������� ��������� ������, ���� ������ ����� ���� ����
        count_of_possible = 0;
        char temp = 'P';
        if (arr_of_p[count_of_t - 1].j + 1 <= 8)
        {
            if (lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j + 1] == '.' or
                lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j + 1] == 'c' or
                lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j + 1] == 'A' or
                lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j + 1] == 'M')
            {
                possible_points[count_of_possible].i = arr_of_p[count_of_t - 1].i;
                possible_points[count_of_possible].j = arr_of_p[count_of_t - 1].j + 1; // ��� ������
                count_of_possible += 1;
            }
        }
        if (arr_of_p[count_of_t - 1].i + 1 <= 8)
        {
            if (lines[arr_of_p[count_of_t - 1].i + 1][arr_of_p[count_of_t - 1].j] == '.' or
                lines[arr_of_p[count_of_t - 1].i + 1][arr_of_p[count_of_t - 1].j] == 'c' or
                lines[arr_of_p[count_of_t - 1].i + 1][arr_of_p[count_of_t - 1].j] == 'A' or
                lines[arr_of_p[count_of_t - 1].i + 1][arr_of_p[count_of_t - 1].j] == 'M')
            {
                possible_points[count_of_possible].i = arr_of_p[count_of_t - 1].i + 1; // ��� ����
                possible_points[count_of_possible].j = arr_of_p[count_of_t - 1].j;
                count_of_possible += 1;
            }
        }
        if (arr_of_p[count_of_t - 1].j - 1 >= 1)
        {
            if (lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j - 1] == '.' or
                lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j - 1] == 'c' or
                lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j - 1] == 'A' or
                lines[arr_of_p[count_of_t - 1].i][arr_of_p[count_of_t - 1].j - 1] == 'M') // ��� �����
            {
                possible_points[count_of_possible].i = arr_of_p[count_of_t - 1].i;
                possible_points[count_of_possible].j = arr_of_p[count_of_t - 1].j - 1;
                count_of_possible += 1;

            }
        }
        if (arr_of_p[count_of_t - 1].i - 1 >= 1)
        {
            if (lines[arr_of_p[count_of_t - 1].i - 1][arr_of_p[count_of_t - 1].j] == '.' or
                lines[arr_of_p[count_of_t - 1].i - 1][arr_of_p[count_of_t - 1].j] == 'c' or
                lines[arr_of_p[count_of_t - 1].i - 1][arr_of_p[count_of_t - 1].j] == 'A' or
                lines[arr_of_p[count_of_t - 1].i - 1][arr_of_p[count_of_t - 1].j] == 'M')
            {
                possible_points[count_of_possible].i = arr_of_p[count_of_t - 1].i - 1;
                possible_points[count_of_possible].j = arr_of_p[count_of_t - 1].j; // ��� �����
                count_of_possible += 1;

            }
        }
        if (count_of_possible == 0)
        {
            delete[] possible_points;
            return 0;
        }
        for (int next_point_index = 0; next_point_index < count_of_possible; next_point_index++)
        {
            temp = lines[possible_points[next_point_index].i][possible_points[next_point_index].j];
            lines[possible_points[next_point_index].i][possible_points[next_point_index].j] = 't';
            //visual_lab(lines); //����� ������ ��� ����������� ����
            if (not is_point_in_arr(arr_of_land_points, possible_points[next_point_index], 64))
            {
                count_of_land += 1;
                arr_of_land_points[count_of_land - 1] = possible_points[next_point_index];
            }
            arr_of_p[count_of_t] = possible_points[next_point_index];
            (void)number_of_land(lines, arr_of_p, count_of_t + 1);
            lines[possible_points[next_point_index].i][possible_points[next_point_index].j] = temp;
        }
        delete[] possible_points;
        return 0;

    }
    int number_of_land_complitely(char** lines)
    {
        int maxx = 0;
        for (int i = 1; i < 9; i++)
        {
            for (int j = 1; j < 9; j++)
            {
                if (lines[i][j] == '.' or lines[i][j] == 'c' or lines[i][j] == 'A' or lines[i][j] == 'M')
                {
                    point* arr_of_p = new point[64];
                    int count_of_t = 1;
                    point p;
                    p.i = i;
                    p.j = j;
                    arr_of_p[0] = p;
                    count_of_land = 0;
                    for (int k = 0; k < 64; k++)
                    {
                        arr_of_land_points[k].i = -1;
                        arr_of_land_points[k].j = -1;
                    }
                    (void)number_of_land(lines, arr_of_p, count_of_t);
                    delete[] arr_of_p;
                    if (count_of_land > maxx) maxx = count_of_land;
                }
            }
        }
        return maxx;
    }
    bool create_river(char** lines, point* river, int count_of_river) // �������� ������� �� ����� ���������� ������ � �� ����������������
    {

        if (count_of_river == number_of_river) // ������� ����, ��� ���� ��������� � ��������� �� ������� ��������� (�����������)
        {
            // if(number_of_land_complitely(lines) < 20) - ��� ���������� ����� ��������� ���������� ������� � ������ (7�� � ����� 5 ��������)
            if (is_along_border(lines, river[number_of_river - 1]) != 0)
            {
                count_of_path += 1;
                paths[count_of_path - 1].river = new point[number_of_river];
                //visual_lab(lines);
                paths[count_of_path - 1].count_of_r_around_summ = 0;
                paths[count_of_path - 1].count_river_along_bord = 0;
                paths[count_of_path - 1].numb_toched_walls = 0;
                paths[count_of_path - 1].toched_walls = new int[20];
                for (int i = 0; i < 5; i++) paths[count_of_path - 1].toched_walls[i] = 0;

                for (int k = 0; k < number_of_river; k++)
                {
                    paths[count_of_path - 1].river[k] = river[k]; // ������ ������ ���� � ������ ���� ���������
                    paths[count_of_path - 1].count_of_r_around_summ += count_of_r_around(lines, paths[count_of_path - 1].river[k]);
                    if (is_along_border(lines, river[k]) != 0)
                    {
                        paths[count_of_path - 1].count_river_along_bord += 1;
                        paths[count_of_path - 1].toched_walls[is_along_border(lines, river[k])] = 1;
                    }
                }
                for (int i = 0; i < 5; i++)
                {
                    paths[count_of_path - 1].numb_toched_walls += paths[count_of_path - 1].toched_walls[i];
                }


                tries += 1;
                delete[] paths[count_of_path - 1].toched_walls;

            }

            return 0;

        }
        int count_of_possible;
        point* possible_points = new point[3]; // ����������� ��������� ������, ���� ������ ����� ���� ����
        count_of_possible = 0;
        if (river[count_of_river - 1].j + 1 <= 8)
        {
            if (lines[river[count_of_river - 1].i][river[count_of_river - 1].j + 1] == '.')
            {
                possible_points[count_of_possible].i = river[count_of_river - 1].i;
                possible_points[count_of_possible].j = river[count_of_river - 1].j + 1; // ��� ������
                count_of_possible += 1;
            }
        }

        if (river[count_of_river - 1].i + 1 <= 8)
        {
            if (lines[river[count_of_river - 1].i + 1][river[count_of_river - 1].j] == '.')
            {
                possible_points[count_of_possible].i = river[count_of_river - 1].i + 1; // ��� ����
                possible_points[count_of_possible].j = river[count_of_river - 1].j;
                count_of_possible += 1;
            }
        }
        if (river[count_of_river - 1].j - 1 >= 1)
        {
            if (lines[river[count_of_river - 1].i][river[count_of_river - 1].j - 1] == '.') // ��� �����
            {
                possible_points[count_of_possible].i = river[count_of_river - 1].i;
                possible_points[count_of_possible].j = river[count_of_river - 1].j - 1;
                count_of_possible += 1;
            }
        }
        if (river[count_of_river - 1].i - 1 >= 1)
        {
            if (lines[river[count_of_river - 1].i - 1][river[count_of_river - 1].j] == '.')
            {
                possible_points[count_of_possible].i = river[count_of_river - 1].i - 1;
                possible_points[count_of_possible].j = river[count_of_river - 1].j; // ��� �����
                count_of_possible += 1;
            }
        }
        if (count_of_possible == 0)
        {
            delete[] possible_points;
            return 0;
        }
        for (int next_river_point_index = 0; next_river_point_index < count_of_possible; next_river_point_index++)
        {
            lines[possible_points[next_river_point_index].i][possible_points[next_river_point_index].j] = 'r';
            // visual_lab(lines); ����� ������ ��� �������� ����
            river[count_of_river] = possible_points[next_river_point_index];
            (void)create_river(lines, river, count_of_river + 1);
            lines[possible_points[next_river_point_index].i][possible_points[next_river_point_index].j] = '.';
        }
        delete[] possible_points;
        return 0;

    }
    bool create_bum(char** lines)
    {
        point* possible = new point[64];
        int count_poss = 0;
        for (int i = 1; i < 9; i++)
        {
            for (int j = 1; j < 9; j++)
            {
                if (lines[i][j] == '.')
                {
                    possible[count_poss].i = i;
                    possible[count_poss].j = j;
                    count_poss += 1;
                }
            }
        }
        //point bum3;
        int repeat = 0;
        do
        {
            if (repeat != 0)
            {
                lines[bum1.i][bum1.j] = '.';
                lines[bum2.i][bum2.j] = '.';
                lines[bum3.i][bum3.j] = '.';
            }
            bum1 = possible[rand_from_a_to_b(0, count_poss - 1)];
            bum2 = possible[rand_from_a_to_b(0, count_poss - 1)];
            bum3 = possible[rand_from_a_to_b(0,count_poss-1)];
            while (bum2.i == bum1.i and bum2.j == bum1.j) bum2 = possible[rand_from_a_to_b(0, count_poss - 1)]; // ������� ����� ���, ����� ��� ���� �� ������ � ����
            while((bum3.i == bum1.i and bum3.j == bum1.j) or (bum3.i == bum2.i and bum3.j == bum2.j)) bum3 = possible[rand_from_a_to_b(0,count_poss-1)]; // ����������
            lines[bum1.i][bum1.j] = 'b';
            lines[bum2.i][bum2.j] = 'b';
            lines[bum3.i][bum3.j] = 'b';
            repeat += 1;
        } while (false);

        delete[] possible;
        return 1;
    }
    void create_pits(char** lines)
    {
        do
        {
            pit1.i = rand_from_a_to_b(1, 8);
            pit1.j = rand_from_a_to_b(1, 8);

            pit2.i = rand_from_a_to_b(1, 8);
            pit2.j = rand_from_a_to_b(1, 8);

            pit3.i = rand_from_a_to_b(1, 8);
            pit3.j = rand_from_a_to_b(1, 8);
            //
        } while (not (lines[pit1.i][pit1.j] == '.' and lines[pit2.i][pit2.j] == '.' and lines[pit3.i][pit3.j] == '.' and
            dist_quadr(pit1, pit2) >= quadr_dist_between_pits and dist_quadr(pit1, pit3) >= quadr_dist_between_pits and dist_quadr(pit2, pit3) >= quadr_dist_between_pits and
            dist_quadr(pit1, u_mouth) > 1 and dist_quadr(u_mouth, pit3) > 1 and dist_quadr(pit2, u_mouth) > 1 and
            dist_quadr(pit1, medbat) > 1 and dist_quadr(medbat, pit3) > 1 and dist_quadr(pit2, medbat) > 1));// and
        //number_of_land_complitely(lines) < 100
        lines[pit1.i][pit1.j] = '1';
        lines[pit2.i][pit2.j] = '2';
        lines[pit3.i][pit3.j] = '3';

    }
    bool create_river_complitely(char** lines)
    {
        river[0] = u_mouth;
        count_of_path = 0;
        sucsess_river = create_river(lines, river, 1);
        if (count_of_path == 0) return 0;
        long minn = 10000000;
        min_path_index = 0;

        /*for (int k = 0; k < count_of_path; k++)
        {
            for (int j = 0; j < number_of_river; j++)
            {
                paths[k].Count_3r_around += count_of_r_around(lines, paths[k].river[j]);
                if( ) paths[k].Count_4r_around += count_of_r_around(lines, paths[k].river[j]);
            }
        }*/
        

        for (int k = 0; k < count_of_path; k++)
        {
            if (k_r_around * paths[k].count_of_r_around_summ + k_along_bord * paths[k].count_river_along_bord + k_touch_walls * paths[k].numb_toched_walls < minn)
            {
                minn = k_r_around * paths[k].count_of_r_around_summ + k_along_bord * paths[k].count_river_along_bord + k_touch_walls * paths[k].numb_toched_walls;
                min_path_index = k;
            }
        }

        for (int k = 0; k < number_of_river; k++)
        {
            lines[paths[min_path_index].river[k].i][paths[min_path_index].river[k].j] = 'r';
            complete_river[k].i = paths[min_path_index].river[k].i;
            complete_river[k].j = paths[min_path_index].river[k].j;
        }
        lines[u_mouth.i][u_mouth.j] = 'u';

        return 1;
    }
    void create_ars_and_med(char** lines)
    {
        do
        {
            arsenal.i = rand_from_a_to_b(1, 8);
            arsenal.j = rand_from_a_to_b(1, 8);

            medbat.i = rand_from_a_to_b(2, 7);
            medbat.j = rand_from_a_to_b(2, 7);

        } while (not (lines[arsenal.i][arsenal.j] == '.' and lines[medbat.i][medbat.j] == '.' and
            dist_quadr(medbat, arsenal) >= 9 and dist_quadr(medbat, u_mouth) >= 9 and dist_quadr(u_mouth, arsenal) >= 9));

        lines[arsenal.i][arsenal.j] = 'A';
        lines[medbat.i][medbat.j] = 'M';
    }
    void create_u_mouth_and_swamp(char** lines)
    {
        center_of_swamp.i = rand_from_a_to_b(3, 6);
        center_of_swamp.j = rand_from_a_to_b(3, 6); // �������� ���������� ����� ������
        for (int n = -1; n <= 1; n++)
        {
            for (int m = -1; m <= 1; m++)
            {
                lines[center_of_swamp.i + n][center_of_swamp.j + m] = 's';
            }
        }
        u_mouth.i = rand_from_a_to_b(center_of_swamp.i - 1, center_of_swamp.i + 1);
        u_mouth.j = rand_from_a_to_b(center_of_swamp.j - 1, center_of_swamp.j + 1);
        while ((u_mouth.i == center_of_swamp.i and u_mouth.j == center_of_swamp.j) or not (is_there_path(lines)))
        {
            u_mouth.i = rand_from_a_to_b(center_of_swamp.i - 1, center_of_swamp.i + 1);
            u_mouth.j = rand_from_a_to_b(center_of_swamp.j - 1, center_of_swamp.j + 1);
        }
        lines[u_mouth.i][u_mouth.j] = 'u';
    }
    bool create_add_swamp(char** lines)
    {
        int number_of_add_swamp = rand_from_a_to_b(add_swamp_min, add_swamp_max);
        point* possible = new point[20];
        point* added_swamp = new point[20];
        int count_of_added = 0;
        int count_of_possible_swamp = 0;
        point p;
        for (int i = 1; i < 9; i++)
        {
            for (int j = 1; j < 9; j++)
            {
                p.i = i;
                p.j = j;
                if (lines[i][j] == '.' and count_of_swamp_around(lines, p) >= 1)
                {
                    count_of_possible_swamp += 1;
                    possible[count_of_possible_swamp - 1].i = p.i;
                    possible[count_of_possible_swamp - 1].j = p.j;
                }
            }
        }
        if (count_of_possible_swamp == 0) return 1;
        if (number_of_add_swamp + 1 >= count_of_possible_swamp) number_of_add_swamp = count_of_possible_swamp - 1;
        for (int k = 0; k < number_of_add_swamp; k++)
        {
            point s = possible[rand_from_a_to_b(0, count_of_possible_swamp - 1)];
            if (not is_point_in_arr(added_swamp, s, count_of_added)) // ������ ���
            { //
                count_of_added += 1; //
                added_swamp[count_of_added - 1].i = s.i; //
                added_swamp[count_of_added - 1].j = s.j; //
                lines[s.i][s.j] = 's'; //
            }  //
        }

        delete[] added_swamp;
        delete[] possible;
        return 1;
    }
    void create_land(char** lines)
    {
        for (int i = 1; i < 9; i++)
        {
            for (int j = 1; j < 9; j++)
            {
                if (lines[i][j] == '.') lines[i][j] = 'c';
            }
        }
    }
    /*point create_treasure_anywhere(char** lines)
    {
        point treas;
        int count_of_possible = 0;

        treas.i = rand_from_a_to_b(1, 8);
        treas.j = rand_from_a_to_b(1, 8);

        return treas;
    } */
    point create_treasure_earth_bum(char** lines)
    {
        point treas;
        int count_of_possible = 0;
        point* possible = new point[10000];
        for (int i = 1; i < 9; i++)
        {
            for (int j = 1; j < 9; j++)
            {
                if (lines[i][j] == 'c')
                {
                    count_of_possible += 1;
                    possible[count_of_possible - 1].i = i;
                    possible[count_of_possible - 1].j = j;
                }
                if (lines[i][j] == 'b')
                {
                    for (int n = 0; n < Chance_For_Treasure_In_Boom; n++) // ��������� ����� ������ ��������� � ����
                    {
                        count_of_possible += 1;
                        possible[count_of_possible - 1].i = i;
                        possible[count_of_possible - 1].j = j;
                        cout << count_of_possible << endl;
                    }
                }

            }
        }
        int index = rand_from_a_to_b(0, count_of_possible - 1);
        treas = possible[index];
        delete[] possible;
        return treas;
    }

    string death(player* player)
    {
        string ans = "";
        (*player).hp = 2;

        (*player).granade = 0;
        (*player).bullets = 0;
        if ((*player).with_treasure == 1)
        {
            treasure_picked = 0;
            ans += (*player).name + " dropped the treasure\n";
            (*player).with_treasure = 0;
            /*if (lines[(*player).coord.i][(*player).coord.j] == 's' or lines[(*player).coord.i][(*player).coord.j] == 'u')
            {
                create_treasure(lines);
                cout << "�������� ���� ���� ����� � " << (*player).name << " � ������ ��� � ������ �����" << endl;
            } */ // ���� ����� ����� ��������� ������������ ��� ��������� � ������
        }
        (*player).coord = medbat;
        ans += (*player).name + " died and appeared in the medical unit.\n";
        return ans;
    }
    void create_exit(char** lines)
    {
        int y_left, y_right, x_up, x_down;
        y_left = rand_from_a_to_b(1, 8);
        y_right = rand_from_a_to_b(1, 8);
        x_up = rand_from_a_to_b(1, 8);
        x_down = rand_from_a_to_b(1, 8);
        lines[y_left][0] = 'E';
        lines[y_right][9] = 'E';
        lines[0][x_up] = 'E';
        lines[9][x_down] = 'E';

    }
    char** create_labirint()
    {
        char** lines = init_arr();
        tries = 0;
        int NumberOfLand_i = 100;
        do
        {
            if (tries != 0)
            {
                for (int i = 0; i < 10; i++)
                {
                    delete[] lines[i];
                }
                delete[] lines;
                for (int k = 0; k < count_of_path; k++)
                {
                    delete[] paths[k].river;
                }
            }
            lines = init_arr();
            create_u_mouth_and_swamp(lines); // ������� ������ � �������� ���������� � ��� �����

            create_ars_and_med(lines); // ������ ����� �������� ������� � ��������� �� ����������^2 �� ����� 17
            create_pits(lines); //  ������ ����� ������� ��� �� ������ �� ��������
            sucsess_river = create_river_complitely(lines); // ������ ������� ���� - ������� 22 ���������������� ������
            sucsess_bum = create_bum(lines);
            sucsess_add_swamp = create_add_swamp(lines);
            NumberOfLand_i = number_of_land_complitely(lines);
            unknown = CreateUnknown();

        } while (not(sucsess_river == 1 and NumberOfLand_i <= 9));
        /*cout << "count_of_tries = " << count << endl;

        cout << "number_of_land = " << number_of_land_complitely(lines) << endl;
        cout << "min_path_index = " << min_path_index << endl;

        cout << "min_number = " << k_r_around * paths[min_path_index].count_of_r_around_summ << " + " <<
            k_along_bord * paths[min_path_index].count_river_along_bord << " + " <<
            k_touch_walls * paths[min_path_index].numb_toched_walls << endl;

        cout << "paths[min_path_index].river[number_of_river] i and j = " << paths[min_path_index].river[number_of_river - 1].i << " " << paths[min_path_index].river[number_of_river - 1].j << endl;
        cout << "count_of_paths = " << count_of_path << endl; */


        create_land(lines);
        create_exit(lines);
        return lines;
    }

    string shoot(vector<player>& victims, player* hunter, string cmd, crocodile* croc)
    {
        string ans = "";
        if ((*hunter).bullets == 0)
        {
            ans += "You pull the trigger and nothing happens. You've wasted a turn\n";
            return ans;
        }
        (*hunter).bullets -= 1;
        if ((*hunter).bullets == 2) ans += "You have 2 bullets left.\n";
        if ((*hunter).bullets == 1) ans += "You have 1 bullet left.\n";
        if ((*hunter).bullets == 0) ans += "You don't have any bullets left.\n";

        for (auto &victim : victims)
        {
            if (cmd == "Down") // �������� ��������
            {
                for (int k = hunter->coord.i; k <= 9; k++)
                {
                    if (k == victim.coord.i and hunter->coord.j == victim.coord.j and victim.id != hunter->id) // ���� ����� � ������
                        // �.�. ���� ����� � �������� �� ����� ������, ������ ������� ���� �����
                    {
                        victim.hp -= 1;
                        ans += "Hit " + victim.name + "\n";
                        if (victim.hp == 0) ans += death(&victim);

                        if (victim.hp == 1 and victim.with_treasure == 1)
                        {
                            victim.with_treasure = 0;
                            treasure_picked = 0;
                            ans += victim.name + " dropped the treasure\n";
                        }
                        
                    }
                    if (k == croc->coord.i and hunter->coord.j == croc->coord.j and croc->alive) // ���� ����� � ���������
                    {
                        croc->alive = 0;
                        croc->time_to_res = 3;
                        ans += "Killed a crocodile\n";
                        ans += "There are 3 moves left before the new crocodile appears.\n";
                        
                    }
                }
            }
            else if (cmd == "Up")
            {
                for (int k = hunter->coord.i; k >= 0; k--)
                {
                    if (k == victim.coord.i and hunter->coord.j == victim.coord.j and victim.id != hunter->id)
                    {
                        victim.hp -= 1;
                        ans += "Hit " + victim.name + "\n";
                        if (victim.hp == 0) ans += death(&victim);
                        if (victim.hp == 1 and victim.with_treasure == 1)
                        {
                            victim.with_treasure = 0;
                            treasure_picked = 0;
                            ans += victim.name + " dropped the treasure\n";
                        }
                        
                    }
                    if (k == croc->coord.i and hunter->coord.j == croc->coord.j and croc->alive) // ���� ����� � ���������
                    {
                        croc->alive = 0;
                        croc->time_to_res = 3;
                        ans += "Killed a crocodile\n";
                        ans += "There are 3 moves left before the new crocodile appears.\n";
                        
                    }
                }
            }
            else if (cmd == "Right")
            {
                for (int k = hunter->coord.j; k <= 9; k++)
                {
                    if (k == victim.coord.j and hunter->coord.i == victim.coord.i and victim.id != hunter->id)
                    {
                        victim.hp -= 1;
                        ans += "Hit " + victim.name + "\n";
                        if (victim.hp == 0) ans += death(&victim);
                        if (victim.hp == 1 and victim.with_treasure == 1)
                        {
                            victim.with_treasure = 0;
                            treasure_picked = 0;
                            ans += victim.name + " dropped the treasure\n";
                        }
                        
                    }
                    if (k == croc->coord.j and hunter->coord.i == croc->coord.i and croc->alive) // ���� ����� � ���������
                    {
                        croc->alive = 0;
                        croc->time_to_res = 3;
                        ans += "Killed a crocodile\n";
                        ans += "There are 3 moves left before the new crocodile appears.\n";
                        
                    }
                }
            }
            else if (cmd == "Left")
            {
                for (int k = hunter->coord.j; k >= 0; k--)
                {
                    if (k == victim.coord.j and hunter->coord.i == victim.coord.i and victim.id != hunter->id)
                    {
                        victim.hp -= 1;
                        ans += "Hit " + victim.name + "\n";
                        if (victim.hp == 0) ans += death(&victim);

                        if (victim.hp == 1 and victim.with_treasure == 1)
                        {
                            victim.with_treasure = 0;
                            treasure_picked = 0;
                            ans += victim.name + " dropped the treasure\n";
                        }
                        
                    }
                    if (k == croc->coord.j and hunter->coord.i == croc->coord.i and croc->alive) // ���� ����� � ���������
                    {
                        croc->alive = 0;
                        croc->time_to_res = 3;
                        ans += "Killed a crocodile\n";
                        ans += "There are 3 moves left before the new crocodile appears.\n";
                        
                    }
                }
            }
            else ans += "You missed\n";
        }

        return ans;
    }

    string explode(player* player, string cmd)
    {
        string ans = "";
        int j = (*player).coord.j;
        int i = (*player).coord.i;
        if ((*player).granade == 0)
        {
            ans += "You put your hand in your pocket, take out a grenade-shaped stone and throw it. Nothing happened. You've wasted a turn\n";
            return ans;
        }
        (*player).granade -= 1;
        if (cmd == "Up")
        {
            if (i - 1 < 1)
            {
                ans += "The wall didn't explode\n";
                ans += "You have " + to_string((*player).granade) + "grenades left\n";
                return ans;
            }
            else
            {
                if (lines[i - 1][j] == 'b')
                {
                    ans += "The wall exploded\n";
                    ans += "You have " + to_string((*player).granade) + "grenades left\n";
                    lines[i - 1][j] = 'c';
                    (void)textures[i - 1][j].loadFromFile("Textures/Breaked_Boom.png");
                    return ans;
                }
            }
        }
        if (cmd == "Down")
        {
            if (i + 1 > 8)
            {
                ans += "The wall didn't explode\n";
                ans += "You have " + to_string((*player).granade) + "grenades left\n";
                return ans;
            }
            else
            {
                if (lines[i + 1][j] == 'b')
                {
                    ans += "The wall exploded\n";
                    ans += "You have " + to_string((*player).granade) + "grenades left\n";
                    lines[i + 1][j] = 'c';
                    (void)textures[i + 1][j].loadFromFile("Textures/Breaked_Boom.png");
                    return ans;
                }
            }
        }
        if (cmd == "Left")
        {
            if (j - 1 < 1)
            {
                ans += "The wall didn't explode\n";
                ans += "You have " + to_string((*player).granade) + "grenades left\n";
                return ans;
            }
            else
            {
                if (lines[i][j - 1] == 'b')
                {
                    ans += "The wall exploded\n";
                    ans += "You have " + to_string((*player).granade) + "grenades left\n";
                    lines[i][j - 1] = 'c';
                    (void)textures[i][j - 1].loadFromFile("Textures/Breaked_Boom.png");
                    return ans;
                }
            }
        }
        if (cmd == "Right")
        {
            if (j + 1 > 8)
            {
                ans += "The wall didn't explode\n";
                ans += "You have " + to_string((*player).granade) + "grenades left\n";
                return ans;
            }
            else
            {
                if (lines[i][j + 1] == 'b')
                {
                    ans += "The wall exploded\n";
                    ans += "You have " + to_string((*player).granade) + "grenades left\n";
                    lines[i][j + 1] = 'c';
                    (void)textures[i][j + 1].loadFromFile("Textures/Breaked_Boom.png");
                    return ans;
                }
            }
        }
        ans += "There was no boom or wall, what did you do? Grenades don't blow up the crocodile or the players.\n";
        ans += "You have " + to_string((*player).granade) + "grenades left\n";
        return ans;
    }

    void getCommandToDo(sf::RenderWindow& window, sf::Text& text)
    {
        while (command_changed == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));

            Button_Pressed(window, text);
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) { window.close(); return; }
                if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
                {
                    if (kp->scancode == sf::Keyboard::Scan::Right)  { command = "Right";   command_changed = 1; }
                    if (kp->scancode == sf::Keyboard::Scan::Left)   { command = "Left";    command_changed = 1; }
                    if (kp->scancode == sf::Keyboard::Scan::Down)   { command = "Down";    command_changed = 1; }
                    if (kp->scancode == sf::Keyboard::Scan::Up)     { command = "Up";      command_changed = 1; }
                    if (kp->scancode == sf::Keyboard::Scan::F)      { command = "Shoot";   command_changed = 1; }
                    if (kp->scancode == sf::Keyboard::Scan::E)      { command = "Explode"; command_changed = 1; }
                    if (kp->scancode == sf::Keyboard::Scan::Z)
                    {
                        // Убираем снапшот текущего хода (он только что был добавлен),
                        // потом восстанавливаем предыдущий
                        if (history.size() >= 2)
                        {
                            history.pop_back(); // убрать снапшот "до текущего хода"
                            restore_snapshot(text);
                        }
                        else if (history.size() == 1)
                        {
                            restore_snapshot(text);
                        }
                        WindowClearDrawDisplay(window, text);
                        // Сигнализируем основному циклу что ход был отменён —
                        // команда "Undo" вернётся в step(), который просто выйдет
                        command = "Undo";
                        command_changed = 1;
                    }
                }
            }
        }
        if (command != "Undo")
        {
            text.setString(text.getString() + command + '\n');
            WindowClearDrawDisplay(window, text);
        }
    }
    string getCommandString(sf::RenderWindow& window, sf::Text& text)
    {
        bool trig = 1;
        string ans = "";
        while (trig == 1)
        {
            // Небольшая задержка чтобы не грузить CPU на 100% и не дублировать символы
            std::this_thread::sleep_for(std::chrono::milliseconds(16));

            Button_Pressed(window, text);
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) { window.close(); return ans; }
                // TextEntered — единственный правильный способ получить символ.
                // Он срабатывает ровно один раз на каждое нажатие (с учётом OS-repeat),
                // поэтому дублирования не будет.
                if (const auto* textEntered = event->getIf<sf::Event::TextEntered>())
                {
                    uint32_t unicode = textEntered->unicode;
                    if (unicode == 8) // Backspace
                    {
                        if (!ans.empty())
                        {
                            ans.pop_back();
                            string temp = "";
                            temp += text.getString();
                            if (!temp.empty()) temp.pop_back();
                            text.setString(temp);
                            WindowClearDrawDisplay(window, text);
                        }
                    }
                    else if (unicode == 13) // Enter
                    {
                        trig = 0;
                        text.setString(text.getString() + '\n');
                        WindowClearDrawDisplay(window, text);
                    }
                    else if (unicode < 128) // Обычный ASCII-символ
                    {
                        char add = static_cast<char>(unicode);
                        ans += add;
                        text.setString(text.getString() + add);
                        WindowClearDrawDisplay(window, text);
                    }
                }
            }
        }
        cout << "Entered some text\n";
        return ans;
    }
    string getDirection(sf::RenderWindow& window, sf::Text& text)
    {
        string ans = "";
        while (ans == "")
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));

            Button_Pressed(window, text);
            while (const std::optional event = window.pollEvent())
            {
                if (event->is<sf::Event::Closed>()) { window.close(); return ans; }
                if (const auto* kp = event->getIf<sf::Event::KeyPressed>())
                {
                    if (kp->scancode == sf::Keyboard::Scan::Right) ans = "Right";
                    if (kp->scancode == sf::Keyboard::Scan::Left)  ans = "Left";
                    if (kp->scancode == sf::Keyboard::Scan::Down)  ans = "Down";
                    if (kp->scancode == sf::Keyboard::Scan::Up)    ans = "Up";
                }
            }
        }

        text.setString(text.getString() + ans + '\n');
        WindowClearDrawDisplay(window, text);
        return ans;
    }

    bool step(player* pl, crocodile* croc, sf::Text& text, sf::RenderWindow& window)
    {
        command_changed = 0;


        if (pl->turns == 0) // �������
        {
            text.setString(text.getString() + "Select the drop-off field with the mouse\n");
            WindowClearDrawDisplay(window, text);
            pl->coord = choose_point(window, text);
            cout << "Called choose_point\n";


            while (lines[pl->coord.i][pl->coord.j] == 'b')
            {
                text.setString(text.getString() + "You landed in boom. Please select another field\n");
                WindowClearDrawDisplay(window, text);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                pl->coord = choose_point(window, text);
            }



            pl->coord.i += 1; // ����� � ������� ��� ����� ������ �� ������, ���� ������� ����������
            command = "Up";
            command_changed = 1;
        }

        if (player_in_exit(pl) and pl->turns != 0) // ���� ������� ��������� "� ������"
        {
            text.setString(text.getString() + pl->name + " goes back into the maze\n");
            WindowClearDrawDisplay(window, text);

            if (pl->coord.i == 9) command = "Up";
            if (pl->coord.i == 0) command = "Down";
            if (pl->coord.j == 9) command = "Left"; // ���������� ���� ���� ������� �������
            if (pl->coord.j == 0) command = "Right";
            command_changed = 1;
        }

        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j // �� ������ ���� ��������� ������������ ����� ��� �������
            and pl->with_treasure == 0 and pl->hp == 2 and not(treasure_picked) and pl->turns != 0)
        {

            text.setString(text.getString() + pl->name + " found a treasure under him. Are you taking it?\n");
            WindowClearDrawDisplay(window, text);

            action = getCommandString(window, text);
            if (action == "Yes" or action == "y" or action == "Y")
            {
                pl->with_treasure = 1;
                treasure_picked = 1;
            }
        }
        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j
            and pl->with_treasure == 0 and pl->hp != 2 and not(treasure_picked) and pl->turns != 0)
        {
            text.setString(text.getString() + pl->name + " found a treasure under himself, but he can't take it because he's not in full health\n");
            WindowClearDrawDisplay(window, text);
        }


        if (command_changed == 0) getCommandToDo(window, text);
        if (command == "Undo") return 1; // ход отменён, выходим без изменений

        if (lines[pl->coord.i][pl->coord.j] == 'M' and pl->turns != 0) // медпункт
        {
            pl->hp = 2;
            pl->TimeInSwamp = 0;
            text.setString(text.getString() + "You are absolutely healthy as you have just been in the medical unit.\n");
            WindowClearDrawDisplay(window, text);
        }
        if (lines[pl->coord.i][pl->coord.j] == 'A' and pl->turns != 0) // арсенал
        {
            pl->bullets = 3;
            pl->granade = 2;
            pl->TimeInSwamp = 0;
            text.setString(text.getString() + "You have 3 bullets and 2 grenades as you have just been in the arsenal\n");
            WindowClearDrawDisplay(window, text);
        }

        if (command_changed == 0) getCommandToDo(window, text);
        if (command == "Undo") return 1; // ход отменён


        if (command == "Up") pl->coord.i -= 1;

        if (command == "Down") pl->coord.i += 1;

        if (command == "Right") pl->coord.j += 1;

        if (command == "Left") pl->coord.j -= 1;


        if (command == "Shoot")
        {
            text.setString(text.getString() + "Where are you shooting at?\n");
            WindowClearDrawDisplay(window, text);

            action = getDirection(window, text);

            text.setString(text.getString() + shoot(players, pl, action, croc));
            if ((lines[pl->coord.i][pl->coord.j] == 's' or
                lines[pl->coord.i][pl->coord.j] == 'u')
                and croc->alive)
            {
                pl->TimeInSwamp += 1;
                text.setString(text.getString() + "The swamp roars " + to_string(pl->TimeInSwamp) + " times\n");
                WindowClearDrawDisplay(window, text);
                if (pl->TimeInSwamp == 3)
                {
                    text.setString(text.getString() + pl->name + " bitten by a crocodile, went to the medical unit\n");
                    WindowClearDrawDisplay(window, text);
                    pl->TimeInSwamp = 0;
                    death(pl);
                    return 1;
                }
            }
            else pl->TimeInSwamp = 0;
            return 1;

        }

        if (command == "Explode")
        {
            text.setString(text.getString() + "Where are you throwing the grenade? \n");
            WindowClearDrawDisplay(window, text);
            action = getDirection(window, text);


            text.setString(text.getString() + explode(pl, action) + "\n");
            WindowClearDrawDisplay(window, text);

            if ((lines[pl->coord.i][pl->coord.j] == 's' or
                lines[pl->coord.i][pl->coord.j] == 'u')
                and croc->alive)
            {
                pl->TimeInSwamp += 1;
                text.setString(text.getString() + "The swamp roars " + to_string(pl->TimeInSwamp) + " times\n");
                WindowClearDrawDisplay(window, text);
                if (pl->TimeInSwamp == 3)
                {
                    pl->TimeInSwamp = 0;
                    text.setString(text.getString() + pl->name + " bitten by a crocodile, went to the medical unit\n" + death(pl));
                    WindowClearDrawDisplay(window, text);
                    return 1;
                }
            }
            else pl->TimeInSwamp = 0;
            return 1;
        }

        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j and treasure_picked != 1 and pl->hp == 2 and
            lines[pl->coord.i][pl->coord.j] != 'b') // ���� ����� ����
        {
            text.setString(text.getString() + pl->name + " found a treasure. Are you taking it?\n");
            WindowClearDrawDisplay(window, text);
            action = getCommandString(window, text);
            if (action == "Yes" or action == "y" or action == "Y")
            {
                pl->with_treasure = 1;
                treasure_picked = 1;
            }
        }

        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j and treasure_picked != 1 and pl->hp < 2 and
            lines[pl->coord.i][pl->coord.j] != 'b')
        {
            text.setString(text.getString() + pl->name + " found the treasure, but he can't take it\n");
            WindowClearDrawDisplay(window, text);

        }


        

        if (lines[pl->coord.i][pl->coord.j] == 'r')
        {
            pl->TimeInSwamp = 0;
            for (int k = 0; k < number_of_river; k++)
            {
                if (pl->coord.i == complete_river[k].i and pl->coord.j == complete_river[k].j)
                {

                    if (k - 3 > 0)
                    {
                        pl->coord = complete_river[k - 3];
                        if (pl->with_treasure == 1)
                        {
                            treasure.i = complete_river[k - 3].i;
                            treasure.j = complete_river[k - 3].j;
                        }

                        text.setString(text.getString() + "The river carried you away\n");
                        WindowClearDrawDisplay(window, text);
                        return 1;
                        break;
                    }
                    else
                    {
                        pl->coord = u_mouth;
                        pl->TimeInSwamp += 1;
                        if (pl->with_treasure == 1)
                        {
                            treasure.i = complete_river[k - 3].i;
                            treasure.j = complete_river[k - 3].j;
                        }

                        text.setString(text.getString() + "The river brought you to the mouth\n" + "The swamp roars " + to_string(pl->TimeInSwamp) + " times\n");
                        WindowClearDrawDisplay(window, text);

                        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j and treasure_picked != 1 and pl->hp == 2 and
                            lines[pl->coord.i][pl->coord.j] != 'b') // ���� ����� ����
                        {
                            text.setString(text.getString() + pl->name + " found a treasure. Are you taking it?\n");
                            WindowClearDrawDisplay(window, text);
                            action = getCommandString(window, text);
                            if (action == "Yes" or action == "y" or action == "Y")
                            {
                                pl->with_treasure = 1;
                                treasure_picked = 1;
                            }
                        }

                        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j and treasure_picked != 1 and pl->hp < 2 and
                            lines[pl->coord.i][pl->coord.j] != 'b')
                        {
                            text.setString(text.getString() + pl->name + " found the treasure, but he can't take it\n");
                            WindowClearDrawDisplay(window, text);

                        }

                        if (pl->with_treasure)
                        {
                            treasure.i = pl->coord.i;
                            treasure.j = pl->coord.j;
                        }
                     

                        return 1;

                    }
                }
            }
        }
        if (lines[pl->coord.i][pl->coord.j] == 'c')
        {
            pl->TimeInSwamp = 0;
            text.setString(text.getString() + "Land\n");
            WindowClearDrawDisplay(window, text);

        }
        if (lines[pl->coord.i][pl->coord.j] == 'u')
        {
            text.setString(text.getString() + "You came to the mouth\n");
            WindowClearDrawDisplay(window, text);

            if (croc->alive)
            {
                pl->TimeInSwamp += 1;
                text.setString(text.getString() + "The swamp roars " + to_string(pl->TimeInSwamp) + " times\n");
                WindowClearDrawDisplay(window, text);
                if (pl->TimeInSwamp == 3)
                {
                    text.setString(text.getString() + pl->name + " bitten by a crocodile, went to the medical unit\n" + death(pl));
                    WindowClearDrawDisplay(window, text);
                    pl->TimeInSwamp = 0;

                }
            }
            else
            {
                text.setString(text.getString() + "There is no crocodile, there is no one to growl in the swamp\n");
                WindowClearDrawDisplay(window, text);
            }


        }
        if (lines[pl->coord.i][pl->coord.j] == 'M')
        {
            pl->hp = 2;
            pl->TimeInSwamp = 0;
            text.setString(text.getString() + "Medical unit\n");
            WindowClearDrawDisplay(window, text);

        }
        if (lines[pl->coord.i][pl->coord.j] == 'A')
        {
            pl->bullets = 3;
            pl->granade = 2;
            pl->TimeInSwamp = 0;
            text.setString(text.getString() + "Arsenal\n You got 3 bullets and 2 grenades\n");
            WindowClearDrawDisplay(window, text);

        }
        if (lines[pl->coord.i][pl->coord.j] == '1')
        {
            pl->TimeInSwamp = 0;
            pl->coord = pit2;
            text.setString(text.getString() + "Got into a hole\n");
            WindowClearDrawDisplay(window, text);
            if (pl->with_treasure)
            {
                treasure.i = pl->coord.i;
                treasure.j = pl->coord.j;
            }
            return 1; // ����� ������ �� ����������� ��������� � ������ ���
        }
        if (lines[pl->coord.i][pl->coord.j] == '2')
        {
            pl->TimeInSwamp = 0;
            pl->coord = pit3;
            text.setString(text.getString() + "Got into a hole\n");
            WindowClearDrawDisplay(window, text);
            if (pl->with_treasure)
            {
                treasure.i = pl->coord.i;
                treasure.j = pl->coord.j;
            }
            return 1; // ����� ������ �� ����������� ��������� � ������ ���
        }
        if (lines[pl->coord.i][pl->coord.j] == '3')
        {
            pl->TimeInSwamp = 0;
            pl->coord = pit1;
            text.setString(text.getString() + "Got into a hole\n");
            WindowClearDrawDisplay(window, text);
            if (pl->with_treasure)
            {
                treasure.i = pl->coord.i;
                treasure.j = pl->coord.j;
            }
            return 1; // ����� ������ �� ����������� ��������� � ������ ���
        }
        if (lines[pl->coord.i][pl->coord.j] == 's')
        {
            if (croc->alive)
            {
                pl->TimeInSwamp += 1;
                text.setString(text.getString() + "The swamp roars " + to_string(pl->TimeInSwamp) + " times\n");
                WindowClearDrawDisplay(window, text);
                if (pl->TimeInSwamp == 3)
                {
                    text.setString(text.getString() + pl->name + " bitten by a crocodile, went to the medical unit\n" + death(pl));
                    WindowClearDrawDisplay(window, text);
                    pl->TimeInSwamp = 0;

                }
            }
            else
            {
                text.setString(text.getString() + "There is no crocodile, there is no one to growl in the swamp\n");
                WindowClearDrawDisplay(window, text);
            }

        }
        if (lines[pl->coord.i][pl->coord.j] == 'b' or
            lines[pl->coord.i][pl->coord.j] == 'w')
        {
            text.setString(text.getString() + "Boom\n");
            WindowClearDrawDisplay(window, text);

            if (command == "Up") pl->coord.i += 1;
            if (command == "Down") pl->coord.i -= 1;
            if (command == "Right") pl->coord.j -= 1;
            if (command == "Left") pl->coord.j += 1;

            if (pl->with_treasure)
            {
                treasure.i = pl->coord.i;
                treasure.j = pl->coord.j;
            }

            if (lines[pl->coord.i][pl->coord.j] == 's' or lines[pl->coord.i][pl->coord.j] == 'u')
            {
                pl->TimeInSwamp += 1;
                text.setString(text.getString() + "The swamp roars " + to_string(pl->TimeInSwamp) + " times\n");
                WindowClearDrawDisplay(window, text);
                if (pl->TimeInSwamp == 3)
                {
                    text.setString(text.getString() + pl->name + " bitten by a crocodile, went to the medical unit\n" + death(pl));
                    WindowClearDrawDisplay(window, text);
                    pl->TimeInSwamp = 0;

                }

            }


        }
        if (lines[pl->coord.i][pl->coord.j] == 'E')
        {
            if (pl->with_treasure)
            {
                pl->TimeInSwamp = 0;
                text.setString(text.getString() + pl->name + " came out of the maze with a treasure and won!\n");
                WindowClearDrawDisplay(window, text);
            }
            else
            {
                pl->TimeInSwamp = 0;
                text.setString(text.getString() + pl->name + "'ve found a way out! You spend the next turn trying to get back in\n");
                WindowClearDrawDisplay(window, text);

            }

        }


        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j and treasure_picked != 1 and pl->hp == 2 and
            lines[pl->coord.i][pl->coord.j] != 'b') // ���� ����� ����
        {
            text.setString(text.getString() + pl->name + " found a treasure. Are you taking it?\n");
            WindowClearDrawDisplay(window, text);
            action = getCommandString(window, text);
            if (action == "Yes" or action == "y" or action == "Y")
            {
                pl->with_treasure = 1;
                treasure_picked = 1;
            }
        }

        if (pl->coord.i == treasure.i and pl->coord.j == treasure.j and treasure_picked != 1 and pl->hp < 2 and
            lines[pl->coord.i][pl->coord.j] != 'b')
        {
            text.setString(text.getString() + pl->name + " found the treasure, but he can't take it\n");
            WindowClearDrawDisplay(window, text);

        }

        if (pl->with_treasure)
        {
            treasure.i = pl->coord.i;
            treasure.j = pl->coord.j;
        }




        return 0;

    }


    Maze(sf::RenderWindow& window, sf::Text& text, sf::Text& /*EnteredText*/)
    {
        (void)font.openFromFile("Fonts/Helvetica.ttf");
        (void)background.loadFromFile("Textures/Planks.png");
        (void)ButtonTexture.loadFromFile("Textures/Button.png");

        sf::Text ButtonText(font);
        text.setString(text.getString() + "Enter number of players\n");
        WindowClearDrawDisplay(window, text);

        int NumberOfPlayers = std::stoi( getCommandString(window, text) );
        
        for (int i = 0; i < NumberOfPlayers; i++)
        {
            init_player(window, text, "Textures/Steve.png");
        }
        
        cout << "Number = " << NumberOfPlayers << endl;
        cout << " ==================\n";

        lines = create_labirint();
        textures = CreateTextures();
        treasure = create_treasure_earth_bum(lines);
        croc = init_croc();
        /*string key = "";
        // ���������� ���� �������
        do
        {
        lines = create_labirint();
        textures = CreateTextures();
        treasure = create_treasure_earth_bum(lines);
            text.setString(text.getString() + "Are you satisfied with the maze?\n");
            WindowClearDrawDisplay(window, text);
            key = getCommandString(window, text);
        } while (not(key == "Yes" or key == "yes" or key == "y" or key == "yep"));*/ // ����� ����� ���� ���������������� ��������
    }
    ~Maze()
    {
        for (int i = 0; i < 10; i++)
        {
            delete[] lines[i];
        }
        delete[] lines;
        for (int k = 0; k < count_of_path; k++)
        {
            delete[] paths[k].river;
        }

        delete[] river;
        delete[] arr_of_land_points;
        delete[] paths;


        for (int i = 0; i < 10; i++)
        {
            delete[] textures[i];
        }
        delete[] textures;

        for (int i = 0; i < 10; i++)
        {
            delete[] unknown[i];
        }
        delete[] unknown;


    }

    void turn(sf::Text& text, sf::RenderWindow& window)
    {
        command = "";

        if (not(croc.alive))
        {
            croc.time_to_res -= 1;
            if (croc.time_to_res == 2) text.setString(text.getString() + "There are 2 moves left in the swamp before the new crocodile appears\n");
            if (croc.time_to_res == 1) text.setString(text.getString() + "There is 1 turn left in the swamp before the new crocodile appears\n");
            if (croc.time_to_res == 0)
            {
                if (respawn_new_place) croc = init_croc();
                if (not(respawn_new_place)) croc.alive = 1;
                text.setString(text.getString() + "There's a new crocodile in the swamp\n");
            }
        }

        for (int pi = 0; pi < static_cast<int>(players.size()); pi++)
        {
            player& pl = players[pi];
            current_player_idx = pi;

            // Сохраняем состояние перед ходом — можно будет откатить через Z
            save_snapshot(text);

            text.setString(text.getString() + pl.name + "'s move: ");
            WindowClearDrawDisplay(window, text);
            trash = step(&pl, &croc, text, window);

            if (command == "Undo")
            {
                // Откат уже произошёл внутри step()/getCommandToDo(),
                // ход не засчитываем — выходим из turn(), основной цикл
                // вызовет turn() снова с уже восстановленным состоянием
                current_player_idx = -1;
                return;
            }

            text.setString(text.getString() + "\n");
            WindowClearDrawDisplay(window, text);
            pl.turns += 1;
        }
        current_player_idx = -1;
    }
};

