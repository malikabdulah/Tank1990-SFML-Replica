#include "tanks.h"
#include "game_data.h"
#include "bullets.h"
#include "assets.h"
#include "audio.h"
#include "game_constants.h"
#include <cmath>
#include <iostream>
using namespace std;

//Check Block Collision
// Returns true if the proposed rectangle intersects any solid blocks or the base.
bool check_block_collision(float x, float y, float w, float h) {
    FloatRect tr(x, y, w, h);
    for (int i = 0; i < block_count; ++i) {
        if (!block_solid[i]) continue;
        FloatRect br(block_x[i], block_y[i], block_w[i], block_h[i]);
        if (tr.intersects(br)) return true;
    }
    // Also check collision with the base (Eagle) if it's alive
    if (baseAlive && tr.intersects(baseRect)) return true;
    return false;
}

//Check Tank Collision
// Returns true if the proposed rectangle intersects any other active tank.
bool check_tank_collision(int selfIndex, float x, float y, float w, float h) {
    FloatRect tr(x, y, w, h);
    for (int i = 0; i < MAX_TANKS; ++i) {
        if (!tank_active[i] || i == selfIndex) continue;
        FloatRect other(tank_x[i], tank_y[i], tank_width[i], tank_height[i]);
        if (tr.intersects(other)) return true;
    }
    return false;
}

//Find Free Tank Slot
// Searches the tank array for an inactive slot to reuse.
int find_free_tank_index() {
    for (int i = 0; i < MAX_TANKS; ++i) {
        if (!tank_active[i]) return i;
    }
    return -1;
}

// Initialize Tank Entity
// Sets up all properties (position, health, type, sprite) for a new tank.
void init_tank_entry(int index, int id, bool isPlayer, float x, float y, float w, float h, Color color) {
    if (index < 0 || index >= MAX_TANKS) return;

    // Core Properties
    tank_active[index] = true;
    tank_id[index] = id;
    tank_isPlayer[index] = isPlayer;
    tank_x[index] = x;
    tank_y[index] = y;
    tank_width[index] = w;
    tank_height[index] = h;
    tank_speed[index] = isPlayer ? 120.0f : 80.0f;

    // Lives & State
    tank_lives[index] = 1; // Current life in this spawn
    tank_totalLives[index] = isPlayer ? 3 : 1; // Total respawns
    tank_skin[index] = 0;
    tank_useSprite[index] = false;

    // Powerup States
    tank_invulnerable[index] = isPlayer; // Player starts with temporary shield
    tank_invulTimer[index] = isPlayer ? 2.0f : 0.0f;
    tank_frozen[index] = false;
    tank_freezeTimer[index] = 0.0f;

    // Movement & Combat
    tank_facingX[index] = 0.0f;
    tank_facingY[index] = -1.0f; // Default facing UP
    tank_moveTimer[index] = 0.0f;
    tank_moveInterval[index] = 0.0f;
    tank_shootTimer[index] = 0.0f;
    tank_shootInterval[index] = isPlayer ? 0.5f : 1.5f;

    // Visuals (Fallback Shape)
    tank_shape[index].setSize(Vector2f(w, h));
    tank_shape[index].setFillColor(color);
    tank_shape[index].setPosition(x, y);

    // Player Sprite Setup
    if (isPlayer) {
        if (assets::tex_tankYellow.getSize().x > 0) {
            tank_skin[index] = 3;
            tank_useSprite[index] = true;
            tank_sprite[index].setTexture(assets::tex_tankYellow);

            Vector2u s = assets::tex_tankYellow.getSize();
            if (s.x > 0 && s.y > 0) {
                int frames = s.x / s.y;
                if (frames < 1) frames = 1;
                int frameW = s.x / frames;
                int frameH = s.y;

                tank_sprite[index].setTextureRect(IntRect(0, 0, frameW, frameH));
                tank_sprite[index].setScale(w / (float)frameW, h / (float)frameH);
            }
        }
    }
}

