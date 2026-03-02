#ifndef powerups_h
#define powerups_h
#include <SFML/Graphics.hpp>
using namespace sf;

// Initialize powerup system
void init_powerups();

// Spawn a powerup at specific location
void spawn_powerup(float x, float y, int type);

// Update powerups (timers, collisions, spawning logic)
void update_powerups(float dt);

// Draw active powerups
void draw_powerups(RenderWindow& window);

#endif