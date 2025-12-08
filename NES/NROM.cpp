#include "NROM.h"
#include<assert.h>
#include<iostream>

using namespace std;

NROM::NROM(std::string filename)
	:Cartridge(filename)
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
	}else if (mirroring == Mirroring::HORIZONTAL) {
		inametable = new HorizontalInterface(&data, this);
	}
}

NROM::~NROM()
{
	if (inametable != nullptr) {
		delete inametable;
	}
}

void NROM::write(uint16_t address, uint8_t data)
{
	Cartridge::write(address, data);
}

bool NROM::good()
{
	return ok && id == 0;
}

NametableInterface* NROM::get_nametable_interface()
{
	return inametable;
}
