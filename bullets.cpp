#include "bullets.h"
#include "game_data.h"
#include "tanks.h" // for damage_tank
#include "assets.h"
#include "audio.h"
#include <cmath>
using namespace std;
using namespace sf;
using namespace assets;

void init_bullet_entry(float x, float y, float vx, float vy, int ownerId) {
    // Search for an inactive bullet slot in the pool to reuse memory
    int idx = -1;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullet_active[i]) {
            idx = i;
            break;
        }
    }
    // If no slots are available (pool full), abort spawning
    if (idx == -1) return;

    // Initialize bullet properties
    bullet_active[idx] = true;
    bullet_x[idx] = x;
    bullet_y[idx] = y;
    bullet_vx[idx] = vx;
    bullet_vy[idx] = vy;
    bullet_radius[idx] = 6.0f;
    bullet_ownerId[idx] = ownerId; // ID of the tank that fired this bullet
    bullet_useSprite[idx] = false;

    // Default graphical fallback (Yellow Circle)
    bullet_shape[idx].setRadius(6.0f);
    bullet_shape[idx].setFillColor(Color::Yellow);
    bullet_shape[idx].setPosition(x, y);

    // Sprite Setup
    if (tex_bullet.getSize().x > 0) {
        bullet_useSprite[idx] = true;
        bullet_sprite[idx].setTexture(tex_bullet);

        // Calculate texture coordinates assuming a sprite strip or grid
        Vector2u texSize = tex_bullet.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            unsigned tw = texSize.x, th = texSize.y;

            // Determine frame dimensions (square frames based on height)
            int frames = (th > 0) ? (int)(tw / th) : 1;
            int frameIdx = 0;

            // Select the texture rectangle for the first frame
            bullet_sprite[idx].setTextureRect(IntRect(frameIdx * th, 0, th, th));

            // Calculate scale factor to match the desired in-game bullet radius (12px diameter)
            float targetPx = bullet_radius[idx] * 2.0f;
            float scaleVal = (th > 0) ? (targetPx / float(th)) : 1.0f;
            bullet_sprite[idx].setScale(scaleVal, scaleVal);

            // Set origin to the center of the sprite for correct rotation
            bullet_sprite[idx].setOrigin((float)th / 2.0f, (float)th / 2.0f);
        }

        bullet_sprite[idx].setPosition(x, y);
    }
}

void update_bullets(float dt, float playAreaW, float playAreaH, float playAreaTop) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        // Skip processing for inactive bullets
        if (!bullet_active[i]) continue;

        // Calculate new position based on velocity and delta time
        float nx = bullet_x[i] + bullet_vx[i] * dt;
        float ny = bullet_y[i] + bullet_vy[i] * dt;

        // Despawn bullet if it leaves the playable area
        if (nx < 0 || nx > playAreaW || ny < playAreaTop || ny > playAreaTop + playAreaH) {
            bullet_active[i] = false;
            continue;
        }

        // -------------- Block Collision Logic -----------------
        FloatRect bRect(nx, ny, 8.0f, 8.0f);
        bool hitBlock = false;

        for (int b = 0; b < block_count; ++b) {
            if (!block_solid[b]) continue; // Skip air or destroyed blocks

            FloatRect br(block_x[b], block_y[b], block_w[b], block_h[b]);
            if (bRect.intersects(br)) {
                hitBlock = true;

                // Handle destructible blocks (hardness > 0)
                if (block_hardness[b] > 0) {
                    block_hardness[b]--;
                    // If hardness reaches 0, destroy the block
                    if (block_hardness[b] <= 0) {
                        block_solid[b] = false;
                        play_wall_break_sfx();
                        block_useSprite[b] = false;
                        block_shape[b].setFillColor(Color::Transparent);
                    }
                }
                break; // Exit loop after hitting the first block
            }
        }

        // ------------------ Base Collision Logic ----------------
        if (baseAlive && bRect.intersects(baseRect)) {
            baseAlive = false;
            isMusicMuted = true; // Silence music for dramatic effect
            play_base_death_sfx();
            play_game_over_sfx();
            gameOver = true; // Game Over
            hitBlock = true;
            isMusicMuted = false; // Silence music for dramatic effect
        }

        //--------------- Bullet-vs-Bullet Collision Logic------------------
        if (!hitBlock) { // Only check if the bullet hasn't already hit a wall
            for (int j = 0; j < MAX_BULLETS; ++j) {
                // Prevent self-collision and ignore inactive bullets
                if (i == j || !bullet_active[j]) continue;

                FloatRect otherBulletRect(bullet_x[j], bullet_y[j], 8.0f, 8.0f);

                // Check for intersection
                if (bRect.intersects(otherBulletRect)) {
                    // Destroy both projectiles upon collision
                    bullet_active[i] = false;
                    bullet_active[j] = false;

                    hitBlock = true; // Mark current bullet as handled
                    break;
                }
            }
        }

        // If collision occurred (Block, Base, or Bullet), deactivate this bullet and skip further checks
        if (hitBlock) {
            bullet_active[i] = false;
            // TODO: Spawn explosion effect here
            continue;
        }

        // --------------------- Tank Collision Logic -------------------------
        for (int t = 0; t < MAX_TANKS; ++t) {
            if (!tank_active[t]) continue;

            // Prevent the bullet from hitting the tank that fired it
            if (tank_id[t] == bullet_ownerId[i]) continue;

            // Friendly Fire Logic: Determine the shooter's team
            bool bulletFromPlayer = false;
            for (int s = 0; s < MAX_TANKS; ++s) {
                if (tank_active[s] && tank_id[s] == bullet_ownerId[i]) {
                    bulletFromPlayer = tank_isPlayer[s];
                    break;
                }
            }

            //Valid Hit Conditions
            //Player bullet hits Enemy tank
            //Enemy bullet hits Player tank
            if (bulletFromPlayer && !tank_isPlayer[t]) {
                // Valid hit on enemy
            }
            else if (!bulletFromPlayer && tank_isPlayer[t]) {
                // Valid hit on player
            }
            else {
                // Same team (Player hitting Player or Enemy hitting Enemy) - Ignore collision
                continue;
            }

            // Check geometric intersection
            FloatRect tr(tank_x[t], tank_y[t], tank_width[t], tank_height[t]);
            if (bRect.intersects(tr)) {
                bullet_active[i] = false; // Destroy bullet
                damage_tank(t, 1);        // Apply damage to tank
                break;
            }
        }

        // ------------------- Final Position Update --------------------------
        // If the bullet is still active after all checks, update its position and visual
        if (bullet_active[i]) {
            bullet_x[i] = nx;
            bullet_y[i] = ny;
            bullet_shape[i].setPosition(nx, ny);

            if (bullet_useSprite[i]) {
                bullet_sprite[i].setPosition(nx, ny);

                // Calculate rotation angle in degrees from velocity vector
                float ang = atan2(bullet_vy[i], bullet_vx[i]) * 180.0f / 3.14159265f;
                // Adjust rotation (sprite likely faces Up or Left by default, requires 180 offset)
                bullet_sprite[i].setRotation(ang + 180.0f);
            }
        }
    }
}

void draw_bullets(RenderWindow& window) {
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullet_active[i]) continue;

        // Render sprite if available, otherwise render fallback shape
        if (bullet_useSprite[i]) {
            window.draw(bullet_sprite[i]);
        }
        else {
            window.draw(bullet_shape[i]);
        }
    }
}