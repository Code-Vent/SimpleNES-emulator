#include"cartridge.h"
#include"backgroung.h"
#include<string>

#ifndef NROM_H
#define NROM_H

class NROM : public Cartridge {
public:
	NROM(std::string filename);
	virtual ~NROM();
	void write(uint16_t address, uint8_t data) override;
	bool good() override;
	NametableInterface* get_nametable_interface() override;
private:
	NametableData data;
	NametableInterface* inametable;
};

#endif



