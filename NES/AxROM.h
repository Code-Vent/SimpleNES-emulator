#include"cartridge.h"
#include<string>

#ifndef AxROM_H
#define AxROM_H

class AxROM : public Cartridge {
public:
	AxROM(std::string filename);
	virtual ~AxROM();
	void write(uint16_t address, uint8_t data) override;
	bool good() override;
	NametableInterface* get_nametable_interface() override;
private:
	void apply_settings();
	uint8_t bank_sel;
	NametableData data;
	NametableInterface* single_screen;
};

#endif


