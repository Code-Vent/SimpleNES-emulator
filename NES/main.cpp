#include<iostream>
#include<fstream>
#include<sstream>
#include<stdint.h>
#include<iomanip>
#include<chrono>
#include"cpu6502.h"
#include"ppu2c05.h"
#include"mapper.h"

#include"raylib/raylib.h"

//#undef SUPPORT_TRACElOG

using namespace std;


int main() {
	auto mapper = Mapper("./games/Battletoads.nes");
	Cartridge* cart = mapper.get_cartridge();
	PPU2C05 ppu;
	CPU6502 cpu = {};
	Controller controller = {};
	cpu.controller = &controller;

	if (cart != nullptr && cart->good()) {
		ppu = PPU2C05{ cart, &cpu };
	}
	else {
		exit(1);
	}
	//SetTraceLogLevel(LOG_NONE);

	
	InitWindow(800, 600, "NES Emulator");
	SetTargetFPS(60);
	ppu.init();
	while (!WindowShouldClose()) {
		SetWindowTitle(TextFormat("NES Emulator - FPS: %i", GetFPS()));		
		controller.update();
		ppu.pre_render();
		ppu.render();
		ppu.post_render();
		//controller.update();
	}	
	return 0;
}