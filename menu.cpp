#include "menu.h"
#include "game_data.h"
#include "persistence.h"
#include "tanks.h" // for init_tank
#include "blocks.h" // for load_level
#include "assets.h"
#include "game_constants.h"
#include "audio.h" // Added for volume controls
#include <iostream>
using namespace std;
using namespace sf;

// Menu State Management
enum MenuState { MAIN, SETTINGS, HIGHSCORES, INSTRUCTIONS, PAUSE };
static MenuState menuState = MAIN; // Default start screen

// Navigation Variables
static int selectedItem = 0;
static const int MENU_ITEMS = 6;
static const string menuLabels[] = { "New Game", "Load Game", "High Scores", "Instructions", "Settings", "Exit" };

// Settings Configuration
static int settingsSelection = 0;
static int startingLevel = 1;
static int difficulty = 0; // 0 = Easy, 1 = Normal, 2 = Hard
static const string difficultyLabels[] = { "Easy", "Normal", "Hard" };

// CHANGED: Increased items to 5 to include Volume
static const int SETTINGS_ITEMS = 5;

// --- NEW FLAG: Load Error State ---
static bool showLoadError = false;

// Static Content
static const string instructionsText[] = {
    "CONTROLS:",
    "  WASD / Arrow Keys:       Move tank",
    "  Spacebar:                Fire bullet",
    "  P:                       Pause game",
    "  ESC:                     Pause/Escape menu",
    "",
    "OBJECTIVE:",
    "  Destroy all enemy tanks before they reach your base",
    "  Protect your base, you have only 1 base hp",
    "  Reach target to advance levels",
    "",
    "ENEMY TYPES:",
    "  Grey Tank: ",
    "  Red Tank: ",
    "  Green Tank: ",
    "",
    "There are 10 levels with increasing difficulty.",
    "",
    "ENJOY!!!"
};

static Font font;
static Text text;

// Initialize Menu Resources
void init_menu() {
    if (!font.loadFromFile("data/joystix monospace.ttf")) {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            cerr << "Failed to load font" << endl;
        }
    }
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(Color::White);
}

// Start New Game
void start_new_game() {
    score = 0;
    currentLevel = startingLevel;
    enemiesDestroyed = 0;
    killTarget = 5 + difficulty * 3;

    gameOver = false;
    paused = false;
    baseAlive = true;
    inMenu = false;
    scoreSaved = false;

    tank_count = 0;
    bullet_count = 0;
    block_count = 0;
    for (int i = 0; i < MAX_TANKS; ++i) tank_active[i] = false;
    for (int i = 0; i < MAX_BULLETS; ++i) bullet_active[i] = false;

    string levelFile = "data/levels/level" + to_string(currentLevel) + ".txt";
    load_level(levelFile.c_str());

    baseRect = FloatRect(PLAY_AREA_W / 2.0f - 30.0f, PLAY_AREA_H - 60.0f, 60.0f, 60.0f);

    float playerX = baseRect.left + baseRect.width / 2.0f - 18.0f;
    float playerY = baseRect.top - 45.0f;
    init_tank_entry(0, 1, true, playerX, playerY, 36.0f, 36.0f, Color::Green);

    int enemyCount = 3 + difficulty;
    for (int i = 0; i < enemyCount; ++i) {
        float enemyX = 100.0f + (i * 300.0f);
        float enemyY = 100.0f;
        spawn_enemy(100 + i, enemyX, enemyY, 1 + (i % 3));
    }
}

