#include"AxROM.h"
#include<assert.h>

AxROM::AxROM(std::string filename)
	:Cartridge(filename), single_screen(nullptr), bank_sel{0}
{
	single_screen = new SingleScreenInterface(&data, this);
	pattern_tbl0 = &reinterpret_cast<uint8_t*>(chr)[0];
	pattern_tbl1 = &reinterpret_cast<uint8_t*>(chr)[0x1000];
	apply_settings();
}

AxROM::~AxROM()
{
	if (single_screen != nullptr)delete single_screen;
}

void AxROM::write(uint16_t address, uint8_t data)
{
	if (address >= 0x8000 && address <= 0xFFFF) {
		auto bus_conflict = read(address);
		bank_sel = data & bus_conflict;
		apply_settings();
	}
	else {
		Cartridge::write(address, data);
	}
}

bool AxROM::good()
{
	return ok && id == 7;
}

NametableInterface* AxROM::get_nametable_interface()
{
	return single_screen;
}

void AxROM::apply_settings() {
	//switch 32K at 0x8000
	assert((bank_sel & 0x07) < prg_banks);
	auto offset = (bank_sel & 0x07) * 0x8000;
	prg_bank0 = &reinterpret_cast<uint8_t*>(prg)[offset];
	prg_bank1 = &reinterpret_cast<uint8_t*>(prg)[offset + 16384];
	auto sel = (0x10 & bank_sel) >> 4;
	reinterpret_cast<SingleScreenInterface*>(single_screen)->set_table(sel);
}
