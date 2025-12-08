#include"cartridge.h"
#include<string>

#ifndef MMC1_H
#define MMC1_H

constexpr uint8_t control_reg = 0;
constexpr uint8_t chr0_reg    = 1;
constexpr uint8_t chr1_reg    = 2;
constexpr uint8_t prg_reg     = 3;

class MMC1 : public Cartridge {
public:
	MMC1(std::string filename);
	virtual ~MMC1();
	void write(uint16_t address, uint8_t data) override;
	bool good() override;
	NametableInterface* get_nametable_interface() override;
private:
	void apply_settings();
	uint8_t shift_reg;
	uint8_t shift_pos;
	uint8_t regs[4];
	void reset(uint8_t data);
	void shift_load(uint8_t data, uint8_t sel);
	NametableData data;
	NametableInterface* horizontal;
	NametableInterface* vertical;
	NametableInterface* single_screen;
	NametableInterface* inametable;
};

#endif