// Update Menu Logic
void update_menu(RenderWindow& window) {
    static bool upPressed = false;
    static bool downPressed = false;
    static bool leftPressed = false;
    static bool rightPressed = false;
    static bool enterPressed = false;

    // --- ERROR POPUP LOGIC ---
    // If the error message is showing, intercept all input to dismiss it.
    if (showLoadError) {
        if (Keyboard::isKeyPressed(Keyboard::Enter) || Keyboard::isKeyPressed(Keyboard::Escape)) {
            if (!enterPressed) {
                showLoadError = false; // Dismiss error
                enterPressed = true;
            }
        }
        else {
            enterPressed = false;
        }
        return; // Skip normal menu navigation
    }

    // Up Navigation
    if (Keyboard::isKeyPressed(Keyboard::Up)) {
        if (!upPressed) {
            if (menuState == SETTINGS) {
                settingsSelection--;
                if (settingsSelection < 0) settingsSelection = SETTINGS_ITEMS - 1;
            }
            else {
                selectedItem--;
                if (selectedItem < 0) selectedItem = MENU_ITEMS - 1;
            }
            upPressed = true;
        }
    }
    else {
        upPressed = false;
    }

    // Down Navigation
    if (Keyboard::isKeyPressed(Keyboard::Down)) {
        if (!downPressed) {
            if (menuState == SETTINGS) {
                settingsSelection++;
                if (settingsSelection >= SETTINGS_ITEMS) settingsSelection = 0;
            }
            else {
                selectedItem++;
                if (selectedItem >= MENU_ITEMS) selectedItem = 0;
            }
            downPressed = true;
        }
    }
    else {
        downPressed = false;
    }

    // Settings Modification (Left / Right)
    if (menuState == SETTINGS) {
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            if (!leftPressed) {
                if (settingsSelection == 0) { // Level
                    startingLevel--;
                    if (startingLevel < 1) startingLevel = 1;
                }
                else if (settingsSelection == 1) { // Difficulty
                    difficulty--;
                    if (difficulty < 0) difficulty = 0;
                }
                else if (settingsSelection == 2) { // NEW: Volume
                    decrease_music_volume();
                }
                leftPressed = true;
            }
        }
        else {
            leftPressed = false;
        }

        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            if (!rightPressed) {
                if (settingsSelection == 0) { // Level
                    startingLevel++;
                    if (startingLevel > 10) startingLevel = 10;
                }
                else if (settingsSelection == 1) { // Difficulty
                    difficulty++;
                    if (difficulty > 2) difficulty = 2;
                }
                else if (settingsSelection == 2) { // NEW: Volume
                    increase_music_volume();
                }
                rightPressed = true;
            }
        }
        else {
            rightPressed = false;
        }
    }

    // Confirm Selection (Enter)
    if (Keyboard::isKeyPressed(Keyboard::Enter)) {
        if (!enterPressed) {
            if (menuState == MAIN) {
                if (selectedItem == 0) { // New Game
                    start_new_game();
                }
                else if (selectedItem == 1) { // Load Game
                    if (!load_game()) {
                        showLoadError = true;
                    }
                }
                else if (selectedItem == 2) { // High Scores
                    menuState = HIGHSCORES;
                }
                else if (selectedItem == 3) { // Instructions
                    menuState = INSTRUCTIONS;
                }
                else if (selectedItem == 4) { // Settings
                    menuState = SETTINGS;
                    settingsSelection = 0;
                }
                else if (selectedItem == 5) { // Exit
                    window.close();
                }
            }
            else if (menuState == HIGHSCORES || menuState == INSTRUCTIONS) {
                menuState = MAIN;
            }
            else if (menuState == SETTINGS) {
                if (settingsSelection == 3) { // Mute Toggle (Shifted to 3)
                    toggle_music_mute();
                }
                else if (settingsSelection == 4) { // Back (Shifted to 4)
                    menuState = MAIN;
                }
            }
            enterPressed = true;
        }
    }
    else {
        enterPressed = false;
    }
}

