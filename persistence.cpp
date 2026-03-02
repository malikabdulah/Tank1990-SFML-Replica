#include "persistence.h"
#include "game_data.h"
#include "tanks.h"
#include "bullets.h"
#include "blocks.h" // for block arrays
#include "assets.h" // for re-assigning textures
#include "game_constants.h"
#include <fstream>
#include <iostream>
using namespace std;
using namespace sf;

// Unique identifier to validate save files
// This ensures we don't try to load an empty file or a random text file
const string SAVE_FILE_HEADER = "NEON_SIEGE_SAVE_DATA_V1";

// Save Game Logic
// dumps the current state of the world into a text file.
bool save_game() {
    ofstream file("data/save.txt");
    if (!file.is_open()) return false;

    // This acts as a stamp of authenticity for the load function
    file << SAVE_FILE_HEADER << "\n";

    // Save Global Stats
    file << "SCORE " << score << "\n";
    file << "LEVEL " << currentLevel << "\n";
    file << "KILLTARGET " << killTarget << "\n";
    file << "ENEMIES_DESTROYED " << enemiesDestroyed << "\n";
    file << "BASE_ALIVE " << baseAlive << "\n";
    file << "BASE_RECT " << baseRect.left << " " << baseRect.top << " " << baseRect.width << " " << baseRect.height << "\n";

    // Save Tanks
    // First, count how many are alive so the loader knows what to expect.
    int activeTanks = 0;
    for (int i = 0; i < MAX_TANKS; ++i) if (tank_active[i]) activeTanks++;

    file << "TANKS " << activeTanks << "\n";

    // Save details for each active tank
    for (int i = 0; i < MAX_TANKS; ++i) {
        if (tank_active[i]) {
            file << "TANK " << i << " " << tank_id[i] << " " << tank_isPlayer[i] << " "
                << tank_x[i] << " " << tank_y[i] << " " << tank_lives[i] << " "
                << tank_aiType[i] << " " << tank_skin[i] << "\n";
        }
    }

    // Save Bullets
    // Same logic: count them, then save their trajectory and owner.
    int activeBullets = 0;
    for (int i = 0; i < MAX_BULLETS; ++i) if (bullet_active[i]) activeBullets++;

    file << "BULLETS " << activeBullets << "\n";

    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (bullet_active[i]) {
            file << "BULLET " << i << " " << bullet_x[i] << " " << bullet_y[i] << " "
                << bullet_vx[i] << " " << bullet_vy[i] << " " << bullet_ownerId[i] << "\n";
        }
    }

    // Save Map State
    // We save every block's condition so destroyed walls stay destroyed.
    file << "BLOCKS " << block_count << "\n";
    for (int i = 0; i < block_count; ++i) {
        file << "BLOCK " << i << " " << block_type[i] << " " << block_solid[i] << " " << block_hardness[i] << "\n";
    }

    return true;
}

