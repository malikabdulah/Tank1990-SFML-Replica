#ifndef GAME_LIMITS_H
#define GAME_LIMITS_H

// Tanks: 1 player + up to 20 simultaneous enemies (waves + respawns)
#define MAX_TANKS 32
// Bullets: player rapid-fire + enemies; allow burst room
#define MAX_BULLETS 256
// Blocks: full grid (LEVEL_ROWS * LEVEL_COLS) — keep product aligned with current constants
// Increased to handle larger levels (30x15=450, but some levels have more cells)
#define MAX_BLOCKS 500
// Highscores stored in memory
#define MAX_HIGHSCORES 5
// Powerups: max active powerups
#define MAX_POWERUPS 3

#endif // GAME_LIMITS_H