// Render Menu
void draw_menu(RenderWindow& window) {
    window.clear(Color(74, 5, 4));

    // State: Main Menu
    if (menuState == MAIN) {
        // Title Card
        RectangleShape titlePanel(Vector2f(800, 200));
        titlePanel.setPosition(550, 80);
        titlePanel.setFillColor(Color(30, 30, 50, 200));
        titlePanel.setOutlineColor(Color(255, 215, 0, 150));
        titlePanel.setOutlineThickness(3.0f);
        window.draw(titlePanel);

        // Title Text
        text.setCharacterSize(80);
        text.setFillColor(Color(255, 215, 0));
        text.setOutlineColor(Color(200, 100, 0));
        text.setOutlineThickness(4.0f);
        text.setStyle(Text::Bold);
        text.setString("Neon Siege:");

        FloatRect titleBounds = text.getLocalBounds();
        text.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        text.setPosition(950, 140);
        window.draw(text);

        // Subtitle Text
        text.setCharacterSize(26);
        text.setFillColor(Color(150, 10, 0));
        text.setOutlineThickness(0);
        text.setStyle(Text::Regular);
        text.setString("Armoured Conquest");

        FloatRect subtitleBounds = text.getLocalBounds();
        text.setOrigin(subtitleBounds.width / 2.0f, subtitleBounds.height / 2.0f);
        text.setPosition(950, 220);
        window.draw(text);

        // Menu Buttons Loop
        float startY = 300;
        float itemHeight = 70;

        for (int i = 0; i < MENU_ITEMS; ++i) {
            bool isSelected = (i == selectedItem);

            RectangleShape itemPanel(Vector2f(500, 60));
            itemPanel.setPosition(700, startY + i * itemHeight);

            if (isSelected) {
                itemPanel.setFillColor(Color(60, 60, 100, 220));
                itemPanel.setOutlineColor(Color(255, 215, 0, 200));
                itemPanel.setOutlineThickness(3.0f);
            }
            else {
                itemPanel.setFillColor(Color(40, 40, 60, 180));
                itemPanel.setOutlineColor(Color(80, 80, 120, 150));
                itemPanel.setOutlineThickness(2.0f);
            }
            window.draw(itemPanel);

            text.setCharacterSize(28);
            text.setStyle(Text::Bold);
            text.setOutlineThickness(2.0f);
            text.setOutlineColor(Color::Black);

            if (isSelected) {
                text.setFillColor(Color(255, 215, 0));
            }
            else {
                text.setFillColor(Color(200, 200, 220));
            }

            text.setString(menuLabels[i]);
            FloatRect textBounds = text.getLocalBounds();
            text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
            text.setPosition(950, startY + i * itemHeight + 30);
            window.draw(text);

            if (isSelected) {
                text.setOrigin(0, 0);
                text.setString(">");
                text.setFillColor(Color(255, 215, 0));
                text.setPosition(730, startY + i * itemHeight + 15);
                window.draw(text);

                text.setString("<");
                text.setPosition(1160, startY + i * itemHeight + 15);
                window.draw(text);
            }
        }

        text.setCharacterSize(18);
        text.setFillColor(Color(120, 120, 150));
        text.setOutlineThickness(0);
        text.setStyle(Text::Regular);
        text.setString("Use Arrow Keys to navigate | Press Enter to select");

        FloatRect instrBounds = text.getLocalBounds();
        text.setOrigin(instrBounds.width / 2.0f, instrBounds.height / 2.0f);
        text.setPosition(950, 730);
        window.draw(text);

    }
    // State: High Scores
    else if (menuState == HIGHSCORES) {
        text.setCharacterSize(48);
        text.setFillColor(Color(255, 215, 0));
        text.setOutlineColor(Color::Black);
        text.setOutlineThickness(3.0f);
        text.setStyle(Text::Bold);
        text.setString("HIGH SCORES");

        FloatRect titleBounds = text.getLocalBounds();
        text.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        text.setPosition(950, 100);
        window.draw(text);

        RectangleShape scoresPanel(Vector2f(700, 400));
        scoresPanel.setPosition(600, 180);
        scoresPanel.setFillColor(Color(30, 30, 50, 200));
        scoresPanel.setOutlineColor(Color(100, 100, 150, 150));
        scoresPanel.setOutlineThickness(2.0f);
        window.draw(scoresPanel);

        int scores[10];
        load_high_scores(scores, 10);

        text.setStyle(Text::Regular);
        text.setOutlineThickness(1.5f);

        for (int i = 0; i < 5; ++i) {
            text.setCharacterSize(32);
            text.setFillColor(Color(180, 180, 200));
            text.setString(to_string(i + 1) + ".");
            text.setOrigin(0, 0);
            text.setPosition(670, 220 + i * 70);
            window.draw(text);

            text.setCharacterSize(36);
            text.setFillColor(Color(255, 215, 0));
            text.setString(to_string(scores[i]));
            text.setPosition(770, 220 + i * 70);
            window.draw(text);
        }

        text.setCharacterSize(20);
        text.setFillColor(Color(150, 150, 180));
        text.setOutlineThickness(0);
        text.setString("Press Enter to return");

        FloatRect instrBounds = text.getLocalBounds();
        text.setOrigin(instrBounds.width / 2.0f, instrBounds.height / 2.0f);
        text.setPosition(950, 650);
        window.draw(text);

    }
    // State: Instructions
    else if (menuState == INSTRUCTIONS) {
        text.setCharacterSize(48);
        text.setFillColor(Color(255, 215, 0));
        text.setOutlineColor(Color::Black);
        text.setOutlineThickness(3.0f);
        text.setStyle(Text::Bold);
        text.setString("INSTRUCTIONS");

        FloatRect titleBounds = text.getLocalBounds();
        text.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        text.setPosition(950, 50);
        window.draw(text);

        RectangleShape instructionsPanel(Vector2f(1000, 640));
        instructionsPanel.setPosition(480, 100);
        instructionsPanel.setFillColor(Color(30, 30, 50, 200));
        instructionsPanel.setOutlineColor(Color(100, 100, 150, 150));
        instructionsPanel.setOutlineThickness(2.0f);
        window.draw(instructionsPanel);

        text.setCharacterSize(20);
        text.setFillColor(Color(220, 220, 240));
        text.setOutlineThickness(1.0f);
        text.setOutlineColor(Color::Black);
        text.setStyle(Text::Regular);

        float yPos = 120;
        for (int i = 0; i < 19; ++i) {
            text.setString(instructionsText[i]);
            text.setOrigin(0, 0);
            text.setPosition(530, yPos);
            window.draw(text);
            yPos += 32;
        }

        text.setCharacterSize(20);
        text.setFillColor(Color(150, 150, 180));
        text.setOutlineThickness(0);
        text.setString("Press Enter to return to Main Menu");

        FloatRect instrBounds = text.getLocalBounds();
        text.setOrigin(instrBounds.width / 2.0f, instrBounds.height / 2.0f);
        text.setPosition(950, 750);
        window.draw(text);
    }
    // State: Settings
    else if (menuState == SETTINGS) {
        text.setCharacterSize(48);
        text.setFillColor(Color(255, 215, 0));
        text.setOutlineColor(Color::Black);
        text.setOutlineThickness(3.0f);
        text.setStyle(Text::Bold);
        text.setString("SETTINGS");

        FloatRect titleBounds = text.getLocalBounds();
        text.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        text.setPosition(950, 100);
        window.draw(text);

        RectangleShape settingsPanel(Vector2f(700, 550)); // Made taller for new option
        settingsPanel.setPosition(600, 180);
        settingsPanel.setFillColor(Color(30, 30, 50, 200));
        settingsPanel.setOutlineColor(Color(100, 100, 150, 150));
        settingsPanel.setOutlineThickness(2.0f);
        window.draw(settingsPanel);

        float yPos = 200;
        float itemHeight = 90;

        // 1. Starting Level Setting
        {
            bool isSelected = (settingsSelection == 0);

            RectangleShape itemPanel(Vector2f(640, 60));
            itemPanel.setPosition(630, yPos);
            if (isSelected) {
                itemPanel.setFillColor(Color(60, 60, 100, 220));
                itemPanel.setOutlineColor(Color(255, 215, 0, 200));
                itemPanel.setOutlineThickness(3.0f);
            }
            else {
                itemPanel.setFillColor(Color(40, 40, 60, 180));
                itemPanel.setOutlineColor(Color(80, 80, 120, 150));
                itemPanel.setOutlineThickness(2.0f);
            }
            window.draw(itemPanel);

            text.setCharacterSize(24);
            text.setStyle(Text::Bold);
            text.setOutlineThickness(1.5f);
            text.setOutlineColor(Color::Black);
            text.setFillColor(Color(200, 200, 220));
            text.setString("Starting Level:");
            text.setOrigin(0, 0);
            text.setPosition(700, yPos + 13);
            window.draw(text);

            text.setCharacterSize(32);
            text.setFillColor(isSelected ? Color(255, 215, 0) : Color(150, 255, 150));
            text.setString("< " + to_string(startingLevel) + " >");
            FloatRect valueBounds = text.getLocalBounds();
            text.setOrigin(valueBounds.width / 2.0f, 0);
            text.setPosition(1100, yPos + 10);
            window.draw(text);
        }

        yPos += itemHeight;

        // 2. Difficulty Setting
        {
            bool isSelected = (settingsSelection == 1);

            RectangleShape itemPanel(Vector2f(640, 60));
            itemPanel.setPosition(630, yPos);
            if (isSelected) {
                itemPanel.setFillColor(Color(60, 60, 100, 220));
                itemPanel.setOutlineColor(Color(255, 215, 0, 200));
                itemPanel.setOutlineThickness(3.0f);
            }
            else {
                itemPanel.setFillColor(Color(40, 40, 60, 180));
                itemPanel.setOutlineColor(Color(80, 80, 120, 150));
                itemPanel.setOutlineThickness(2.0f);
            }
            window.draw(itemPanel);

            text.setCharacterSize(24);
            text.setStyle(Text::Bold);
            text.setOutlineThickness(1.5f);
            text.setOutlineColor(Color::Black);
            text.setFillColor(Color(200, 200, 220));
            text.setString("Difficulty:");
            text.setOrigin(0, 0);
            text.setPosition(700, yPos + 13);
            window.draw(text);

            text.setCharacterSize(32);
            Color diffColor = difficulty == 0 ? Color(100, 255, 100) :
                (difficulty == 1 ? Color(255, 215, 0) : Color(255, 100, 100));
            text.setFillColor(isSelected ? diffColor : Color(180, 180, 200));
            text.setString("< " + string(difficultyLabels[difficulty]) + " >");
            FloatRect valueBounds = text.getLocalBounds();
            text.setOrigin(valueBounds.width / 2.0f, 0);
            text.setPosition(1095, yPos + 10);
            window.draw(text);
        }

        yPos += itemHeight;

        // 3. NEW: Master Volume Setting
        {
            bool isSelected = (settingsSelection == 2);

            RectangleShape itemPanel(Vector2f(640, 60));
            itemPanel.setPosition(630, yPos);
            if (isSelected) {
                itemPanel.setFillColor(Color(60, 60, 100, 220));
                itemPanel.setOutlineColor(Color(255, 215, 0, 200));
                itemPanel.setOutlineThickness(3.0f);
            }
            else {
                itemPanel.setFillColor(Color(40, 40, 60, 180));
                itemPanel.setOutlineColor(Color(80, 80, 120, 150));
                itemPanel.setOutlineThickness(2.0f);
            }
            window.draw(itemPanel);

            text.setCharacterSize(24);
            text.setStyle(Text::Bold);
            text.setOutlineThickness(1.5f);
            text.setOutlineColor(Color::Black);
            text.setFillColor(Color(200, 200, 220));
            text.setString("Master Volume:");
            text.setOrigin(0, 0);
            text.setPosition(700, yPos + 13);
            window.draw(text);

            float currentVol = get_music_volume();
            text.setCharacterSize(32);
            text.setFillColor(isSelected ? Color(255, 215, 0) : Color(100, 200, 255));
            text.setString("< " + to_string((int)currentVol) + "% >");
            FloatRect valueBounds = text.getLocalBounds();
            text.setOrigin(valueBounds.width / 2.0f, 0);
            text.setPosition(1120, yPos + 10);
            window.draw(text);
        }

        yPos += itemHeight;

        // 4. Music Toggle Setting
        {
            bool isSelected = (settingsSelection == 3);

            RectangleShape itemPanel(Vector2f(640, 60));
            itemPanel.setPosition(630, yPos);
            if (isSelected) {
                itemPanel.setFillColor(Color(60, 60, 100, 220));
                itemPanel.setOutlineColor(Color(255, 215, 0, 200));
                itemPanel.setOutlineThickness(3.0f);
            }
            else {
                itemPanel.setFillColor(Color(40, 40, 60, 180));
                itemPanel.setOutlineColor(Color(80, 80, 120, 150));
                itemPanel.setOutlineThickness(2.0f);
            }
            window.draw(itemPanel);

            text.setCharacterSize(24);
            text.setStyle(Text::Bold);
            text.setOutlineThickness(1.5f);
            text.setOutlineColor(Color::Black);
            text.setFillColor(Color(200, 200, 220));
            text.setString("Mute Music:");
            text.setOrigin(0, 0);
            text.setPosition(700, yPos + 13);
            window.draw(text);

            string musicStatus = isMusicMuted ? "YES" : "NO";
            Color musicColor = isMusicMuted ? Color(255, 100, 100) : Color(100, 255, 100);

            text.setCharacterSize(32);
            text.setFillColor(isSelected ? Color(255, 215, 0) : musicColor);
            text.setString(musicStatus);
            FloatRect valueBounds = text.getLocalBounds();
            text.setOrigin(valueBounds.width / 2.0f, 0);
            text.setPosition(1120, yPos + 10);
            window.draw(text);
        }

        yPos += itemHeight;

        // 5. Back Button
        {
            bool isSelected = (settingsSelection == 4);

            RectangleShape itemPanel(Vector2f(300, 60));
            itemPanel.setPosition(800, yPos + 10);
            if (isSelected) {
                itemPanel.setFillColor(Color(60, 60, 100, 220));
                itemPanel.setOutlineColor(Color(255, 215, 0, 200));
                itemPanel.setOutlineThickness(3.0f);
            }
            else {
                itemPanel.setFillColor(Color(40, 40, 60, 180));
                itemPanel.setOutlineColor(Color(80, 80, 120, 150));
                itemPanel.setOutlineThickness(2.0f);
            }
            window.draw(itemPanel);

            text.setCharacterSize(28);
            text.setStyle(Text::Bold);
            text.setOutlineThickness(2.0f);
            text.setFillColor(isSelected ? Color(255, 215, 0) : Color(200, 200, 220));
            text.setString("Back");
            FloatRect textBounds = text.getLocalBounds();
            text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
            text.setPosition(950, yPos + 40);
            window.draw(text);
        }

        text.setCharacterSize(16);
        text.setFillColor(Color(150, 150, 180));
        text.setOutlineThickness(0);
        text.setStyle(Text::Regular);
        text.setString("Use Arrow Keys to navigate | Left/Right to change values | Enter to select");

        FloatRect instrBounds = text.getLocalBounds();
        text.setOrigin(instrBounds.width / 2.0f, instrBounds.height / 2.0f);
        text.setPosition(970, 750);
        window.draw(text);
    }

    // -------------LOAD ERROR POPUP ---------
    // If showLoadError is true, draw this on top of everything else.
    if (showLoadError) {
        // Dark background overlay
        RectangleShape overlay(Vector2f(PLAY_AREA_W + 360.0f, PLAY_AREA_H)); // Cover entire window
        overlay.setPosition(0, 0);
        overlay.setFillColor(Color(0, 0, 0, 220)); // High opacity black
        window.draw(overlay);

        // Error Panel
        RectangleShape errorPanel(Vector2f(600, 200));
        errorPanel.setPosition(650, 350); // Centered roughly
        errorPanel.setFillColor(Color(50, 10, 10)); // Dark red bg
        errorPanel.setOutlineColor(Color::Red);
        errorPanel.setOutlineThickness(3.0f);
        window.draw(errorPanel);

        // "LOAD FAILED" Text
        text.setCharacterSize(48);
        text.setStyle(Text::Bold);
        text.setFillColor(Color::Red);
        text.setOutlineColor(Color::Black);
        text.setOutlineThickness(2.0f);
        text.setString("LOAD FAILED");

        FloatRect titleBounds = text.getLocalBounds();
        text.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        text.setPosition(950, 400);
        window.draw(text);

        // Instruction Text
        text.setCharacterSize(20);
        text.setStyle(Text::Regular);
        text.setFillColor(Color::White);
        text.setOutlineThickness(0);
        text.setString("File corrupt or empty. Press Enter.");

        FloatRect msgBounds = text.getLocalBounds();
        text.setOrigin(msgBounds.width / 2.0f, msgBounds.height / 2.0f);
        text.setPosition(950, 480);
        window.draw(text);
    }
}

