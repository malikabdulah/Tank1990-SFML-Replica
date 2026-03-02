#include "tanks.h"
#include "powerups.h"
#include "game_data.h"
#include "game_constants.h"
#include "blocks.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

void init_powerups() {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        powerup_active[i] = false;
    }
    powerup_killCount = 0;
    powerup_timer = 0.0f;
}

void spawn_powerup(float x, float y, int type) {
    //Find a free slot in the powerup array
    int idx = -1;
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        if (!powerup_active[i]) {
            idx = i;
            break;
        }
    }
    // If no slots are available, exit
    if (idx == -1) return;

    //Determine a safe spawn position
    const float pw = 30.0f;
    const float ph = 30.0f;
    const int maxAttempts = 30;
    int attempts = 0;

    float spawnX = x;
    float spawnY = y;

    while (attempts < maxAttempts) {
        // Check collision against other things
        bool hitBlock = check_block_collision(spawnX, spawnY, pw, ph);
        bool hitTank = check_tank_collision(-1, spawnX, spawnY, pw, ph);

        // If the spot is free, we break the loop
        if (!hitBlock && !hitTank) {
            break;
        }
        // Otherwise, pick a random position within the FULL play area
        spawnX = static_cast<float>(rand() % static_cast<int>(PLAY_AREA_W - pw));
        spawnY = static_cast<float>(rand() % static_cast<int>(PLAY_AREA_H - ph));

        attempts++;
    }

    //Initialize the powerup at the safe coordinates
    powerup_active[idx] = true;
    powerup_x[idx] = spawnX;
    powerup_y[idx] = spawnY;
    powerup_type[idx] = type;
    powerup_spawnTime[idx] = 15.0f; // Disappear after 15 seconds
}

void apply_powerup_effect(int type, int playerIndex) {
    if (type == 0) { // Shield
        tank_invulnerable[playerIndex] = true;
        tank_invulTimer[playerIndex] = 10.0f;
    }
    else if (type == 1) { // Life
        tank_lives[playerIndex]++;
        tank_totalLives[playerIndex]++;
    }
    else if (type == 2) { // Freeze
        // Freeze all enemies
        for (int i = 0; i < MAX_TANKS; ++i) {
            if (tank_active[i] && !tank_isPlayer[i]) {
                tank_frozen[i] = true;
                tank_freezeTimer[i] = 8.0f;
            }
        }
    }
}

void update_powerups(float dt) {
    // Update timer only if we have kills pending
    if (powerup_killCount > 0) {
        powerup_timer += dt;
        if (powerup_timer > 15.0f) {
            cout << "[Powerups] Timer expired (15s), resetting kill count from " << powerup_killCount << " to 0" << endl;
            powerup_killCount = 0;
            powerup_timer = 0.0f;
        }
    }
    else {
        powerup_timer = 0.0f;
    }

    // Check spawn condition (3 kills in 15s)
    if (powerup_killCount >= 3) {
        cout << "[Powerups] Spawn condition met! Spawning powerup." << endl;
        // Spawn at a random safe location within play area
        float px = 100.0f + static_cast<float>(rand() % (int)(PLAY_AREA_W - 200));
        float py = 100.0f + static_cast<float>(rand() % (int)(PLAY_AREA_H - 200));
        spawn_powerup(px, py, rand() % 3);

        powerup_killCount = 0;
        powerup_timer = 0.0f;
    }

    // Update active powerups
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        if (!powerup_active[i]) continue;

        powerup_spawnTime[i] -= dt;
        if (powerup_spawnTime[i] <= 0) {
            powerup_active[i] = false;
            cout << "[Powerups] Powerup expired." << endl;
            continue;
        }

        // Check collision with player
        FloatRect pRect(powerup_x[i] - 10, powerup_y[i] - 10, 20, 20);

        for (int t = 0; t < MAX_TANKS; ++t) {
            if (tank_active[t] && tank_isPlayer[t]) {
                FloatRect tRect(tank_x[t], tank_y[t], tank_width[t], tank_height[t]);
                if (pRect.intersects(tRect)) {
                    apply_powerup_effect(powerup_type[i], t);
                    powerup_active[i] = false;
                    break;
                }
            }
        }
    }
}

void draw_powerups(RenderWindow& window) {
    for (int i = 0; i < MAX_POWERUPS; ++i) {
        if (!powerup_active[i]) continue;

        CircleShape shape(10.0f);
        shape.setOrigin(10.0f, 10.0f);
        shape.setPosition(powerup_x[i], powerup_y[i]);

        if (powerup_type[i] == 0) shape.setFillColor(Color::Cyan); // Shield
        else if (powerup_type[i] == 1) shape.setFillColor(Color::Red); // Life
        else if (powerup_type[i] == 2) shape.setFillColor(Color(100, 100, 255)); // Freeze

        shape.setOutlineColor(Color::White);
        shape.setOutlineThickness(2.0f);

        window.draw(shape);
    }
}