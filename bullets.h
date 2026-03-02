#ifndef bullets_h
#define bullets_h
#include <SFML/Graphics.hpp>
using namespace sf;

void init_bullet_entry(float x, float y, float vx, float vy, int ownerId);
void update_bullets(float dt, float playAreaW, float playAreaH, float playAreaTop);
void draw_bullets(RenderWindow& window);

#endif