// In-Game Pause Menu Logic
static int pauseSelection = 0;
static const string pauseLabels[] = { "Resume", "Save Game", "Exit to Menu" };

void handle_pause_input(RenderWindow& window) {
    static bool upPressed = false;
    static bool downPressed = false;
    static bool enterPressed = false;

    // Up Navigation
    if (Keyboard::isKeyPressed(Keyboard::Up)) {
        if (!upPressed) {
            pauseSelection--;
            if (pauseSelection < 0) pauseSelection = 2; // Wrap
            upPressed = true;
        }
    }
    else {
        upPressed = false;
    }

    // Down Navigation
    if (Keyboard::isKeyPressed(Keyboard::Down)) {
        if (!downPressed) {
            pauseSelection++;
            if (pauseSelection > 2) pauseSelection = 0; // Wrap
            downPressed = true;
        }
    }
    else {
        downPressed = false;
    }

    // Selection
    if (Keyboard::isKeyPressed(Keyboard::Enter)) {
        if (!enterPressed) {
            if (pauseSelection == 0) { // Resume
                paused = false;
            }
            else if (pauseSelection == 1) { // Save
                save_game();
                paused = false; // Immediately unpause after saving
            }
            else if (pauseSelection == 2) { // Exit
                inMenu = true;
                paused = false;
                menuState = MAIN;
            }
            enterPressed = true;
        }
    }
    else {
        enterPressed = false;
    }
}

