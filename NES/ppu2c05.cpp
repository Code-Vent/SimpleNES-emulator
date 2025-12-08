#include"ppu2c05.h"
#include"raylib/raylib.h"
#include<assert.h>
#include<iostream>
#include"nes_pal.h"

using namespace std;

constexpr uint8_t SPRITE_0_HIT = 0x40;
constexpr uint8_t SPRITE_OVERFFLOW = 0x20;
constexpr uint8_t VBLANK = 0x80;


const uint8_t PALETTE[] =  {
		0x0F, 0x31, 0x32, 0x33,
		0x0F, 0x35, 0x36, 0x37,
		0x0F, 0x39, 0x3A, 0x3B,
		0x0F, 0x3D, 0x3E, 0x3F,
		//////////////////////////
		0x0F, 0x1C, 0x15, 0x14,
		0x0F, 0x02, 0x38, 0x3C,
		0x0F, 0x1C, 0x15, 0x14,
		0x0F, 0x02, 0x38, 0x3C,
};

class PPUBus {
public:
	static PPU2C05* ppu;
	static uint8_t read(uint16_t address);
	static void    write(uint16_t address, uint8_t data);
};

PPU2C05::PPU2C05(Cartridge* cart, CPU6502* cpu)
	:pixels(nullptr), regs{ 0 }, sprites(cart)
{
	assert(cart != nullptr);
	scanline = 0;
	cycle = 0;
	
	this->cart = cart;
	internal_t = 0;
	internal_v = 0;
	internal_w = 0;
	internal_x = 0;

	memcpy_s(&palette, sizeof(palette), &PALETTE, sizeof(palette));
	
	pixels = reinterpret_cast<Color(*)[240][256]>(new Color[240 * 256]);
	image = Image{
		pixels,
		256,
		240,
		1,
		PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
	};

	if (cpu != nullptr) {
		this->cpu = cpu;		
		cpu->ppu_read = &PPUBus::read;
		cpu->ppu_write = &PPUBus::write;
		PPUBus::ppu = this;
		cpu->init(cart);		
	}
}

PPU2C05& PPU2C05::operator=(PPU2C05&& ppu) noexcept
{
	*this = ppu;
	ppu.pixels = nullptr;
	PPUBus::ppu = this;
	return *this;
}

PPU2C05::~PPU2C05()
{
	if (pixels != nullptr) {
		delete [] pixels;
		UnloadTexture(tex);
		UnloadImage(image);
	}
}

void PPU2C05::init()
{
	tex = LoadTextureFromImage(image);
}

void PPU2C05::render()
{
	update_image();
	BeginDrawing();
	UpdateTexture(tex, pixels);
	draw();
	EndDrawing();
}

void PPU2C05::post_render()
{
	idle_line();
	//Call NMI if enabled
	if (nmi_enabled) {
		cpu->nmi();
	}
	if (sprites.dma_enabled) {
		assert(cpu != nullptr);
		cpu->dma(sprites.oam, sprites.dma_src_address, sizeof(sprites.oam));
		sprites.dma_enabled = false;
	}
	//VBlank duration Adjustment based on even or odd frame
	if ((even_or_odd & 0x01) == 0) {
		idle_line(20);
	}
	else {
		idle_line(19);
		idle_line(1, 110);
	}
	even_or_odd += 1;
}

void PPU2C05::pre_render()
{
	scanline = -1;
	idle_line();
	//Resets table offset if a hit occurred in last frame
	cart->get_nametable_interface()->reset(sprites.hit_point > -1);	
	sprites.reset();                        
}

void PPU2C05::state_manager(Event e, uint8_t* status)
{	
	switch (e) {
	case Event::READ_STATUS:
		assert(status != nullptr);
		if (scanline >= 0 && scanline < 240) {
			int hit_cycle = sprites.hit_point / 3;
			if (sprites.hit_point > -1 && cpu->cycles >= (hit_cycle)) {
				*status |= SPRITE_0_HIT;
			}
			if (sprites.sprite_count > 8) {
				*status |= SPRITE_OVERFFLOW;
			}
		}
		if (scanline > 240){
			//Inside NMI Handler
			*status |= VBLANK;
			//SMB needs this line to work properly
			cart->get_nametable_interface()->set_base_address(0);
		}
		else {
		}
		break;
	case Event::WRITE_SCROLL:
		cart->get_nametable_interface()->begin();
	default:
		break;
	}
}

void PPU2C05::update_image()
{
	static uint8_t priority[256] = { 0 };
	uint8_t(*pal)[2][4][4] = reinterpret_cast<uint8_t(*)[2][4][4]>(palette);
	
	while (scanline < 240) {
		memset(&priority, 0, sizeof(priority));
		cart->get_nametable_interface()->begin();
		cart->get_nametable_interface()->draw(&(*pal)[0], scanline, &(*pixels)[scanline], &priority);
		sprites.draw(&(*pal)[1], scanline, &(*pixels)[scanline] , &priority);
		idle_line();
	}
}

void PPU2C05::draw() const
{
	Rectangle src = Rectangle{ 0, 0, 256, 240 };
	Rectangle dest = Rectangle{ 0, 0, 800, 600 };

	DrawTexturePro(tex, src, dest, Vector2{ 0,0 }, 0, WHITE);		
}

void PPU2C05::idle_line(int count, int cpu_cycles)
{
	do {
		cpu->execute(cpu_cycles);
	} while (--count);
	scanline += 1;
}

PPU2C05* PPUBus::ppu = nullptr;

