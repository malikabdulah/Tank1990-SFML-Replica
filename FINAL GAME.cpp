#include "tanks.h"
#include "bullets.h"
#include "blocks.h"
#include "menu.h"
#include "persistence.h"
#include "assets.h"
#include "audio.h"
#include "game_constants.h"
#include "game_data.h"
#include "powerups.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;
using namespace assets;

// --- VIEW CONFIGURATION ---
// We add a border margin to the view so we can see "outside" the 0,0 coordinate.
const float VIEW_MARGIN = 10.0f;

// Letterbox View Scaling
static void update_letterbox_view(RenderWindow& window, View& view, unsigned newW, unsigned newH, float viewW, float viewH) {
    if (newW == 0 || newH == 0) return;
    float windowRatio = static_cast<float>(newW) / static_cast<float>(newH);
    float viewRatio = viewW / viewH;
    float sizeX = 1.f; float sizeY = 1.f; float posX = 0.f; float posY = 0.f;

    if (windowRatio > viewRatio) {
        sizeX = viewRatio / windowRatio;
        posX = (1.f - sizeX) * 0.5f;
    }
    else {
        sizeY = windowRatio / viewRatio;
        posY = (1.f - sizeY) * 0.5f;
    }

    view.setSize(viewW, viewH);
    // Center the view so the margins are even on all sides
    view.setCenter(viewW / 2.0f - VIEW_MARGIN, viewH / 2.0f - VIEW_MARGIN);
    view.setViewport(FloatRect(posX, posY, sizeX, sizeY));
    window.setView(view);
}