// Render Pause Overlay
// Draws the menu on top of the paused game screen.
void draw_pause_menu(RenderWindow& window) {
    // Dark Overlay Effect
    RectangleShape overlay(Vector2f(PLAY_AREA_W, PLAY_AREA_H));
    overlay.setPosition(0, 0);
    overlay.setFillColor(Color(0, 0, 0, 180));
    window.draw(overlay);

    // Menu Box
    RectangleShape menuPanel(Vector2f(800, 450));
    menuPanel.setPosition(380, 165);
    menuPanel.setFillColor(Color(20, 20, 40, 240));
    menuPanel.setOutlineColor(Color(255, 215, 0, 200));
    menuPanel.setOutlineThickness(4.0f);
    window.draw(menuPanel);

    // Title
    text.setCharacterSize(64);
    text.setFillColor(Color(255, 215, 0));
    text.setOutlineColor(Color(200, 100, 0));
    text.setOutlineThickness(3.0f);
    text.setStyle(Text::Bold);
    text.setString("PAUSED");

    FloatRect titleBounds = text.getLocalBounds();
    text.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    text.setPosition(780, 230);
    window.draw(text);

    // Render Buttons
    float startY = 320;
    float itemHeight = 80;

    for (int i = 0; i < 3; ++i) {
        bool isSelected = (i == pauseSelection);

        // Button Background
        RectangleShape itemPanel(Vector2f(500, 60));
        itemPanel.setPosition(530, startY + i * itemHeight);

        if (isSelected) {
            itemPanel.setFillColor(Color(60, 60, 100, 220));
            itemPanel.setOutlineColor(Color(255, 215, 0, 200));
            itemPanel.setOutlineThickness(3.0f);
        }
        else {
            itemPanel.setFillColor(Color(30, 30, 50, 180));
            itemPanel.setOutlineColor(Color(80, 80, 120, 150));
            itemPanel.setOutlineThickness(2.0f);
        }
        window.draw(itemPanel);

        // Button Text
        text.setCharacterSize(32);
        text.setStyle(Text::Bold);
        text.setOutlineThickness(2.0f);
        text.setOutlineColor(Color::Black);

        if (isSelected) {
            text.setFillColor(Color(255, 215, 0));
        }
        else {
            text.setFillColor(Color(200, 200, 220));
        }

        text.setString(pauseLabels[i]);
        FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        text.setPosition(780, startY + i * itemHeight + 30);
        window.draw(text);

        // Arrows for selected
        if (isSelected) {
            text.setOrigin(0, 0);
            text.setString(">");
            text.setFillColor(Color(255, 215, 0));
            text.setPosition(560, startY + i * itemHeight + 15);
            window.draw(text);

            text.setString("<");
            text.setPosition(990, startY + i * itemHeight + 15);
            window.draw(text);
        }
    }

    text.setCharacterSize(18);
    text.setFillColor(Color(150, 150, 180));
    text.setOutlineThickness(0);
    text.setStyle(Text::Regular);
    text.setString("Use Arrow Keys to navigate | Press Enter to select");

    FloatRect instrBounds = text.getLocalBounds();
    text.setOrigin(instrBounds.width / 2.0f, instrBounds.height / 2.0f);
    text.setPosition(780, 590);
    window.draw(text);
}