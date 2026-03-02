#include "game_data.h"
using namespace std;
using namespace sf;

// Definitions of global arrays

// --- TANKS ---
int tank_count = 0;
int tank_id[MAX_TANKS];
bool tank_isPlayer[MAX_TANKS];
float tank_x[MAX_TANKS];
float tank_y[MAX_TANKS];
float tank_width[MAX_TANKS];
float tank_height[MAX_TANKS];
float tank_speed[MAX_TANKS];
int tank_lives[MAX_TANKS];
int tank_totalLives[MAX_TANKS];
int tank_skin[MAX_TANKS];
bool tank_useSprite[MAX_TANKS];
float tank_spriteScaleX[MAX_TANKS];
float tank_spriteScaleY[MAX_TANKS];
int tank_aiType[MAX_TANKS];
float tank_facingX[MAX_TANKS];
float tank_facingY[MAX_TANKS];
float tank_moveTimer[MAX_TANKS];
float tank_moveInterval[MAX_TANKS];
float tank_shootTimer[MAX_TANKS];
float tank_shootInterval[MAX_TANKS];
bool tank_invulnerable[MAX_TANKS];
float tank_invulTimer[MAX_TANKS];
bool tank_frozen[MAX_TANKS];
float tank_freezeTimer[MAX_TANKS];
bool tank_active[MAX_TANKS];
RectangleShape tank_shape[MAX_TANKS];
Sprite tank_sprite[MAX_TANKS];

// --- BULLETS ---
int bullet_count = 0;
bool bullet_active[MAX_BULLETS];
float bullet_x[MAX_BULLETS];
float bullet_y[MAX_BULLETS];
float bullet_vx[MAX_BULLETS];
float bullet_vy[MAX_BULLETS];
float bullet_radius[MAX_BULLETS];
int bullet_ownerId[MAX_BULLETS];
bool bullet_useSprite[MAX_BULLETS];
CircleShape bullet_shape[MAX_BULLETS];
Sprite bullet_sprite[MAX_BULLETS];

// --- BLOCKS ---
int block_count = 0;
float block_x[MAX_BLOCKS];
float block_y[MAX_BLOCKS];
float block_w[MAX_BLOCKS];
float block_h[MAX_BLOCKS];
bool block_solid[MAX_BLOCKS];
int block_type[MAX_BLOCKS];
int block_hardness[MAX_BLOCKS];
bool block_useSprite[MAX_BLOCKS];
RectangleShape block_shape[MAX_BLOCKS];
Sprite block_sprite[MAX_BLOCKS];

// --- GAME STATE ---
int score = 0;
int currentLevel = 1;
int enemiesDestroyed = 0;
int killTarget = 5; // default kill target
bool gameOver = false;
bool paused = false;
bool baseAlive = true;
FloatRect baseRect;
bool inMenu = true;
bool scoreSaved = false;

// --- POWERUPS ---
bool powerup_active[MAX_POWERUPS];
float powerup_x[MAX_POWERUPS];
float powerup_y[MAX_POWERUPS];
int powerup_type[MAX_POWERUPS];
float powerup_spawnTime[MAX_POWERUPS];

int powerup_killCount = 0;
float powerup_timer = 0.0f;

// --- Audio ---
bool isMusicMuted = false;
Music backgroundMusic;