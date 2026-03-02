#ifndef game_constants_h
#define game_constants_h

// Play area dimensions
extern const float PLAY_AREA_W;
extern const float PLAY_AREA_H;
extern const float PLAY_AREA_TOP;
extern const float WINDOW_H_FLOAT;

// Gameplay tuning
extern const float TANK_SCALE;
extern const float BULLET_RADIUS_BASE;

// Special block types
extern const int KURAK_TYPE;

// Volume
const float MENU_VOLUME = 100.0f; // High volume for menu
const float PAUSED_VOLUME = 0.0f; // Medium volume for paused/game over
const float GAME_VOLUME = 20.0f; // Reduced volume for active gameplay
const float VOLUME_FADE_SPEED = 20.0f; // Volume units per second (Increased fade speed)


#endif