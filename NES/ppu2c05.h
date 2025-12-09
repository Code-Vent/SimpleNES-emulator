#include<stdint.h>

#ifndef PPU2C05_H
#define PPU2C05_H

//#include"nametable.h"
#include"cartridge.h"
#include"cpu6502.h"
#include"sprite.h"
#include"backgroung.h"

#include"raylib/raylib.h"


class PPURegisters;

enum class Event {
	READ_STATUS,
	WRITE_SCROLL
};

class PPU2C05 {
	friend class PPURegisters;
public:
	PPU2C05() = default;
	PPU2C05(Cartridge* cart, CPU6502* cpu = nullptr);
	PPU2C05& operator= (const PPU2C05&) = default;
	PPU2C05& operator= (PPU2C05&&) noexcept;
	~PPU2C05();
	void init();
	void render();
	void post_render();
	void pre_render();
	void state_manager(Event e, uint8_t* status = nullptr);
private:
	Image     image;
	Texture2D tex;
	Color     (*pixels)[240][256];
	uint8_t   regs[0x0008];
	uint8_t   palette[0x0020];
	Sprites   sprites;
	uint16_t  internal_t;
	uint16_t  internal_v;
	uint8_t   internal_x;
	uint8_t   internal_w;
	uint8_t   vram_inc;
	Cartridge* cart;
	CPU6502*   cpu;
	uint8_t   even_or_odd;
	void    update_image();
	void    draw() const;
	int     scanline;
	int     cycle;
	bool    nmi_enabled;
public:
	void    idle_line(int count = 1, int cycles = 113);
};

#endif