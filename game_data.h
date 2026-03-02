#ifndef game_data_h
#define game_data_h
#include "game_limits.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
using namespace sf;

// Global Game Data Arrays

// --- TANKS ---
extern int tank_count;
extern int tank_id[MAX_TANKS];
extern bool tank_isPlayer[MAX_TANKS];
extern float tank_x[MAX_TANKS];
extern float tank_y[MAX_TANKS];
extern float tank_width[MAX_TANKS];
extern float tank_height[MAX_TANKS];
extern float tank_speed[MAX_TANKS];
extern int tank_lives[MAX_TANKS];
extern int tank_totalLives[MAX_TANKS]; // Total lives remaining (for respawn)
extern int tank_skin[MAX_TANKS]; // 0 = none, 1 = green, 2 = red, 3 = yellow
extern bool tank_useSprite[MAX_TANKS];
extern float tank_spriteScaleX[MAX_TANKS];
extern float tank_spriteScaleY[MAX_TANKS];
extern int tank_aiType[MAX_TANKS];
extern float tank_facingX[MAX_TANKS];
extern float tank_facingY[MAX_TANKS];
extern float tank_moveTimer[MAX_TANKS];
extern float tank_moveInterval[MAX_TANKS];
extern float tank_shootTimer[MAX_TANKS];
extern float tank_shootInterval[MAX_TANKS];
extern bool tank_invulnerable[MAX_TANKS];
extern float tank_invulTimer[MAX_TANKS];
extern bool tank_frozen[MAX_TANKS];
extern float tank_freezeTimer[MAX_TANKS];
extern bool tank_active[MAX_TANKS];
extern RectangleShape tank_shape[MAX_TANKS];
extern Sprite tank_sprite[MAX_TANKS];

// --- BULLETS ---
extern int bullet_count;
extern bool bullet_active[MAX_BULLETS];
extern float bullet_x[MAX_BULLETS];
extern float bullet_y[MAX_BULLETS];
extern float bullet_vx[MAX_BULLETS];
extern float bullet_vy[MAX_BULLETS];
extern float bullet_radius[MAX_BULLETS];
extern int bullet_ownerId[MAX_BULLETS];
extern bool bullet_useSprite[MAX_BULLETS];
extern CircleShape bullet_shape[MAX_BULLETS];
extern Sprite bullet_sprite[MAX_BULLETS];

// --- BLOCKS ---
extern int block_count;
extern float block_x[MAX_BLOCKS];
extern float block_y[MAX_BLOCKS];
extern float block_w[MAX_BLOCKS];
extern float block_h[MAX_BLOCKS];
extern bool block_solid[MAX_BLOCKS];
extern int block_type[MAX_BLOCKS];
extern int block_hardness[MAX_BLOCKS];
extern bool block_useSprite[MAX_BLOCKS];
extern RectangleShape block_shape[MAX_BLOCKS];
extern Sprite block_sprite[MAX_BLOCKS];

// --- GAME STATE ---
extern int score;
extern int currentLevel;
extern int enemiesDestroyed;
extern int killTarget; // number of enemy kills required to pass current level
extern bool gameOver;
extern bool paused;
extern bool baseAlive;
extern FloatRect baseRect; // Keep this for base logic
extern bool inMenu;
extern bool scoreSaved;

// --- POWERUPS ---
extern bool powerup_active[MAX_POWERUPS];
extern float powerup_x[MAX_POWERUPS];
extern float powerup_y[MAX_POWERUPS];
extern int powerup_type[MAX_POWERUPS]; // 0 = shield, 1 = life, 2 = freeze
extern float powerup_spawnTime[MAX_POWERUPS];

extern int powerup_killCount;
extern float powerup_timer;

// --- Audio ---
extern bool isMusicMuted;
extern Music backgroundMusic;

#endif