void PPUBus::write(uint16_t address, uint8_t data) {
	assert(ppu != nullptr);
	uint16_t temp = 0;
	uint16_t temp2 = 0;
	switch (address) {
	case 0x2000:
		//Control bits
		ppu->cart->get_nametable_interface()->set_base_address(data & 0x03);		
		ppu->vram_inc = ((data & 0x04) == 0) ? 1 : 32;
		ppu->sprites.tbl = ((data & 0x08) == 0) ? SpriteTable::TABLE_0000 : SpriteTable::TABLE_1000;
		ppu->cart->get_nametable_interface()->tbl = ((data & 0x10) == 0) ? BackgroundTable::TABLE_0000 : BackgroundTable::TABLE_1000;
		ppu->sprites.mode_8x8 = (data & 0x20) == 0;
		ppu->nmi_enabled = (data & 0x80) != 0;
		ppu->internal_t = (ppu->internal_t & 0xF3FF) | (uint16_t(data & 0x03) << 10);
		break;
	case 0x2001:
		ppu->cart->get_nametable_interface()->show_leftmost = (data & 0x02) != 0;
		ppu->sprites.show_leftmost = (data & 0x04) != 0;
		ppu->cart->get_nametable_interface()->enabled = (data & 0x08) != 0;
		ppu->sprites.enabled = (data & 0x10) != 0;
		//TODO
		//Grayscale, Emphasize Red, Emphasize Green, Emphasize Blue
		break;
	case 0x2003:
		ppu->sprites.oam_address = data;
		break;
	case 0x2004:
		ppu->sprites.oam[ppu->sprites.oam_address] = data;
		ppu->sprites.oam_address += 1;
		break;
	case 0x2005:
		if ((ppu->internal_w & 0x01) == 0) {
			temp = (uint16_t(data) & 0xF8) >> 3;
			ppu->internal_t = (ppu->internal_t & 0xFFE0) | temp;
			ppu->internal_x = data & 0x07;
			ppu->internal_w = 1;
		}
		else {
			temp = uint16_t(data & 0x07) << 12;
			temp2 = uint16_t(data & 0xF8) << 2;
			ppu->internal_t = (ppu->internal_t & 0x8C1F) | (temp2 | temp);
			ppu->internal_w = 0;
			ppu->cart->get_nametable_interface()->set_scroll(ppu->internal_t, ppu->internal_x);
			ppu->state_manager(Event::WRITE_SCROLL);
		}
		break;
	case 0x2006:
		if ((ppu->internal_w & 0x01) == 0) {
			temp = uint16_t(data & 0x3F) << 8;
			ppu->internal_t = (ppu->internal_t & 0x00FF) | temp;
			ppu->internal_w = 1;
		}
		else {
			ppu->internal_t = (ppu->internal_t & 0xFF00) | uint16_t(data);
			ppu->internal_v = ppu->internal_t;
			ppu->internal_w = 0;
		}
		break;
	case 0x2007:
		if (ppu->internal_v >= 0x0000 && ppu->internal_v < 0x2000) {
			//Pattern Tables
			ppu->cart->write(ppu->internal_v, data);
		}
		else if (ppu->internal_v >= 0x2000 && ppu->internal_v < 0x3000) {
			//Nametables
			ppu->cart->get_nametable_interface()->write(ppu->internal_v, data);
		}
		else if (ppu->internal_v >= 0x3F00 && ppu->internal_v < 0x3FFF) {
			//Palette
			auto addr = ppu->internal_v & 0x001F;
			if (addr == 0x0010) addr = 0x0000;
			if (addr == 0x0014) addr = 0x0004;
			if (addr == 0x0018) addr = 0x0008;
			if (addr == 0x001C) addr = 0x000C;
			ppu->palette[addr] = data;
		}
		else {
			assert(false);
		}
		ppu->internal_v += ppu->vram_inc;
		break;
	case 0x4014:
		ppu->sprites.dma_enabled = true;
		ppu->sprites.dma_src_address = uint16_t(data) << 8;
		break;
	default:
		break;
	}
	return;
}

uint8_t PPUBus::read(uint16_t address)
{
	assert(ppu != nullptr);
	static uint8_t read_buffer = 0;
	uint16_t temp = 0;
	uint8_t data = 0;
	switch (address) {
	case 0x2002://PPUSTATUS
		ppu->internal_w = 0;
		ppu->state_manager(Event::READ_STATUS, &data);
		break;
	case 0x2004://OAM DATA
		data = ppu->sprites.oam[ppu->sprites.oam_address];
		break;
	case 0x2007://PPU DATA
		data = read_buffer;
		if (ppu->internal_v >= 0x0000 && ppu->internal_v < 0x2000) {
			//Pattern Tables
			read_buffer = ppu->cart->read(ppu->internal_v);
		}
		else if (ppu->internal_v >= 0x2000 && ppu->internal_v < 0x3000) {
			//Nametables
			read_buffer = ppu->cart->get_nametable_interface()->read(ppu->internal_v);
		}
		else if (ppu->internal_v >= 0x3F00 && ppu->internal_v <= 0x3FFF) {
			//Palette
			auto addr = ppu->internal_v & 0x001F;
			if (addr == 0x0010) addr = 0x0000;
			if (addr == 0x0014) addr = 0x0004;
			if (addr == 0x0018) addr = 0x0008;
			if (addr == 0x001C) addr = 0x000C;
			data = ppu->palette[addr];
		}
		else {
			assert(false);
		}
		ppu->internal_v += ppu->vram_inc;
		break;
	default:
		break;
	}
	return data;
}