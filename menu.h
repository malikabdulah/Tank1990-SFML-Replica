#ifndef menu_h
#define menu_h
#include <SFML/Graphics.hpp>
using namespace sf;

void init_menu();
void update_menu(RenderWindow& window);
void draw_menu(RenderWindow& window);
void draw_pause_menu(RenderWindow& window);
void handle_pause_input(RenderWindow& window);

#endif