// Load Game Logic
// Reads the text file and reconstructs the game state.
bool load_game() {
    ifstream file("data/save.txt");
    if (!file.is_open()) return false;

    // ------------ VALIDATION CHECK --------------------
    // Try to read the first string in the file.
    // If the file is empty, 'file >> header' returns false.
    // If the file contains random text, 'header' will not match our unique ID.
    string header;
    if (!(file >> header) || header != SAVE_FILE_HEADER) {
        // Validation failed: The file is empty, corrupt, or not a valid save file.
        // Abort loading immediately to prevent game corruption.
        return false;
    }

    string label;
    // Parse the file token by token
    while (file >> label) {
        if (label == "SCORE") file >> score;
        else if (label == "LEVEL") file >> currentLevel;
        else if (label == "ENEMIES_DESTROYED") file >> enemiesDestroyed;
        else if (label == "BASE_ALIVE") file >> baseAlive;
        else if (label == "BASE_RECT") {
            float l, t, w, h; file >> l >> t >> w >> h; baseRect = FloatRect(l, t, w, h);
        }
        else if (label == "KILLTARGET") {
            file >> killTarget;
        }
        // Load Tanks
        else if (label == "TANKS") {
            int count;
            file >> count;

            // Wipe existing tanks before loading
            for (int i = 0; i < MAX_TANKS; ++i) tank_active[i] = false;

            for (int k = 0; k < count; ++k) {
                string tLabel;
                int idx, id, lives, ai, skin;
                bool isPlayer;
                float x, y;
                file >> tLabel >> idx >> id >> isPlayer >> x >> y >> lives >> ai >> skin;

                // Determine color based on type (visual fallback)
                Color c = Color::White;
                if (isPlayer) c = Color::Green;
                else if (ai == 1) c = Color::Red;
                else if (ai == 2) c = Color::Green;

                // Re-initialize the tank struct
                init_tank_entry(idx, id, isPlayer, x, y, isPlayer ? 36.0f : 32.0f, isPlayer ? 36.0f : 32.0f, c);

                // Restore specific stats that init_tank_entry might have reset
                tank_lives[idx] = lives;
                tank_aiType[idx] = ai;
                tank_skin[idx] = skin;
                tank_facingX[idx] = 0.0f;
                tank_facingY[idx] = -1.0f;

                // Re-assign Textures
                // Since pointers don't survive file saving, we must manually re-link the global assets.
                if (skin == 3 && assets::tex_tankYellow.getSize().x > 0) {
                    tank_useSprite[idx] = true;
                    tank_sprite[idx].setTexture(assets::tex_tankYellow);
                }
                else if (skin == 1 && assets::tex_tankGreen.getSize().x > 0) {
                    tank_useSprite[idx] = true;
                    tank_sprite[idx].setTexture(assets::tex_tankGreen);
                }
                else if (skin == 2 && assets::tex_tankRed.getSize().x > 0) {
                    tank_useSprite[idx] = true;
                    tank_sprite[idx].setTexture(assets::tex_tankRed);
                }
                else {
                    tank_useSprite[idx] = true;
                    tank_sprite[idx].setTexture(assets::tex_tankGrey);
                }

                // Restore Sprite Scaling and Frame Rects
                if (tank_useSprite[idx]) {
                    Vector2u s = tank_sprite[idx].getTexture()->getSize();
                    int frameW = 0, frameH = 0;
                    if (s.x > 0 && s.y > 0) {
                        int frames = (s.x / s.y) > 0 ? (int)(s.x / s.y) : 1;
                        frameW = s.x / frames;
                        frameH = s.y;
                        tank_sprite[idx].setTextureRect(IntRect(0, 0, frameW, frameH));
                        tank_sprite[idx].setScale(tank_width[idx] / (float)frameW, tank_height[idx] / (float)frameH);
                        tank_sprite[idx].setOrigin(0.f, 0.f);
                        tank_sprite[idx].setPosition(tank_x[idx], tank_y[idx]);
                    }
                }
            }
        }
        // Load Bullets
        else if (label == "BULLETS") {
            int count;
            file >> count;
            for (int i = 0; i < MAX_BULLETS; ++i) bullet_active[i] = false;

            for (int k = 0; k < count; ++k) {
                string bLabel;
                int idx, owner;
                float x, y, vx, vy;
                file >> bLabel >> idx >> x >> y >> vx >> vy >> owner;
                init_bullet_entry(x, y, vx, vy, owner);
            }
        }
        // Load Map State
        else if (label == "BLOCKS") {
            int count;
            file >> count;

            // 1. Reload the base level structure from the map file
            load_level("data/levels/level" + to_string(currentLevel) + ".txt");

            // 2. Apply the saved state (damage, destruction) on top of the loaded map
            for (int k = 0; k < count; ++k) {
                string bLabel;
                int idx, type, hardness;
                bool solid;
                file >> bLabel >> idx >> type >> solid >> hardness;

                if (idx < block_count) {
                    block_type[idx] = type;
                    block_solid[idx] = solid;
                    block_hardness[idx] = hardness;

                    // If the block was destroyed in the save, make it invisible/passable
                    if (!solid && type != 4) {
                        block_useSprite[idx] = false;
                        block_shape[idx].setFillColor(Color::Transparent);
                    }
                }
            }
        }
    }

    // Post-Load Setup
    inMenu = false; // Switch to game view

    // Safety check for base position
    if (baseRect.width <= 0.0f || baseRect.height <= 0.0f) {
        baseRect = FloatRect(PLAY_AREA_W / 2.0f - 30.0f, PLAY_AREA_H - 60.0f, 60.0f, 60.0f);
    }

    paused = true; // Give the player a moment to get ready

    // Final pass to ensure sprites are correctly positioned
    for (int i = 0; i < MAX_TANKS; ++i) {
        if (!tank_active[i]) continue;
        tank_shape[i].setPosition(tank_x[i], tank_y[i]);
        if (tank_useSprite[i] && tank_sprite[i].getTexture()) {
            Vector2u s = tank_sprite[i].getTexture()->getSize();
            if (s.x > 0 && s.y > 0) {
                int frames = (s.x / s.y) > 0 ? (int)(s.x / s.y) : 1;
                int frameW = s.x / frames;
                int frameH = s.y;
                tank_sprite[i].setTextureRect(IntRect(0, 0, frameW, frameH));
                tank_sprite[i].setScale(tank_width[i] / (float)frameW, tank_height[i] / (float)frameH);
                tank_sprite[i].setPosition(tank_x[i], tank_y[i]);
            }
        }
    }
    return true;
}

// Load High Scores
// Reads simple list of integers from file
void load_high_scores(int scores[], int max) {
    for (int i = 0; i < max; ++i) scores[i] = 0;

    ifstream file("data/highscore.txt");
    if (!file.is_open()) return;

    for (int i = 0; i < max; ++i) {     
        if (!(file >> scores[i])) break;
    }
}

// Save High Score
// Inserts new score into top 10 and saves back to file
void save_high_score(int newScore) {
    int scores[10];
    load_high_scores(scores, 10);

    // Insertion Sort logic to place new score in correct rank
    for (int i = 0; i < 10; ++i) {
        if (newScore > scores[i]) {
            // Shift lower scores down
            for (int j = 9; j > i; --j) {
                scores[j] = scores[j - 1];
            }
            scores[i] = newScore;
            break;
        }
    }

    ofstream file("data/highscore.txt");
    if (!file.is_open()) return;

    for (int i = 0; i < 10; ++i) {
        file << scores[i] << "\n";
    }
}