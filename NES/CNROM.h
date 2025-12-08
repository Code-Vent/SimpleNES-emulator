#include"cartridge.h"
#include<string>

#ifndef CNROM_H
#define CNROM_H

class CNROM : public Cartridge {
public:
	CNROM(std::string filename);
	virtual ~CNROM();
	void write(uint16_t address, uint8_t data) override;
	bool good() override;
	NametableInterface* get_nametable_interface() override;
private:
	NametableData data;
	NametableInterface* inametable;
};

#endif


