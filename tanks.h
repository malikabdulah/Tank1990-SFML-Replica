#ifndef tanks_h
#define tanks_h
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

// Tank Logic Functions
void init_tank_entry(int index, int id, bool isPlayer, float x, float y, float w, float h, Color color);
void update_tanks(float dt, float playAreaW, float playAreaH, float playAreaTop);
void draw_tanks(RenderWindow& window);
int find_free_tank_index();
void spawn_enemy(int id, float x, float y, int type);
void damage_tank(int index, int damage);
bool check_block_collision(float x, float y, float w, float h);
bool check_tank_collision(int selfIndex, float x, float y, float w, float h);

#endif