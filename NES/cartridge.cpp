#include"cartridge.h"
#include<iostream>
#include<fstream>

using namespace std;

Cartridge::Cartridge(std::string filename)
	:prg(nullptr), chr(nullptr),
	prg_bank0(nullptr), pattern_tbl0(nullptr),
	prg_bank1(nullptr), pattern_tbl1(nullptr),
	chr_banks(0), prg_banks(0), mirroring(Mirroring::VERTICAL)
{
	id = 0;
	// iNES Format Header
	sHeader header{};

	ok = false;

	std::ifstream ifs;
	ifs.open(filename, std::ifstream::binary);
	if (ifs.is_open())
	{
		// Read file header
		ifs.read((char*)&header, sizeof(sHeader));

		// If a "trainer" exists we just need to read past
		// it before we get to the good stuff
		if (header.mapper1 & 0x04)
			ifs.seekg(512, std::ios_base::cur);

		// Determine Mapper ID
		id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
		mirroring = (header.mapper1 & 0x01) ? Mirroring::VERTICAL : Mirroring::HORIZONTAL;

		prg_banks = header.prg_rom_chunks;
		prg = new uint8_t[prg_banks * 16384];
		ifs.read((char*)prg, prg_banks * 16384);

		chr_banks = header.chr_rom_chunks;
		int chr_size = 0;
		if (chr_banks == 0)
		{
			// Create CHR RAM
			chr = new uint8_t[8192];
		}
		else
		{
			// Allocate for ROM
			chr_size = chr_banks * 8192;
			chr = new uint8_t[chr_size];
		}
		ifs.read((char*)chr, chr_size);
		ok = true;
		ifs.close();
	}
}

Cartridge::~Cartridge()
{
	if (prg != nullptr) delete prg;
	if (chr != nullptr) delete chr;
}

int Cartridge::get_id(std::string filename)
{
	sHeader header;
	int id = 0;
	std::ifstream ifs;
	ifs.open(filename, std::ifstream::binary);
	if (ifs.is_open())
	{
		ifs.read((char*)&header, sizeof(sHeader));
		if (header.mapper1 & 0x04)
			ifs.seekg(512, std::ios_base::cur);
		// Determine Mapper ID
		id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
	}
	ifs.close();
	return id;
}

void Cartridge::get_prg_bank_ptrs(void** bank0, void** bank1) const
{
	*bank0 = reinterpret_cast<void*>(this->prg_bank0);
	*bank1 = reinterpret_cast<void*>(this->prg_bank1);
}

void Cartridge::get_pattern_table_ptrs(void** tbl0, void** tbl1) const
{
	*tbl0 = reinterpret_cast<void*>(this->pattern_tbl0);
	*tbl1 = reinterpret_cast<void*>(this->pattern_tbl1);
}

void Cartridge::write(uint16_t address, uint8_t data) 
{
	if (address >= 0x0000 && address < 0x2000) {
		if (chr_banks != 0) {
			return;//Not CHR RAM
		}
		uint8_t (*ptr)[0x2000] = reinterpret_cast<uint8_t(*)[0x2000]>(chr);
		(*ptr)[address] = data;
	}
}

uint8_t Cartridge::read(uint16_t address) const {
	uint8_t data = 0;
	uint8_t* ptr;
	if (address >= 0x0000 && address <= 0x0FFF) {
		ptr = reinterpret_cast<uint8_t*>(pattern_tbl0);
		data = ptr[address];
	}else if (address >= 0x1000 && address <= 0x1FFF) {
		ptr = reinterpret_cast<uint8_t*>(pattern_tbl1);
		data = ptr[address - 0x1000];
	}
	else if (address >= 0x8000 && address <= 0xBFFF) {
		ptr = reinterpret_cast<uint8_t*>(prg_bank0);
		data = ptr[address - 0x8000];
	}
	else if (address >= 0xC000 && address <= 0xFFFF) {
		ptr = reinterpret_cast<uint8_t*>(prg_bank1);
		data = ptr[address - 0xC000];
	}
	return data;
}

