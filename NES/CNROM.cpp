#include "CNROM.h"
#include<assert.h>
#include<iostream>

using namespace std;

CNROM::CNROM(std::string filename)
	:Cartridge(filename), data{},inametable(nullptr)
{
	pattern_tbl0 = chr;
	pattern_tbl1 = &reinterpret_cast<uint8_t*>(chr)[4096];

	if (prg_banks == 2) {
		prg_bank0 = prg;
		prg_bank1 = &reinterpret_cast<uint8_t*>(prg)[16384];
	}
	else if (prg_banks == 1) {
		prg_bank0 = prg;
		prg_bank1 = prg;
	}
	else {
		ok = false;
	}
	if (mirroring == Mirroring::VERTICAL) {
		inametable = new VerticalInterface(&data, this);
	}
	else if (mirroring == Mirroring::HORIZONTAL) {
		inametable = new HorizontalInterface(&data, this);
	}
}

CNROM::~CNROM()
{
	if (inametable != nullptr) {
		delete inametable;
	}
}

void CNROM::write(uint16_t address, uint8_t data)
{
	if (address >= 0x8000 && address <= 0xFFFF) {
		uint8_t value = read(address);
		value &= data;
		int active_chr_bank;
		if (chr_banks == 0) {
			active_chr_bank = 0;
		}
		else {
			// use mask if power-of-two banks, else modulo
			unsigned mask = 1;
			while (mask < chr_banks) mask <<= 1;
			if ((mask == chr_banks) && ((chr_banks & (chr_banks - 1)) == 0)) {
				active_chr_bank = value & (chr_banks - 1);
			}
			else {
				active_chr_bank = value % chr_banks;
			}
		}
		auto bank_offset = active_chr_bank * 8192;
		pattern_tbl0 = &reinterpret_cast<uint8_t*>(chr)[bank_offset];
		pattern_tbl1 = &reinterpret_cast<uint8_t*>(chr)[bank_offset + 4096];
	}
	else {
		Cartridge::write(address, data);
	}
}

bool CNROM::good()
{
	return ok && id == 3;
}

NametableInterface* CNROM::get_nametable_interface()
{
	return inametable;
}
