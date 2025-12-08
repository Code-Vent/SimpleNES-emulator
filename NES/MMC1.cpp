#include "MMC1.h"
#include<assert.h>
#include<iostream>
#include "AxROM.h"

using namespace std;

MMC1::MMC1(std::string filename)
	:Cartridge(filename), horizontal(nullptr), 
	vertical(nullptr), single_screen(nullptr)
{
	regs[control_reg] = 0x0C;
	regs[chr0_reg] = 0;
	regs[chr1_reg] = 0;
	regs[prg_reg] = 0;

	shift_reg = 0b10;
	horizontal = new HorizontalInterface(&data, this);
	vertical = new VerticalInterface(&data, this);
	single_screen = new SingleScreenInterface(&data, this);
	
	apply_settings();
}

MMC1::~MMC1()
{
	if (horizontal != nullptr)delete horizontal;
	if (vertical != nullptr)delete vertical;
	if (single_screen != nullptr)delete single_screen;
}

void MMC1::write(uint16_t address, uint8_t data)
{
	if (address >= 0x8000 && address <= 0xFFFF) {
		auto reg_sel = (address & 0x6000) >> 13;
		if ((0x80 & data) != 0) {
			reset(data);
		}
		else {
			shift_load(data, reg_sel);
		}
	}
	else {
		Cartridge::write(address, data);
	}	
}

bool MMC1::good()
{
	return ok && id == 1;
}

NametableInterface* MMC1::get_nametable_interface()
{
	assert(inametable != nullptr);
	return inametable;
}

void MMC1::apply_settings()
{
	int bank = 0, bank0 = 0, bank1 = 0;
	int offset = 0, offset0 = 0, offset1 = 0;
	auto control = regs[control_reg];
	//Select Nametable
	switch (control & 0x03) {
	case 0:
		reinterpret_cast<SingleScreenInterface*>(single_screen)->set_table(0);
		inametable = single_screen;
		break;
	case 1:
		reinterpret_cast<SingleScreenInterface*>(single_screen)->set_table(1);
		inametable = single_screen;
		break;
	case 2:
		inametable = vertical;
		break;
	case 3:
		inametable = horizontal;
		break;
	default:
		break;
	}
	//Set PRG Bank
	switch (control & 0x0C) {
	case 0: case 4:
		//switch 32K at 0x8000
		bank = regs[prg_reg] & 0x0E;
		offset = bank * 0x8000;
		prg_bank0 = &reinterpret_cast<uint8_t*>(prg)[offset];
		prg_bank1 = &reinterpret_cast<uint8_t*>(prg)[offset + 16384];
		break;
	case 8:
		//Fix first bank at 0x8000 and switch 16K at 0xC000
		bank = regs[prg_reg] & 0x0F;
		offset = bank * 0x4000;
		prg_bank0 = &reinterpret_cast<uint8_t*>(prg)[0];
		prg_bank1 = &reinterpret_cast<uint8_t*>(prg)[offset];
		break;
	case 12:
		//Fix last bank at 0xC000 and switch 16K at 0x8000
		bank = regs[prg_reg] & 0x0F;
		offset0 = bank * 0x4000;
		offset1 = (prg_banks - 1) * 0x4000;
		prg_bank0 = &reinterpret_cast<uint8_t*>(prg)[offset0];
		prg_bank1 = &reinterpret_cast<uint8_t*>(prg)[offset1];
		break;
	default:
		break;
	}
	//Select CHR Bank
	switch (control & 0x10) {
	case 0:
		//Switch 8K
		bank = regs[chr0_reg] & 0x1E;
		offset = bank * 0x2000;
		pattern_tbl0 = &reinterpret_cast<uint8_t*>(chr)[offset];
		pattern_tbl1 = &reinterpret_cast<uint8_t*>(chr)[offset + 0x1000];
		break;
	case 16:
		bank0 = regs[chr0_reg] & 0x1F;
		bank1 = regs[chr1_reg] & 0x1F;
		offset0 = bank0 * 0x1000;
		offset1 = bank1 * 0x1000;
		pattern_tbl0 = &reinterpret_cast<uint8_t*>(chr)[offset0];
		pattern_tbl1 = &reinterpret_cast<uint8_t*>(chr)[offset1];
		break;
	default:
		break;
	}
}

void MMC1::reset(uint8_t data)
{
	shift_pos = 0;
	shift_reg = 0;
	regs[control_reg] = 0x0C;
	apply_settings();
}

void MMC1::shift_load(uint8_t data, uint8_t sel)
{
	shift_reg |= ((data & 0x01) << shift_pos);
	shift_pos += 1;
	if (shift_pos == 5) {
		regs[sel] = shift_reg & 0x1F;
		apply_settings();
		shift_pos = 0;
		shift_reg = 0;
	}
}
