#include "game_constants.h"
using namespace std;

// Grid and window size
static const int TILE_SIZE = 52;
static const int LEVEL_COLS = 30;
static const int LEVEL_ROWS = 15;
static const unsigned HUD_HEADER_H = 0;
static const int HUD_PANEL_W = 360;
static const int WINDOW_W = LEVEL_COLS * TILE_SIZE + HUD_PANEL_W;
static const int WINDOW_H = LEVEL_ROWS * TILE_SIZE + HUD_HEADER_H;

// Exported constants - definitions
const float PLAY_AREA_W = (float)(WINDOW_W - HUD_PANEL_W);
const float PLAY_AREA_H = (float)(LEVEL_ROWS * TILE_SIZE);
const float PLAY_AREA_TOP = (float)HUD_HEADER_H;
const float WINDOW_H_FLOAT = (float)WINDOW_H;
const float TANK_SCALE = 1.25f;
const float BULLET_RADIUS_BASE = 12.0f;
const int KURAK_TYPE = 7;