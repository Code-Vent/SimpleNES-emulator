#include "mapper.h"
#include<iostream>


Mapper::Mapper(std::string filename)
	:cart(nullptr)
{
	auto id = Cartridge::get_id(filename);
	switch (id) {
	case 0:
		cart = new NROM(filename);
		break;
	case 1:
		cart = new MMC1(filename);
		break;
	case 3:
		cart = new CNROM(filename);
		break;
	case 7:
		cart = new AxROM(filename);
		break;
	default:
		std::cout << "Error! Unsupported Mapper.\n";
		break;
	}
}

Cartridge* Mapper::get_cartridge()
{
	return cart;
}

Mapper::~Mapper()
{
	if (cart != nullptr) {
		delete cart;
	}
}
