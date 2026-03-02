#include "game_data.h"
#include "audio.h"
#include "game_constants.h" // For MAX_VOLUME
#include <iostream>
using namespace std;

// Assume MAX_VOLUME is defined in game_constants.h or defined here
const float MAX_VOLUME = 60;

// CHANGED: Default volume set to 50% so it's not too loud on startup
float bgMusicVolume =20.0f;

// Wall break
SoundBuffer wallBreakBuffer;
Sound wallBreakSounds;

// Enemy Tank Explosion
SoundBuffer explosionBuffer;
Sound explosionSound;

// Player death audio
SoundBuffer playerDeathBuffer;
Sound playerDeathSounds;

// Base death
SoundBuffer baseDeathBuffer;
Sound baseDeathSounds;

// Next level sfx
SoundBuffer nextLevelBuffer;
Sound nextLevelSounds;

// Game Over audio
SoundBuffer gameOverBuffer;
Sound gameOverSounds;

// Game Win audio
SoundBuffer gameWinBuffer;
Sound gameWinSounds;


//Initializes, loads, and starts the background music stream.
void init_audio() {
    // Load the file. Change the path to where your audio file is located.
    if (!backgroundMusic.openFromFile("data/audio/bg_music.ogg")) {
        // Output error if file isn't found
        cerr << "Error: Could not load background music file. Check path." << endl;
    }
    else {
        //Configure and Play
        backgroundMusic.setLoop(true); // Loops indefinitely
        backgroundMusic.setVolume(bgMusicVolume);
        backgroundMusic.play();
        cout << "Background music initialized and started." << endl;
    }

    // Load the file. Change the path to where your audio file is located.
    if (explosionBuffer.loadFromFile("data/audio/explosion.wav")) {
        explosionSound.setBuffer(explosionBuffer);
        // Set a default volume
        explosionSound.setVolume(80.0f);
    }
    else {
        cerr << "Failed to load explosion sound effect!" << endl;
    }

    // Load the file. Change the path to where your audio file is located.
    if (playerDeathBuffer.loadFromFile("data/audio/player_death.wav")) {
        playerDeathSounds.setBuffer(playerDeathBuffer);
        playerDeathSounds.setVolume(100.0f); // Maximum volume for impact
    }
    else {
        cerr << "Failed to load game over sound effect!" << endl;
    }

    // Load the file. Change the path to where your audio file is located.
    if (wallBreakBuffer.loadFromFile("data/audio/wall_break.wav")) {
        wallBreakSounds.setBuffer(wallBreakBuffer);
        wallBreakSounds.setVolume(100.0f); // Slightly lower volume than explosion
    }
    else {
        cerr << "Failed to load wall break sound effect!" << endl;
    }

    // Load the file. Change the path to where your audio file is located.
    if (baseDeathBuffer.loadFromFile("data/audio/base_death.wav")) {
        baseDeathSounds.setBuffer(baseDeathBuffer);
        baseDeathSounds.setVolume(100.0f); // Make it loud
    }
    else {
        cerr << "Failed to load base death sound effect!" << endl;
    }

    // Load the file. Change the path to where your audio file is located.
    if (nextLevelBuffer.loadFromFile("data/audio/next_level.wav")) {
        nextLevelSounds.setBuffer(nextLevelBuffer);
        nextLevelSounds.setVolume(100.0f);
    }
    else {
        cerr << "Failed to load next level sound effect!" << endl;
    }

    // Load the file. Change the path to where your audio file is located.
    if (gameOverBuffer.loadFromFile("data/audio/game_over.wav")) {
        gameOverSounds.setBuffer(gameOverBuffer);
        gameOverSounds.setVolume(150.0f); // Maximum volume for impact
    }
    else {
        cerr << "Failed to load game over sound effect!" << endl;
    }


    // Load the file. Change the path to where your audio file is located.
    if (gameWinBuffer.loadFromFile("data/audio/game_win.wav")) {
        gameWinSounds.setBuffer(gameWinBuffer);
        gameWinSounds.setVolume(150.0f); // Maximum volume for impact
    }
    else {
        cerr << "Failed to load game over sound effect!" << endl;
    }
}

//Toggles the mute state for the background music
void toggle_music_mute() {
    isMusicMuted = !isMusicMuted; // Toggle the state

    if (isMusicMuted) {
        backgroundMusic.setVolume(0.0f);
        cout << "Music muted." << endl;
    }
    else {
        backgroundMusic.setVolume(bgMusicVolume);
        cout << "Music unmuted, volume set to " << bgMusicVolume << endl;
    }
}

// Function to set the volume of the background music
void set_music_volume(float volume) {
    // SFML volume is a float between 0 (silent) and 100 (full volume)
    backgroundMusic.setVolume(volume);
}

// For tank death
void play_explosion_sfx() {
    explosionSound.play();
}

// For player tank death
void play_player_death_sfx() {
    playerDeathSounds.play();
}

// For wall break
void play_wall_break_sfx() {
    wallBreakSounds.play();
}

// for base death
void play_base_death_sfx() {
    baseDeathSounds.play();
}

// for next level
void play_next_level_sfx() {
    nextLevelSounds.play();
}

// for game over
void play_game_over_sfx() {
    gameOverSounds.play();
}

// for game win
void play_game_win_sfx() {
    gameWinSounds.play();
}

// Increases volume by 10
void increase_music_volume() {
    bgMusicVolume += 10.0f;
    if (bgMusicVolume > 100.0f) bgMusicVolume = 100.0f;

    // Immediately update if not muted
    if (!isMusicMuted) backgroundMusic.setVolume(bgMusicVolume);

    // If we raise volume, ensure mute flag is off
    if (bgMusicVolume > 0) isMusicMuted = false;
}

// Decreases volume by 10
void decrease_music_volume() {
    bgMusicVolume -= 10.0f;
    if (bgMusicVolume < 0.0f) bgMusicVolume = 0.0f;

    // Immediately update if not muted
    if (!isMusicMuted) backgroundMusic.setVolume(bgMusicVolume);

    // If volume hits 0, consider it muted
    if (bgMusicVolume == 0) isMusicMuted = true;
}

// Returns the current volume for the Menu to display
float get_music_volume() {
    return bgMusicVolume;
}