int main() {
    srand(time(0));

    const float WINDOW_W = PLAY_AREA_W + 360.0f;
    const float WINDOW_H = PLAY_AREA_H;

    RenderWindow window(VideoMode((int)WINDOW_W, (int)WINDOW_H), "Neon Siege: Armoured Conquest");
    window.setFramerateLimit(120);

    // --- VIEW SETUP ---
    // We make the view slightly larger than the window content to reveal the "outside" borders
    float totalViewW = WINDOW_W + (VIEW_MARGIN * 2);
    float totalViewH = WINDOW_H + (VIEW_MARGIN * 2);
    View view(FloatRect(-VIEW_MARGIN, -VIEW_MARGIN, totalViewW, totalViewH));

    update_letterbox_view(window, view, window.getSize().x, window.getSize().y, totalViewW, totalViewH);

    if (!loadAll()) {
        cerr << "Failed to load assets!" << endl;
    }

    init_audio();
    init_menu();
    init_powerups();

    Font hudFont;
    bool hasHudFont = false;
    if (hudFont.loadFromFile("data/joystix monospace.ttf")) {
        hasHudFont = true;
    }
    else if (hudFont.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        hasHudFont = true;
    }

    Clock clock;
    bool gameWon = false;
    float currentMusicVolume = backgroundMusic.getVolume();

    // --------------------------------------------------------
    // MAIN GAME LOOP
    // --------------------------------------------------------
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        // --- Audio Management ---
        // Get the User's master volume preference (0 - 100)
        float masterVolume = get_music_volume();

        float desiredVolume;
        if (inMenu) {
            // In menu, play at full master volume
            desiredVolume = masterVolume;
        }
        else {
            if (paused || gameOver || gameWon) {
                // When paused or game over, silence music or play very quiet
                desiredVolume = 0.0f;
            }
            else {
                // In active gameplay, play at 30% of master volume
                // This ensures it doesn't overpower sound effects
                desiredVolume = masterVolume * 0.3f;
            }
        }

        // Apply mute logic
        float targetVolume = isMusicMuted ? 0.0f : desiredVolume;

        // Smooth fade to target volume
        if (currentMusicVolume < targetVolume) {
            currentMusicVolume += VOLUME_FADE_SPEED * dt;
            if (currentMusicVolume > targetVolume) currentMusicVolume = targetVolume;
        }
        else if (currentMusicVolume > targetVolume) {
            currentMusicVolume -= VOLUME_FADE_SPEED * dt * 2.0f;
            if (currentMusicVolume < targetVolume) currentMusicVolume = targetVolume;
        }

        set_music_volume(currentMusicVolume);

        // --- Event Polling ---
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed) window.close();
            if (ev.type == Event::Resized) {
                // Pass the enlarged View dimensions to keep aspect ratio correct
                update_letterbox_view(window, view, ev.size.width, ev.size.height, totalViewW, totalViewH);
            }
            if (ev.type == Event::KeyPressed) {
                if (ev.key.code == Keyboard::Escape || ev.key.code == Keyboard::P) {
                    if (!inMenu) {
                        if (gameOver || gameWon) {
                            inMenu = true;
                            paused = false;
                            gameOver = false;
                            gameWon = false;
                            scoreSaved = false;
                        }
                        else {
                            paused = !paused;
                        }
                    }
                }
            }
        }

        // --- Logic Updates ---
        if (inMenu) {
            update_menu(window);
        }
        else {
            if (paused) {
                handle_pause_input(window);
            }
            else if (!gameOver && !gameWon) {
                update_tanks(dt, PLAY_AREA_W, PLAY_AREA_H, PLAY_AREA_TOP);
                update_bullets(dt, PLAY_AREA_W, PLAY_AREA_H, PLAY_AREA_TOP);
                update_powerups(dt);

                int enemyCount = 0;
                for (int i = 0; i < MAX_TANKS; ++i) if (tank_active[i] && !tank_isPlayer[i]) enemyCount++;
                if (enemyCount < 3) {
                    spawn_enemy(100 + rand() % 1000, (float)(rand() % (int)PLAY_AREA_W), 100.0f, 1 + rand() % 3);
                }

                if (!gameOver && enemiesDestroyed >= killTarget) {
                    play_next_level_sfx();
                    currentLevel++;

                    if (currentLevel > 10) {
                        gameWon = true;
                        currentLevel--;
                    }
                    else {
                        enemiesDestroyed = 0;
                        killTarget += 2;

                        string nextLevelFile = "data/levels/level" + to_string(currentLevel) + ".txt";
                        if (!load_level(nextLevelFile)) {
                            cerr << "[INFO] Failed to load next level: " << nextLevelFile << " - staying on current map." << endl;
                        }

                        for (int bi = 0; bi < MAX_BULLETS; ++bi) bullet_active[bi] = false;

                        for (int ti = 0; ti < MAX_TANKS; ++ti) {
                            if (!tank_isPlayer[ti]) tank_active[ti] = false;
                        }

                        for (int ti = 0; ti < MAX_TANKS; ++ti) {
                            if (tank_active[ti] && tank_isPlayer[ti]) {
                                float playerX = baseRect.left + baseRect.width / 2.0f - tank_width[ti] / 2.0f;
                                float playerY = baseRect.top - tank_height[ti] - 20.0f;
                                tank_x[ti] = playerX; tank_y[ti] = playerY;
                                tank_shape[ti].setPosition(playerX, playerY);
                                break;
                            }
                        }

                        for (int i = 0; i < 3; ++i) {
                            spawn_enemy(200 + rand() % 1000, (float)(rand() % (int)PLAY_AREA_W), 100.0f + i * 30.0f, 1 + rand() % 2);
                        }
                    }
                }
            }
        }

        // --- Rendering ---
        window.clear(Color(10, 10, 10)); // Dark Grey Border Background

        if (inMenu) {
            draw_menu(window);
        }
        else {
            // 1. Draw Green Play Area Background
            RectangleShape playAreaBg(Vector2f(PLAY_AREA_W, PLAY_AREA_H));
            playAreaBg.setFillColor(Color(0, 51, 25)); // Original Field Green
            playAreaBg.setPosition(0, 0);
            window.draw(playAreaBg);

            // 2. Draw The Boundary Line (Outside the play area)
            // Because we zoomed out the view, drawing at -thickness is visible!
            RectangleShape playAreaBorder(Vector2f(PLAY_AREA_W, PLAY_AREA_H));
            playAreaBorder.setFillColor(Color::Transparent); // Hollow
            playAreaBorder.setOutlineColor(Color(100, 100, 120)); // Concrete/Steel border color
            playAreaBorder.setOutlineThickness(VIEW_MARGIN); // Positive grows OUTWARD
            playAreaBorder.setPosition(0, 0);
            window.draw(playAreaBorder);

            // Draw World Elements
            draw_blocks(window);
            draw_powerups(window);

            // Draw Base
            if (baseAlive) {
                if (tex_kurak.getSize().x > 0) {
                    Sprite kurakSprite;
                    kurakSprite.setTexture(tex_kurak);
                    Vector2u texSize = tex_kurak.getSize();
                    if (texSize.x > 0 && texSize.y > 0) {
                        float scaleX = baseRect.width / texSize.x;
                        float scaleY = baseRect.height / texSize.y;
                        kurakSprite.setScale(scaleX, scaleY);
                    }
                    kurakSprite.setPosition(baseRect.left, baseRect.top);
                    window.draw(kurakSprite);
                }
                else {
                    RectangleShape baseShape(Vector2f(baseRect.width, baseRect.height));
                    baseShape.setPosition(baseRect.left, baseRect.top);
                    baseShape.setFillColor(Color(100, 100, 255));
                    baseShape.setOutlineColor(Color(200, 200, 255));
                    baseShape.setOutlineThickness(2.0f);
                    window.draw(baseShape);
                }
            }
            else {
                if (tex_kurakDead.getSize().x > 0) {
                    Sprite kurakDeadSprite;
                    kurakDeadSprite.setTexture(tex_kurakDead);
                    Vector2u texSize = tex_kurakDead.getSize();
                    if (texSize.x > 0 && texSize.y > 0) {
                        float scaleX = baseRect.width / texSize.x;
                        float scaleY = baseRect.height / texSize.y;
                        kurakDeadSprite.setScale(scaleX, scaleY);
                    }
                    kurakDeadSprite.setPosition(baseRect.left, baseRect.top);
                    window.draw(kurakDeadSprite);
                }
                else {
                    RectangleShape baseShape(Vector2f(baseRect.width, baseRect.height));
                    baseShape.setPosition(baseRect.left, baseRect.top);
                    baseShape.setFillColor(Color(60, 60, 60));
                    baseShape.setOutlineColor(Color(100, 100, 100));
                    baseShape.setOutlineThickness(2.0f);
                    window.draw(baseShape);
                }
            }

            draw_tanks(window);
            draw_bullets(window);

            // Draw HUD (Right Panel)
            if (hasHudFont) {
                float hudX = PLAY_AREA_W;
                float hudW = 360.0f;

                int playerTotalLives = 0;
                for (int i = 0; i < MAX_TANKS; ++i) {
                    if (tank_active[i] && tank_isPlayer[i]) {
                        playerTotalLives = tank_totalLives[i];
                        break;
                    }
                }

                int activeEnemies = 0;
                for (int i = 0; i < MAX_TANKS; ++i) {
                    if (tank_active[i] && !tank_isPlayer[i]) activeEnemies++;
                }

                // HUD Background
                RectangleShape rightPanel(Vector2f(hudW, PLAY_AREA_H));
                rightPanel.setPosition(hudX, 0);
                rightPanel.setFillColor(Color(15, 15, 25, 255));
                window.draw(rightPanel);

                // Divider Line
                RectangleShape accentLine(Vector2f(6, PLAY_AREA_H));
                accentLine.setPosition(hudX, 0);
                accentLine.setFillColor(Color(255, 215, 0)); // Gold divider
                window.draw(accentLine);

                float yPos = 20;

                // Score Panel
                RectangleShape scorePanel(Vector2f(hudW - 40, 80));
                scorePanel.setPosition(hudX + 20, yPos);
                scorePanel.setFillColor(Color(40, 40, 60, 200));
                scorePanel.setOutlineColor(Color(100, 100, 150, 150));
                scorePanel.setOutlineThickness(2.0f);
                window.draw(scorePanel);

                Text scoreLabel;
                scoreLabel.setFont(hudFont);
                scoreLabel.setCharacterSize(16);
                scoreLabel.setFillColor(Color(180, 180, 200));
                scoreLabel.setString("SCORE");
                scoreLabel.setPosition(hudX + 35, yPos + 15);
                window.draw(scoreLabel);

                Text scoreValue;
                scoreValue.setFont(hudFont);
                scoreValue.setCharacterSize(28);
                scoreValue.setFillColor(Color(255, 215, 0));
                scoreValue.setStyle(Text::Bold);
                scoreValue.setString(to_string(score));
                scoreValue.setPosition(hudX + 35, yPos + 40);
                window.draw(scoreValue);

                yPos += 100;

                // Lives Panel
                RectangleShape livesPanel(Vector2f(hudW - 40, 100));
                livesPanel.setPosition(hudX + 20, yPos);
                livesPanel.setFillColor(Color(60, 40, 40, 200));
                livesPanel.setOutlineColor(Color(150, 100, 100, 150));
                livesPanel.setOutlineThickness(2.0f);
                window.draw(livesPanel);

                Text livesLabel;
                livesLabel.setFont(hudFont);
                livesLabel.setCharacterSize(16);
                livesLabel.setFillColor(Color(200, 180, 180));
                livesLabel.setString("LIVES REMAINING");
                livesLabel.setPosition(hudX + 35, yPos + 15);
                window.draw(livesLabel);

                for (int i = 0; i < playerTotalLives && i < 5; ++i) {
                    CircleShape heart(12, 3);
                    heart.setPosition(hudX + 35 + i * 50, yPos + 55);
                    heart.setFillColor(Color(255, 80, 80));
                    window.draw(heart);
                }

                yPos += 120;

                // Level Panel
                RectangleShape levelPanel(Vector2f(hudW - 40, 80));
                levelPanel.setPosition(hudX + 20, yPos);
                levelPanel.setFillColor(Color(40, 60, 40, 200));
                levelPanel.setOutlineColor(Color(100, 150, 100, 150));
                levelPanel.setOutlineThickness(2.0f);
                window.draw(levelPanel);

                Text levelLabel;
                levelLabel.setFont(hudFont);
                levelLabel.setCharacterSize(16);
                levelLabel.setFillColor(Color(180, 200, 180));
                levelLabel.setString("LEVEL");
                levelLabel.setPosition(hudX + 35, yPos + 15);
                window.draw(levelLabel);

                Text levelValue;
                levelValue.setFont(hudFont);
                levelValue.setCharacterSize(32);
                levelValue.setFillColor(Color(100, 255, 100));
                levelValue.setStyle(Text::Bold);
                levelValue.setString(to_string(currentLevel));
                levelValue.setPosition(hudX + 35, yPos + 40);
                window.draw(levelValue);

                yPos += 100;

                // Enemies Panel
                RectangleShape enemiesPanel(Vector2f(hudW - 40, 120));
                enemiesPanel.setPosition(hudX + 20, yPos);
                enemiesPanel.setFillColor(Color(60, 40, 40, 200));
                enemiesPanel.setOutlineColor(Color(150, 100, 100, 150));
                enemiesPanel.setOutlineThickness(2.0f);
                window.draw(enemiesPanel);

                Text enemiesLabel;
                enemiesLabel.setFont(hudFont);
                enemiesLabel.setCharacterSize(16);
                enemiesLabel.setFillColor(Color(200, 180, 180));
                enemiesLabel.setString("ENEMIES");
                enemiesLabel.setPosition(hudX + 35, yPos + 15);
                window.draw(enemiesLabel);

                Text activeLabel;
                activeLabel.setFont(hudFont);
                activeLabel.setCharacterSize(14);
                activeLabel.setFillColor(Color(255, 150, 150));
                activeLabel.setString("Active: " + to_string(activeEnemies));
                activeLabel.setPosition(hudX + 35, yPos + 50);
                window.draw(activeLabel);

                Text killedLabel;
                killedLabel.setFont(hudFont);
                killedLabel.setCharacterSize(14);
                killedLabel.setFillColor(Color(150, 255, 150));
                killedLabel.setString("Killed: " + to_string(enemiesDestroyed) + " / " + to_string(killTarget));
                killedLabel.setPosition(hudX + 35, yPos + 80);
                window.draw(killedLabel);
            }

            // Draw Overlays (Pause, Win, Loss)
            if (paused) {
                draw_pause_menu(window);
            }

            if (gameWon) {
                if (!scoreSaved) {
                    save_high_score(score);
                    scoreSaved = true;
                }

                if (hasHudFont) {
                    RectangleShape overlay(Vector2f(PLAY_AREA_W, PLAY_AREA_H));
                    overlay.setPosition(0, 0);
                    overlay.setFillColor(Color(0, 0, 0, 200));
                    window.draw(overlay);

                    Text winText;
                    winText.setFont(hudFont);
                    winText.setCharacterSize(72);
                    winText.setFillColor(Color(50, 255, 50));
                    winText.setOutlineColor(Color::White);
                    winText.setOutlineThickness(3.0f);
                    winText.setStyle(Text::Bold);
                    winText.setString("VICTORY!");

                    FloatRect textBounds = winText.getLocalBounds();
                    winText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
                    winText.setPosition(PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f - 80);
                    window.draw(winText);

                    Text scoreText;
                    scoreText.setFont(hudFont);
                    scoreText.setCharacterSize(36);
                    scoreText.setFillColor(Color(255, 215, 0));
                    scoreText.setOutlineColor(Color::Black);
                    scoreText.setOutlineThickness(2.0f);
                    scoreText.setString("Final Score: " + to_string(score));

                    FloatRect scoreBounds = scoreText.getLocalBounds();
                    scoreText.setOrigin(scoreBounds.width / 2.0f, scoreBounds.height / 2.0f);
                    scoreText.setPosition(PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f);
                    window.draw(scoreText);

                    Text instructText;
                    instructText.setFont(hudFont);
                    instructText.setCharacterSize(24);
                    instructText.setFillColor(Color::White);
                    instructText.setOutlineColor(Color::Black);
                    instructText.setOutlineThickness(1.5f);
                    instructText.setString("Press ESC to return to menu");

                    FloatRect instructBounds = instructText.getLocalBounds();
                    instructText.setOrigin(instructBounds.width / 2.0f, instructBounds.height / 2.0f);
                    instructText.setPosition(PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f + 80);
                    window.draw(instructText);
                }
            }

            else if (gameOver) {
                if (!scoreSaved) {
                    save_high_score(score);
                    scoreSaved = true;
                }

                if (hasHudFont) {
                    RectangleShape overlay(Vector2f(PLAY_AREA_W, PLAY_AREA_H));
                    overlay.setPosition(0, 0);
                    overlay.setFillColor(Color(0, 0, 0, 200));
                    window.draw(overlay);

                    Text gameOverText;
                    gameOverText.setFont(hudFont);
                    gameOverText.setCharacterSize(72);
                    gameOverText.setFillColor(Color(255, 50, 50));
                    gameOverText.setOutlineColor(Color::Black);
                    gameOverText.setOutlineThickness(3.0f);
                    gameOverText.setStyle(Text::Bold);
                    gameOverText.setString("GAME OVER");

                    FloatRect textBounds = gameOverText.getLocalBounds();
                    gameOverText.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
                    gameOverText.setPosition(PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f - 80);
                    window.draw(gameOverText);

                    Text scoreText;
                    scoreText.setFont(hudFont);
                    scoreText.setCharacterSize(36);
                    scoreText.setFillColor(Color(255, 215, 0));
                    scoreText.setOutlineColor(Color::Black);
                    scoreText.setOutlineThickness(2.0f);
                    scoreText.setString("Final Score: " + to_string(score));

                    FloatRect scoreBounds = scoreText.getLocalBounds();
                    scoreText.setOrigin(scoreBounds.width / 2.0f, scoreBounds.height / 2.0f);
                    scoreText.setPosition(PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f);
                    window.draw(scoreText);

                    Text instructText;
                    instructText.setFont(hudFont);
                    instructText.setCharacterSize(24);
                    instructText.setFillColor(Color::White);
                    instructText.setOutlineColor(Color::Black);
                    instructText.setOutlineThickness(1.5f);
                    instructText.setString("Press ESC to return to menu");

                    FloatRect instructBounds = instructText.getLocalBounds();
                    instructText.setOrigin(instructBounds.width / 2.0f, instructBounds.height / 2.0f);
                    instructText.setPosition(PLAY_AREA_W / 2.0f, PLAY_AREA_H / 2.0f + 80);
                    window.draw(instructText);
                }
            }
        }

        window.display();
    }

    return 0;
}