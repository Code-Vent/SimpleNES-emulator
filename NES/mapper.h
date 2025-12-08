#ifndef MAPPER_H
#define MAPPER_H
#include"NROM.h"
#include"CNROM.h"
#include"MMC1.h"
#include"AxROM.h"

class Mapper {
public:
	Mapper(std::string filename);
	Cartridge* get_cartridge();
	~Mapper();
private:
	Cartridge* cart;
};


#endif
