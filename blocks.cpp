#include "blocks.h"
#include "game_data.h"
#include "game_constants.h"
#include "assets.h"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
using namespace sf;

// ------------ LEVEL LOADER LOGIC ------------
// Reads a text file where the map is defined by numbers.
// Example: 1 = Brick, 2 = Stone, 0 = Empty Air
bool load_level(const string& path) {
    ifstream file(path);
    if (!file.is_open()) return false; // File missing? Abort.

    // Wipe the slate clean before loading new map
    block_count = 0;

    // Read the file line by line
    string line;
    getline(file, line); // Skip the first line (often contains metadata/headers)

    int row = 0;
    float tileSize = 52.0f; // Fixed tile size (based on screen width logic)

    // Limit map size to 20 rows to prevent memory overflow
    while (getline(file, line) && row < 20) {
        stringstream ss(line);
        int type;
        int col = 0;

        // Parse numbers from the line (columns)
        while (ss >> type && col < 30) { // Limit to 30 columns

            // If the block is NOT air (type 0), we create a block object
            if (type != 0) {
                if (block_count < MAX_BLOCKS) {
                    int i = block_count++; // Grab next available slot

                    // Calculate pixel position based on grid coordinates
                    block_x[i] = col * tileSize;
                    block_y[i] = row * tileSize;
                    block_w[i] = tileSize;
                    block_h[i] = tileSize;

                    block_type[i] = type;
                    // Type 4 might be "water" or "bush" (non-solid but visible)
                    block_solid[i] = (type != 4);

                    // --------- Hardness Logic -----------
                    // Brick (1) breaks easily. Stone (2) is invincible.
                    block_hardness[i] = (type == 2) ? 5 : 1;
                    if (type == 2) block_hardness[i] = 1000; // Effectively infinite health

                    block_useSprite[i] = false;

                    // Setup fallback shape (colored square) just in case sprites fail
                    block_shape[i].setSize(Vector2f(tileSize, tileSize));
                    block_shape[i].setPosition(block_x[i], block_y[i]);

                    // ------------ Texture Assignment -----------
                    // Assign the correct texture based on the block ID number
                    if (type == 1 && assets::tex_brick.getSize().x > 0) {
                        block_useSprite[i] = true;
                        block_sprite[i].setTexture(assets::tex_brick);
                    }
                    else if (type == 2 && assets::tex_stone.getSize().x > 0) {
                        block_useSprite[i] = true;
                        block_sprite[i].setTexture(assets::tex_stone);
                    }

                    // ----------- Visual Scaling ---------
                    if (block_useSprite[i]) {
                        // Check if texture is valid before accessing it
                        if (block_sprite[i].getTexture()) {
                            Vector2u s = block_sprite[i].getTexture()->getSize();
                            if (s.x > 0) {
                                // Scale the sprite to fit the target tile size perfectly
                                block_sprite[i].setScale(tileSize / s.x, tileSize / s.y);
                            }
                            block_sprite[i].setPosition(block_x[i], block_y[i]);
                        }
                    }
                    else {
                        // ------- Fallback Colors -----------
                        // If sprites didn't load, paint squares so the game is still playable
                        if (type == 1) block_shape[i].setFillColor(Color(150, 100, 50)); // Brown (Brick)
                        else if (type == 2) block_shape[i].setFillColor(Color(100, 100, 100)); // Grey (Stone)
                        else if (type == 4) block_shape[i].setFillColor(Color::Blue); // Water?
                        else block_shape[i].setFillColor(Color::Magenta); // Error color
                    }
                }
            }
            col++;
        }
        row++;
    }
    return true; // Level loaded successfully!
}

// ---------- RENDER LOGIC -------------------
void draw_blocks(RenderWindow& window) {
    for (int i = 0; i < block_count; ++i) {
        // Skip blocks that are destroyed (hardness <= 0) 
        // UNLESS they are type 4 (Water/Bush) or type 2 (Indestructible)
        if (!block_solid[i] && block_type[i] != 4) continue;

        if (block_hardness[i] <= 0 && block_type[i] != 2 && block_type[i] != 4) continue;

        // Draw the appropriate visual
        if (block_useSprite[i]) {
            window.draw(block_sprite[i]);
        }
        else {
            window.draw(block_shape[i]);
        }
    }
}