// Spawn Enemy Logic
// Attempts to spawn an enemy at a valid location, preventing overlap with walls.
void spawn_enemy(int id, float x, float y, int type) {
    int idx = find_free_tank_index();
    if (idx == -1) return;

    const float tankW = 40.0f;
    const float tankH = 40.0f;

    // Fixed spawn points (Left, Center, Right) at the top of the map
    float spawnPointsX[] = {
        20.0f,
        PLAY_AREA_W / 2.0f - tankW / 2.0f,
        PLAY_AREA_W - tankW - 20.0f
    };
    float fixedSpawnY = 20.0f;

    const int maxAttempts = 50;
    int attempts = 0;
    float finalX = -1000.0f;
    float finalY = -1000.0f;
    bool foundSpot = false;

    // 1. Check if specific coordinates were requested (e.g. from level load)
    if (x > 0 && y > 0) {
        // Use a 1-pixel buffer to ensure the tank isn't spawning *inside* a wall
        if (!check_block_collision(x - 1, y - 1, tankW + 2, tankH + 2) &&
            !check_tank_collision(-1, x - 1, y - 1, tankW + 2, tankH + 2)) {
            finalX = x;
            finalY = y;
            foundSpot = true;
        }
    }

    // 2. If no spot found yet, search for one
    if (!foundSpot) {
        while (attempts < maxAttempts) {
            float testX, testY;

            // Phase A: Try fixed top spots first
            if (attempts < 10) {
                int spotIndex = rand() % 3;
                testX = spawnPointsX[spotIndex];
                testY = fixedSpawnY;
            }
            // Phase B: If fixed spots blocked, try random spots in the top half
            else {
                float maxSpawnY = (PLAY_AREA_H / 2.0f) - tankH;
                testX = static_cast<float>(rand() % static_cast<int>(PLAY_AREA_W - tankW));
                testY = static_cast<float>(rand() % static_cast<int>(maxSpawnY));
            }

            // Perform collision check with buffer
            bool hitBlock = check_block_collision(testX - 1, testY - 1, tankW + 2, tankH + 2);
            bool hitTank = check_tank_collision(-1, testX - 1, testY - 1, tankW + 2, tankH + 2);

            if (!hitBlock && !hitTank) {
                finalX = testX;
                finalY = testY;
                foundSpot = true;
                break;
            }
            attempts++;
        }
    }

    // Abort if no safe spot was found to prevent glitches
    if (!foundSpot) return;

    // Initialize the enemy tank
    Color c = Color::White;
    if (type == 1) c = Color::Red;
    else if (type == 2) c = Color::Green;

    init_tank_entry(idx, id, false, finalX, finalY, tankW, tankH, c);
    tank_aiType[idx] = type;
    tank_speed[idx] = (type == 1) ? 92.0f : 70.0f; // Green tanks are faster

    // Apply specific textures based on enemy type
    if (type == 2 && assets::tex_tankRed.getSize().x > 0) {
        tank_useSprite[idx] = true;
        tank_sprite[idx].setTexture(assets::tex_tankRed);
        tank_skin[idx] = 2; // Red
        Vector2u s = assets::tex_tankRed.getSize();
        if (s.x > 0) {
            int frames = s.x / s.y;
            if (frames < 1) frames = 1;
            int frameW = s.x / frames;
            int frameH = s.y;
            tank_sprite[idx].setTextureRect(IntRect(0, 0, frameW, frameH));
            tank_sprite[idx].setScale(tankW / (float)frameW, tankH / (float)frameH);
        }
    }
    else if (type == 1 && assets::tex_tankGreen.getSize().x > 0) {
        tank_useSprite[idx] = true;
        tank_skin[idx] = 1; // Green
        tank_sprite[idx].setTexture(assets::tex_tankGreen);
        Vector2u s = assets::tex_tankGreen.getSize();
        if (s.x > 0) {
            int frames = s.x / s.y;
            if (frames < 1) frames = 1;
            int frameW = s.x / frames;
            int frameH = s.y;
            tank_sprite[idx].setTextureRect(IntRect(0, 0, frameW, frameH));
            tank_sprite[idx].setScale(tankW / (float)frameW, tankH / (float)frameH);
        }
    }
    else {
        tank_useSprite[idx] = true;
        tank_skin[idx] = 0; // Grey (Default)
        tank_sprite[idx].setTexture(assets::tex_tankGrey);
        Vector2u s = assets::tex_tankGrey.getSize();
        if (s.x > 0) {
            int frames = s.x / s.y;
            if (frames < 1) frames = 1;
            int frameW = s.x / frames;
            int frameH = s.y;
            tank_sprite[idx].setTextureRect(IntRect(0, 0, frameW, frameH));
            tank_sprite[idx].setScale(tankW / (float)frameW, tankH / (float)frameH);
        }
    }
}

