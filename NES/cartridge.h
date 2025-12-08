#include<stdint.h>
#include<string>
#include"backgroung.h"

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

struct sHeader
{
	char name[4];
	uint8_t prg_rom_chunks;
	uint8_t chr_rom_chunks;
	uint8_t mapper1;
	uint8_t mapper2;
	uint8_t prg_ram_size;
	uint8_t tv_system1;
	uint8_t tv_system2;
	char unused[5];
};

enum class Mirroring {
	VERTICAL,
	HORIZONTAL,
	FOURSCREEN,
	SINGLESCREEN,
};

class Cartridge : public ReadWriteable{
public:
	Cartridge(std::string filename);
	virtual ~Cartridge();
	static int get_id(std::string filename);
	void get_prg_bank_ptrs(void** bank0, void** bank1) const;
	void get_pattern_table_ptrs(void** tbl0, void** tbl1) const;
	virtual NametableInterface* get_nametable_interface() { return nullptr; };
	uint8_t read(uint16_t address) const override;
	void write(uint16_t address, uint8_t data) override;
	virtual bool good() = 0;
	Mirroring mirroring;
//protected:
	void*     prg;
	void*     chr;
	void*     prg_bank0;
	void*     prg_bank1;
	void*     pattern_tbl0;
	void*     pattern_tbl1;
	uint8_t   chr_banks;
	uint8_t   prg_banks;
	uint8_t   id;
	bool      ok;
};

#endif // !1
