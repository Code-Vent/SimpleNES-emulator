#ifndef CONTROLLER_H
#define CONTROLLER_H

#include<stdint.h>


class Controller {
public:
	Controller();
	void write(uint16_t address, uint8_t data);
	uint8_t read(uint16_t address);
	void update();
private:
	bool    shift_enabled;
	bool    latch_enabled;
	uint8_t button;
	uint8_t player0;
	uint8_t player1;
};


#endif