// Update Logic for All Tanks
void update_tanks(float dt, float playAreaW, float playAreaH, float playAreaTop) {
    for (int i = 0; i < MAX_TANKS; ++i) {
        if (!tank_active[i]) continue;

        // --- Status Effects ---

        // Invulnerability Timer
        if (tank_invulnerable[i]) {
            tank_invulTimer[i] -= dt;
            if (tank_invulTimer[i] <= 0) tank_invulnerable[i] = false;
        }

        // Freeze Timer
        if (tank_frozen[i]) {
            tank_freezeTimer[i] -= dt;
            if (tank_freezeTimer[i] <= 0) {
                tank_frozen[i] = false;
            }
            else {
                // Skip movement updates but refresh visual position
                tank_shape[i].setPosition(tank_x[i], tank_y[i]);
                if (tank_useSprite[i]) tank_sprite[i].setPosition(tank_x[i], tank_y[i]);
                continue;
            }
        }

        float dx = 0.0f, dy = 0.0f;

        // --- Player Input ---
        if (tank_isPlayer[i]) {
            if (Keyboard::isKeyPressed(Keyboard::A)) dx = -1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::D)) dx = 1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::W)) dy = -1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::S)) dy = 1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::Left)) dx = -1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::Right)) dx = 1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::Up)) dy = -1.25f;
            else if (Keyboard::isKeyPressed(Keyboard::Down)) dy = 1.25f;
        }
        // --- Enemy AI ---
        else {
            tank_moveTimer[i] -= dt;

            // Decision Logic: When timer expires, pick a new direction
            if (tank_moveTimer[i] <= 0) {
                // Set next move duration (2.0 to 4.0 seconds)
                tank_moveTimer[i] = (float)(rand() % 100) / 50.0f + 2.0f;

                tank_facingX[i] = 0;
                tank_facingY[i] = 0;

                // 50% chance for Smart AI vs Random Movement
                int moveType = rand() % 2;

                // Smart AI: Green/Red tanks try to move towards the base
                if ((tank_aiType[i] == 1 || tank_aiType[i] == 2) && moveType == 1) {
                    float tankCenterX = tank_x[i] + tank_width[i] / 2.0f;
                    float tankCenterY = tank_y[i] + tank_height[i] / 2.0f;

                    float targetX = baseRect.left + baseRect.width / 2.0f;
                    float targetY = baseRect.top + baseRect.height / 2.0f;

                    // Randomly choose horizontal or vertical alignment
                    int axis = rand() % 2;

                    if (axis == 0) {
                        // Move horizontally towards target
                        if (targetX > tankCenterX) tank_facingX[i] = 1.35f;
                        else tank_facingX[i] = -1.35f;
                    }
                    else {
                        // Move vertically towards target
                        if (targetY > tankCenterY) tank_facingY[i] = 1.35f;
                        else tank_facingY[i] = -1.35f;
                    }
                }
                else {
                    // Random Movement (Fallback for Grey tanks or when Smart AI fails roll)
                    int dir = rand() % 4;
                    if (dir == 0) tank_facingX[i] = 1.35f;       // Right
                    else if (dir == 1) tank_facingX[i] = -1.35f; // Left
                    else if (dir == 2) tank_facingY[i] = 1.35f;  // Down
                    else if (dir == 3) tank_facingY[i] = -1.35f; // Up
                }
            }

            dx = tank_facingX[i];
            dy = tank_facingY[i];
        }

        // --- Apply Movement ---
        if (dx != 0 || dy != 0) {
            // Update facing direction for player
            if (tank_isPlayer[i]) {
                tank_facingX[i] = dx;
                tank_facingY[i] = dy;
            }

            // Calculate new position
            float newX = tank_x[i] + dx * tank_speed[i] * dt;
            float newY = tank_y[i] + dy * tank_speed[i] * dt;

            // Bounds Check
            if (newX < 0) newX = 0;
            if (newX + tank_width[i] > playAreaW) newX = playAreaW - tank_width[i];
            if (newY < playAreaTop) newY = playAreaTop;
            if (newY + tank_height[i] > playAreaTop + playAreaH) newY = playAreaTop + playAreaH - tank_height[i];

            // Collision Check
            if (!check_block_collision(newX, newY, tank_width[i], tank_height[i]) &&
                !check_tank_collision(i, newX, newY, tank_width[i], tank_height[i])) {
                tank_x[i] = newX;
                tank_y[i] = newY;
            }
        }

        // --- Visual Update ---
        tank_shape[i].setPosition(tank_x[i], tank_y[i]);
        if (tank_useSprite[i]) {
            // Sprite Direction Logic
            int frame = 0;
            float fdx = tank_facingX[i];
            float fdy = tank_facingY[i];
            if (abs(fdx) > abs(fdy)) {
                frame = (fdx > 0.0f) ? 1 : 0; // Right : Left
            }
            else {
                frame = (fdy > 0.0f) ? 3 : 2; // Down : Up
            }

            Vector2u s;
            if (tank_skin[i] == 3)      s = assets::tex_tankYellow.getSize();
            else if (tank_skin[i] == 2) s = assets::tex_tankRed.getSize();
            else if (tank_skin[i] == 1) s = assets::tex_tankGreen.getSize();
            else                        s = assets::tex_tankGrey.getSize();

            if (s.x > 0 && s.y > 0) {
                int frames = s.x / s.y;
                if (frames < 1) frames = 1;
                int frameW = s.x / frames;
                int frameH = s.y;

                if (frame >= frames) frame = 0;

                tank_sprite[i].setTextureRect(IntRect(frame * frameW, 0, frameW, frameH));
            }

            tank_sprite[i].setRotation(0);
            tank_sprite[i].setOrigin(0, 0);
            tank_sprite[i].setPosition(tank_x[i], tank_y[i]);
        }

        // --- Shooting Logic ---
        if (tank_shootTimer[i] > 0) tank_shootTimer[i] -= dt;

        bool shoot = false;
        if (tank_isPlayer[i]) {
            if (Keyboard::isKeyPressed(Keyboard::Space)) shoot = true;
        }
        else {
            // AI fires randomly if cooldown is ready
            if (tank_shootTimer[i] <= 0 && (rand() % 70 < 2)) shoot = true;
        }

        if (shoot && tank_shootTimer[i] <= 0) {
            // Spawn bullet at tank center
            float bx = tank_x[i] + tank_width[i] / 2.0f;
            float by = tank_y[i] + tank_height[i] / 2.0f;
            // Add velocity
            float bvx = tank_facingX[i] * 300.0f;
            float bvy = tank_facingY[i] * 300.0f;
            // Offset spawn point to prevent self-collision
            bx += tank_facingX[i] * 20.0f;
            by += tank_facingY[i] * 20.0f;

            init_bullet_entry(bx, by, bvx, bvy, tank_id[i]);
            tank_shootTimer[i] = tank_shootInterval[i];
        }
    }
}

