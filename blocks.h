#ifndef blocks_h
#define blocks_h
#include <string>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

bool load_level(const string& path);
void draw_blocks(RenderWindow& window);

#endif
