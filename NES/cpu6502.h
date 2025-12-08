#include<stdint.h>
#include"cartridge.h"
#include"controller.h"
#include<iomanip>

#ifndef CPU6502_H
#define CPU6502_H

#define LOG_ADDRESSING_MODE(log1, log2, fout)
#define LOG_INSTRUCTION(pc, fout)   

struct CPU6502 {
	CPU6502() = default;
	void init(Cartridge* cart);
	void execute(int cycles);
	void nmi();
	void irq();
	void dma(uint8_t* dest, uint16_t src, size_t count) const;
	uint8_t read(uint16_t address) const;
	uint16_t read16(uint16_t address) const;
	void  write(uint16_t address, uint8_t data);
	void  push(uint8_t data);
	uint8_t pull();
	void  set_flag(uint8_t mask);
	void  clear_flag(uint8_t mask);
	void  write_flag(uint8_t mask, bool value);
	void  updateZN_flags(uint8_t result);
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
	uint8_t    ram     [0x0800];
	Cartridge* cart;
	Controller* controller;
	void    (*ppu_write)(uint16_t address, uint8_t data);
	uint8_t (*ppu_read)(uint16_t address);
	//uint8_t(*apu_regs) [0x0018];
	uint8_t cart_ram [0x2000];
	//////////////////////////////////
	// Registers
	/////////////////////////////////
	uint16_t PC;
	uint8_t  A, X, Y, SR;
	int16_t  SP;
	uint8_t  data_bus;
	uint16_t address_bus;
	int   cycles;
	uint8_t  opcode;
};

void BRK(CPU6502* cpu);
void BIT(CPU6502* cpu);
void SEI(CPU6502* cpu);
void SEC(CPU6502* cpu);
void SED(CPU6502* cpu);
void PHP(CPU6502* cpu);
void PLP(CPU6502* cpu);
void PHA(CPU6502* cpu);
void PLA(CPU6502* cpu);
void CLD(CPU6502* cpu);
void CLC(CPU6502* cpu);
void CLI(CPU6502* cpu);
void CLV(CPU6502* cpu);
void ADC(CPU6502* cpu);
void SBC(CPU6502* cpu);
void AND(CPU6502* cpu);
void ORA(CPU6502* cpu);
void EOR(CPU6502* cpu);
void LDA(CPU6502* cpu);
void LDX(CPU6502* cpu);
void LDY(CPU6502* cpu);
void INC(CPU6502* cpu);
void INX(CPU6502* cpu);
void INY(CPU6502* cpu);
void DEC(CPU6502* cpu);
void DEX(CPU6502* cpu);
void DEY(CPU6502* cpu);
void TAX(CPU6502* cpu);
void TAY(CPU6502* cpu);
void TSX(CPU6502* cpu);
void TXA(CPU6502* cpu);
void TYA(CPU6502* cpu);
void TXS(CPU6502* cpu);
void CMP(CPU6502* cpu);
void CPX(CPU6502* cpu);
void CPY(CPU6502* cpu);
void STA(CPU6502* cpu);
void STX(CPU6502* cpu);
void STY(CPU6502* cpu);
void BCC(CPU6502* cpu);
void BCS(CPU6502* cpu);
void BEQ(CPU6502* cpu);
void BNE(CPU6502* cpu);
void BPL(CPU6502* cpu);
void BMI(CPU6502* cpu);
void BVC(CPU6502* cpu);
void BVS(CPU6502* cpu);
void JMP(CPU6502* cpu);
void JSR(CPU6502* cpu);
void RTS(CPU6502* cpu);
void RTI(CPU6502* cpu);
void NOP(CPU6502* cpu);
void LSR(CPU6502* cpu);
void ASL(CPU6502* cpu);
void ROR(CPU6502* cpu);
void ROL(CPU6502* cpu);
void LAX(CPU6502* cpu);
void SAX(CPU6502* cpu);
void DCP(CPU6502* cpu);
void ISB(CPU6502* cpu);
void SLO(CPU6502* cpu);
void RLA(CPU6502* cpu);
void SRE(CPU6502* cpu);
void RRA(CPU6502* cpu);



void IMP(CPU6502* cpu);
void IMM(CPU6502* cpu);
void REL(CPU6502* cpu);
void ZPX(CPU6502* cpu);
void ZPY(CPU6502* cpu);
void ABS(CPU6502* cpu);
void ABX(CPU6502* cpu);
void ABY(CPU6502* cpu);
void INDX(CPU6502* cpu);
void INDY(CPU6502* cpu);
void IND(CPU6502* cpu);
void ZP0(CPU6502* cpu);

#endif