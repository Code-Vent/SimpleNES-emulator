#include "controller.h"
#include"raylib/raylib.h"
#include<stdio.h>
#include<array>

constexpr std::array<KeyboardKey, 8> keys = {
	KEY_X,
	KEY_Z,
	KEY_A,
	KEY_S,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT
};

constexpr std::array<uint8_t, 8> button_masks = {
	(1 << 0),//BUTTON_A
	(1 << 1),//BUTTON_B
	(1 << 2),//SELECT
	(1 << 3),//START
	(1 << 4),//UP
	(1 << 5),//DOWN
	(1 << 6),//LEFT
	(1 << 7)//RIGHT 
};

Controller::Controller()
	:player0{0}, player1{0}, button{0}
{
	shift_enabled = false;
	latch_enabled = false;
}

void Controller::write(uint16_t address, uint8_t data) {
	if (address == 0x4016) {
		shift_enabled = (data & 0x01) == 0;
		latch_enabled = (data & 0x01) != 0;
	}
	else {
		shift_enabled = (data & 0x01) == 0;
	}
}


uint8_t Controller::read(uint16_t address) {
	uint8_t key = 0;
	if (shift_enabled) {
		key = (player0 & button_masks[button]) >> button;
		button += 1;
		button %= 8;
	}
	else {
		key = player0;
	}	
	return key;
}

void Controller::update()
{
	for (int i = 0; i < 8; ++i) {
		if (IsKeyDown(keys[i])) {
			player0 |= button_masks[i];
		}
		else {
			player0 &= ~button_masks[i];
		}
	}	
}
