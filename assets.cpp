#include "assets.h"
#include <iostream>
#include <string>
using namespace std;
using namespace sf;

namespace assets {

    // Global Initialization Flag
    bool loaded = false;

    // Texture Storage
    Texture tex_bullet;
    Texture tex_tankGreen;
    Texture tex_tankRed;
    Texture tex_tankYellow;
    Texture tex_tankGrey;
    Texture tex_kurak;      // The Base (Eagle)
    Texture tex_kurakDead;  // Destroyed Base
    Texture tex_air;        // Background Tile
    Texture tex_brick;      // Destructible Wall
    Texture tex_stone;      // Indestructible Wall

    // Sprite Prototypes
    Sprite proto_bullet;
    Sprite proto_tankGreen;
    Sprite proto_tankRed;
    Sprite proto_tankYellow;
    Sprite proto_tankGrey;
    Sprite proto_block_brick;
    Sprite proto_block_stone;
    Sprite proto_block_air;
    Sprite proto_kurak;

    // Direct Texture Loader
    // Loads an image from a specific path. Returns true on success.
    bool loadTexture(Texture& tx, const string& path) {
        if (tx.loadFromFile(path)) {
            cout << "[Assets] Loaded: " << path << endl;
            return true;
        }
        // Failure is logged but does not crash the application immediately
        cerr << "[Assets] ERROR: Failed to load " << path << endl;
        return false;
    }

    // Direct Font Loader
    bool tryLoadFont(Font& font, const string& path) {
        if (font.loadFromFile(path)) {
            cout << "[Assets] Loaded font: " << path << endl;
            return true;
        }
        cerr << "[Assets] ERROR: Failed to load font " << path << endl;
        return false;
    }

    // Main Asset Loading Routine
    bool loadAll() {
        cout << "--- Initializing Assets ---" << endl;
        bool success = true;

        // Load Projectiles
        if (!loadTexture(tex_bullet, "data/bulletTx.gif")) success = false;

        // Load Tanks
        if (!loadTexture(tex_tankGreen, "data/tanks/tankGreenTx.png")) success = false;
        if (!loadTexture(tex_tankRed, "data/tanks/tankRedTx.png")) success = false;
        if (!loadTexture(tex_tankYellow, "data/tanks/tankYellowTx.png")) success = false;

        // Load Grey tank (Check for common capitalization typo)
        if (!loadTexture(tex_tankGrey, "data/tanks/tankGreyTx.png")) {
            // Fallback: Try lowercase 'tx'
            loadTexture(tex_tankGrey, "data/tanks/tankGreytx.png");
        }

        // Load Environment

        // Air/Background (Try modern name, fallback to legacy name)
        if (!loadTexture(tex_air, "data/blocks/airTx.png")) {
            loadTexture(tex_air, "data/blocks/greenTx.png");
        }

        // Brick Wall
        if (!loadTexture(tex_brick, "data/blocks/brickTx.png")) success = false;

        // Stone Wall (Try standard, fallback to 'Big' variant)
        if (!loadTexture(tex_stone, "data/blocks/stoneTx.png")) {
            loadTexture(tex_stone, "data/blocks/stoneBigTx.png");
        }

        // Load Base (Eagle)
        if (!loadTexture(tex_kurak, "data/kurak/kurakTx.png")) success = false;
        if (!loadTexture(tex_kurakDead, "data/kurak/kurakDeadTx.png")) success = false;

        // Texture Configuration
        // We disable smoothing to maintain the retro "Pixel Art" aesthetic.
        if (tex_bullet.getSize().x > 0) tex_bullet.setSmooth(false);
        if (tex_tankGreen.getSize().x > 0) tex_tankGreen.setSmooth(false);
        if (tex_tankRed.getSize().x > 0) tex_tankRed.setSmooth(false);
        if (tex_tankGrey.getSize().x > 0) tex_tankGrey.setSmooth(false);
        if (tex_tankYellow.getSize().x > 0) tex_tankYellow.setSmooth(false);
        if (tex_air.getSize().x > 0) tex_air.setSmooth(false);
        if (tex_brick.getSize().x > 0) tex_brick.setSmooth(false);
        if (tex_stone.getSize().x > 0) tex_stone.setSmooth(false);
        if (tex_kurak.getSize().x > 0) tex_kurak.setSmooth(false);
        if (tex_kurakDead.getSize().x > 0) tex_kurakDead.setSmooth(false);

        // Sprite Initialization
        // Assigns textures to sprites and centers their origins for rotation.
        if (tex_bullet.getSize().x > 0) {
            proto_bullet.setTexture(tex_bullet);
            proto_bullet.setOrigin(tex_bullet.getSize().x / 2.0f, tex_bullet.getSize().y / 2.0f);
        }
        if (tex_tankGreen.getSize().x > 0) {
            proto_tankGreen.setTexture(tex_tankGreen);
            proto_tankGreen.setOrigin(tex_tankGreen.getSize().x / 2.0f, tex_tankGreen.getSize().y / 2.0f);
        }
        if (tex_tankRed.getSize().x > 0) {
            proto_tankRed.setTexture(tex_tankRed);
            proto_tankRed.setOrigin(tex_tankRed.getSize().x / 2.0f, tex_tankRed.getSize().y / 2.0f);
        }
        if (tex_tankGrey.getSize().x > 0) {
            proto_tankGrey.setTexture(tex_tankGrey);
            proto_tankGrey.setOrigin(tex_tankGrey.getSize().x / 2.0f, tex_tankGrey.getSize().y / 2.0f);
        }
        if (tex_tankYellow.getSize().x > 0) {
            proto_tankYellow.setTexture(tex_tankYellow);
            proto_tankYellow.setOrigin(tex_tankYellow.getSize().x / 2.0f, tex_tankYellow.getSize().y / 2.0f);
        }
        if (tex_brick.getSize().x > 0) {
            proto_block_brick.setTexture(tex_brick);
            proto_block_brick.setOrigin(tex_brick.getSize().x / 2.0f, tex_brick.getSize().y / 2.0f);
        }
        if (tex_stone.getSize().x > 0) {
            proto_block_stone.setTexture(tex_stone);
            proto_block_stone.setOrigin(tex_stone.getSize().x / 2.0f, tex_stone.getSize().y / 2.0f);
        }
        if (tex_air.getSize().x > 0) {
            proto_block_air.setTexture(tex_air);
            proto_block_air.setOrigin(tex_air.getSize().x / 2.0f, tex_air.getSize().y / 2.0f);
        }
        if (tex_kurak.getSize().x > 0) {
            proto_kurak.setTexture(tex_kurak);
            proto_kurak.setOrigin(tex_kurak.getSize().x / 2.0f, tex_kurak.getSize().y / 2.0f);
        }

        loaded = success;
        return loaded;
    }

} // namespace assets