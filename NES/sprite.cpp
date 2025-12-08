#include "sprite.h"
#include"nes_pal.h"

#include<assert.h>
#include<iostream>
using namespace std;


static uint8_t blit_bits[256];
static int     hit_pixel_pos = 0;

struct Sprite {
	uint8_t y_pos;
	uint8_t tile_no;
	uint8_t attrib;
	uint8_t x_pos;

	bool visible(bool show_leftmost) const;
	bool on_line(int scanline, uint8_t size, uint8_t* fine_y) const;
	void draw(uint16_t pattern,	uint8_t(*palette)[4][4], 
		Color(*buffer)[256], uint8_t(*priority)[256]) const;
	void blit(Color(*buffer)[256], Color(*row)[8], uint8_t(*priority)[256]) const;
};

Sprites::Sprites(Cartridge* cart)
	:oam{}, oam_address{0}, dma_src_address{0}, cart{cart}
{
	dma_enabled = false;
	enabled = false;
	show_leftmost = false;
	tbl = SpriteTable::TABLE_0000;
	mode_8x8 = true;
	hit_count = 0;
	hit_point = -1;
	sprite_count = 0;
}

void Sprites::draw(
	uint8_t(*palette)[4][4],
	int scanline,
	Color(*pixels)[256],
	uint8_t(*priority)[256]
){
	Sprite* sprite = reinterpret_cast<Sprite*>(oam);
	constexpr int SPRITES_N = sizeof(oam) / sizeof(Sprite);
	uint8_t size = mode_8x8 ? 8 : 16;
	uint16_t pattern = 0;
	uint8_t fine_y = 0;
	
	sprite_count = 0;
	//assert(scanline < 240);
	if (!enabled) {
		return;
	}
	memset(&blit_bits, 1, sizeof(blit_bits));
	for (int i = 0; i < SPRITES_N; ++i) {
		if (sprite->visible(show_leftmost) && sprite->on_line(scanline, size, &fine_y)) {			
			if (sprite_count < 8) {
				pattern = get_pattern(sprite, fine_y);				
				sprite->draw(pattern, palette, pixels, priority);
				if (i == 0 && hit_count < 1 && hit_pixel_pos > -1) {
					hit_count += 1;
					hit_point = hit_pixel_pos;
				}
			}
			sprite_count += 1;
		}
		sprite += 1;
	}
}

uint16_t Sprites::get_pattern(
	Sprite* sprite,
	uint8_t fine_y
) const {
	auto base_addr = uint16_t(tbl);
	int offset = 0;
	if (mode_8x8) {
		offset = 16 * int(sprite->tile_no);
	}
	else {
		base_addr = uint16_t(((sprite->tile_no & 0x01) == 0) ? SpriteTable::TABLE_0000 : SpriteTable::TABLE_1000);
		
		if (fine_y >= 8) {//
			offset = 16 * int(((sprite->tile_no & 0xFE) | 0x01));
		}
		else {
			offset = 16 * int(sprite->tile_no & 0xFE);
		}
	}
	fine_y &= 0x07;
	uint8_t plane0, plane1;
	uint8_t bit_mask = 0x80;
	uint16_t pattern = 0;

	plane0 = cart->read(base_addr + offset + fine_y);
	plane1 = cart->read(base_addr + offset + 8 + fine_y);
	for (int i = 7; i >= 0; --i) {
		pattern |= (uint16_t(bit_mask & plane0) << i);
		pattern |= (uint16_t(bit_mask & plane1) << (i + 1));
		bit_mask >>= 1;
	}
	return pattern;
}

bool Sprite::visible(bool show_leftmost) const {
	if (!show_leftmost && x_pos < 8) {
		return false;
	}
	return (y_pos < 0xEF) && (y_pos > 0x00);
}

bool Sprite::on_line(
	int scanline,
	uint8_t size,
	uint8_t* fine_y
) const {
	int y = y_pos + 1;
	*fine_y = scanline - y;
	if (*fine_y >= size){
		return false;
	}
	//Apply Vertical flip 
	if (attrib & 0x80) {
		*fine_y = size - *fine_y - 1;
	}
	return true;
}

void Sprite::draw(
	uint16_t pattern,
	uint8_t(*palette)[4][4],
	Color(*buffer)[256],
	uint8_t(*priority)[256]
) const {
	Color row[8]{};
	uint8_t pal = attrib & 0x03;
	uint8_t j = 0, k = 7;
	for (int i = 7; i >= 0; --i) {
		uint8_t pixel = (pattern >> (2 * i)) & 0x03;//Pixel value from pattern table
		uint8_t col = (*palette)[pal][pixel];
		Color pixel_color = NES_PAL[col];
		set_transparecy(pixel, &pixel_color);
		if (attrib & 0x40) {//Flip Horizontally
			row[k--] = pixel_color;
		}
		else {
			row[j++] = pixel_color;
		}
	}
	blit(buffer, &row, priority);
}

void Sprites::reset() {
	hit_count = 0;
	hit_point = -1;
}

void Sprites::begin() {
	
}

void Sprite::blit(Color(*buffer)[256], Color(*row)[8], uint8_t(*priority)[256]) const
{
	int fine_x = x_pos;
	hit_pixel_pos = -1;
	for (int i = 0; i < 8 && fine_x < 256; ++i, ++fine_x) {
		auto background = (*buffer)[fine_x];
		auto foreground = (*row)[i];
		if ((*priority)[fine_x] == 0 && foreground.a == 0) {
	
		}
		else if ((*priority)[fine_x] == 0 && foreground.a != 0) {
			(*buffer)[fine_x] = (*row)[i];
		}
		else if ((*priority)[fine_x] != 0 && foreground.a == 0) {

		}
		else if ((*priority)[fine_x] != 0 && foreground.a != 0) {
			hit_pixel_pos = fine_x;
			if ((attrib & 0x20) == 0) {//In front of background
				(*buffer)[fine_x] = (*row)[i];
			}
			else {
			}			
		}		
	}	 
}
