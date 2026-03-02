#ifndef audio_h
#define audio_h

// The main initialization function for audio
void init_audio();

// The control function for the settings panel
void toggle_music_mute();

// When playing reduces volume
void set_music_volume(float volume);

// --- NEW: Volume Controls ---
void increase_music_volume();
void decrease_music_volume();
float get_music_volume();
// ----------------------------

//Function to play the explosion sound effect.
void play_explosion_sfx();

//Function to play the player death sound effect.
void play_player_death_sfx();

// Function to play the wall break sound effect.
void play_wall_break_sfx();

// Function to play (Base/Bird Death)
void play_base_death_sfx();

// Function to play next level audio
void play_next_level_sfx();

// Function to play game over audio
void play_game_over_sfx();

// Function to play game win audio
void play_game_win_sfx();

#endif