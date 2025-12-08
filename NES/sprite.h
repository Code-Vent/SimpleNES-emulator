#ifndef SPRITE_H
#define SPRITE_H

#include<stdint.h>
#include<raylib/raylib.h>
#include"cartridge.h"

struct Sprite;

enum class SpriteTable {
	TABLE_0000 = 0x0000,
	TABLE_1000 = 0x1000,
};

class Sprites {
public:
	Sprites(Cartridge* cart=nullptr);
	void     draw(uint8_t(*palette)[4][4],
	              int scanline,
		          Color(*pixels)[256],
		          uint8_t (*priority)[256]);
	uint16_t get_pattern(Sprite* sprite, uint8_t fine_y) const;
	void     reset();
	void     begin();
	bool     dma_enabled;
	uint16_t dma_src_address;
	SpriteTable tbl;
	bool     mode_8x8;
	bool     enabled;
	bool     show_leftmost;
	uint8_t  oam_address;
	uint8_t  oam[256];
	uint8_t  sprite_count;
	uint8_t  hit_count;
	int      hit_point;
	Cartridge* cart;
};

#endif