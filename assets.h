#ifndef assets_h
#define assets_h
#include <SFML/Graphics.hpp>
#include <string>
using namespace std;
using namespace sf;

namespace assets {

	extern bool loaded;

	extern Texture tex_bullet;
	extern Texture tex_tankGreen;
	extern Texture tex_tankRed;
	extern Texture tex_tankYellow;
	extern Texture tex_tankGrey;
	extern Texture tex_kurak;
	extern Texture tex_kurakDead;
	extern Texture tex_air;
	extern Texture tex_brick;
	extern Texture tex_stone;

	// prototype sprites
	extern Sprite proto_bullet;
	extern Sprite proto_tankGreen;
	extern Sprite proto_tankRed;
	extern Sprite proto_tankYellow;
	extern Sprite proto_tankGrey;
	extern Sprite proto_block_brick;
	extern Sprite proto_block_stone;
	extern Sprite proto_block_air;
	extern Sprite proto_kurak;

	// Load all assets. Returns true if main tank textures loaded (same semantics as original).
	bool loadAll();

	// Helpers for loading resources using relative search paths
	bool tryLoadFont(Font& font, const string& relPath);

} 

#endif