void draw_tanks(RenderWindow& window) {
    static Clock blinkClock;
    float blinkTime = blinkClock.getElapsedTime().asSeconds();

    for (int i = 0; i < MAX_TANKS; ++i) {
        if (!tank_active[i]) continue;

        // Visual blinking effect for invulnerable tanks
        if (tank_invulnerable[i]) {
            if (((int)(blinkTime * 6.67f)) % 2 == 0) {
                continue;
            }
        }

        if (tank_useSprite[i]) {
            window.draw(tank_sprite[i]);
        }
        else {
            window.draw(tank_shape[i]);
        }

        // Draw shield indicator
        if (tank_invulnerable[i]) {
            CircleShape shield(tank_width[i] * 0.7f);
            shield.setPosition(tank_x[i] - tank_width[i] * 0.2f, tank_y[i] - tank_height[i] * 0.2f);
            shield.setFillColor(Color::Transparent);
            shield.setOutlineColor(Color(100, 200, 255, 100));
            shield.setOutlineThickness(2.0f);
            window.draw(shield);
        }
    }
}

// Damage Handling Logic
void damage_tank(int index, int damage) {
    if (index < 0 || index >= MAX_TANKS || !tank_active[index]) return;
    if (tank_invulnerable[index]) return;

    tank_lives[index] -= damage;

    // Tank Destruction
    if (tank_lives[index] <= 0) {

        if (!tank_isPlayer[index]) {
            // Enemy Destruction
            tank_active[index] = false;
            play_explosion_sfx();
            enemiesDestroyed++;
            score += 100;

            // Update powerup counter
            powerup_killCount++;
            cout << "[Game] Enemy Killed! Powerup kill count: " << powerup_killCount << "/3" << endl;
        }
        else {
            // Player Destruction
            tank_totalLives[index]--;
            play_player_death_sfx();

            if (tank_totalLives[index] <= 0) {
                tank_active[index] = false;
                gameOver = true; // Game Over... for real this time.
                play_game_over_sfx();
            }
            else {
                // Respawn Logic
                tank_lives[index] = 1;

                // Find a safe respawn spot near the base
                float respawnX = baseRect.left + baseRect.width / 2.0f - tank_width[index] / 2.0f;
                float respawnY = baseRect.top - tank_height[index] - 20.0f;

                bool foundSafe = false;
                // Try 8 different offset positions to find a clear spot
                for (int attempt = 0; attempt < 8 && !foundSafe; ++attempt) {
                    float testX = respawnX;
                    float testY = respawnY;

                    if (attempt == 1) testX -= 60;
                    else if (attempt == 2) testX += 60;
                    else if (attempt == 3) testY -= 60;
                    else if (attempt == 4) { testX -= 60; testY -= 60; }
                    else if (attempt == 5) { testX += 60; testY -= 60; }
                    else if (attempt == 6) testY -= 120;
                    else if (attempt == 7) testY += 60;

                    // Bounds Check
                    if (testX < 0 || testX + tank_width[index] > PLAY_AREA_W ||
                        testY < PLAY_AREA_TOP || testY + tank_height[index] > PLAY_AREA_TOP + PLAY_AREA_H) {
                        continue;
                    }

                    // Collision Check
                    if (!check_block_collision(testX, testY, tank_width[index], tank_height[index]) &&
                        !check_tank_collision(index, testX, testY, tank_width[index], tank_height[index])) {
                        respawnX = testX;
                        respawnY = testY;
                        foundSafe = true;
                    }
                }

                // Apply Respawn
                tank_x[index] = respawnX;
                tank_y[index] = respawnY;
                tank_invulnerable[index] = true;
                tank_invulTimer[index] = 3.0f; // Temporary shield

                tank_facingX[index] = 0.0f;
                tank_facingY[index] = -1.0f;
            }
        }
    }
}