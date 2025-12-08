#include"cpu6502.h"
#include<assert.h>
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>

constexpr auto CARRY     = 0x01;
constexpr auto ZERO      = 0x02;
constexpr auto INTERRUPT = 0x04;
constexpr auto DECIMAL   = 0x08;
constexpr auto BREAK     = 0x10;
constexpr auto HIGH      = 0x20;
constexpr auto OVERFLOW_ = 0x40;
constexpr auto NEGATIVE  = 0x80;

static std::stringstream logger;

static const int INSTR_CYCLES[] = {
    7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,//0
    2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,//1
    6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,//2
    2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,//3
    6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,//4
    2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,//5
    6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,//6
    2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,//7
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,//8
    2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,//9
    2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,//A
    2,5,2,5,4,4,4,4,2,4,2,4,4,4,4,4,//B
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,//C
    2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,//D
    2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,//E
    2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,//F
};

void CPU6502::init(Cartridge* cart)
{
    assert(cart != nullptr);
	this->cart = cart;
	PC = read16(0xFFFC);
	SP = 0xFF;
    SR = HIGH;
}

void CPU6502::execute(int target) {
    
    this->cycles = 0;
    while (this->cycles < target) {
        opcode = read(PC);
        switch (opcode) {
        case 0:
            BRK(this); break;
        case 0x78:
            SEI(this); break;
        case 0x38:
            SEC(this); break;
        case 0xF8:
            SED(this); break;
        case 0x08:
            PHP(this); break;
        case 0x28:
            PLP(this); break;
        case 0x48:
            PHA(this); break;
        case 0x68:
            PLA(this); break;
        case 0xD8:
            CLD(this); break;
        case 0x18:
            CLC(this); break;
        case 0x58:
            CLI(this); break;
        case 0xB8:
            CLV(this); break;
        case 0x69: case 0x65: case 0x75: case 0x6D:
        case 0x7D: case 0x79: case 0x61: case 0x71:
            ADC(this); break;
        case 0xE9: case 0xE5: case 0xF5: case 0xED:
        case 0xFD: case 0xF9: case 0xE1: case 0xF1:
        case 0xEB:
            SBC(this); break;
        case 0x29: case 0x25: case 0x35: case 0x2D:
        case 0x3D: case 0x39: case 0x21: case 0x31:
            AND(this); break;
        case 0x09: case 0x05: case 0x15: case 0x0D:
        case 0x1D: case 0x19: case 0x01: case 0x11:
            ORA(this); break;
        case 0xE6: case 0xF6: case 0xEE: case 0xFE:
            INC(this); break;
        case 0xE8:
            INX(this); break;
        case 0xC8:
            INY(this); break;
        case 0xAA:
            TAX(this); break;
        case 0xA8:
            TAY(this); break;
        case 0xBA:
            TSX(this); break;
        case 0x8A:
            TXA(this); break;
        case 0x9A:
            TXS(this); break;
        case 0x98:
            TYA(this); break;
        case 0xC6: case 0xD6: case 0xCE: case 0xDE:
            DEC(this); break;
        case 0xCA:
            DEX(this); break;
        case 0x88:
            DEY(this); break;
        case 0x49: case 0x45: case 0x55: case 0x4D:
        case 0x5D: case 0x59: case 0x41: case 0x51:
            EOR(this); break;
        case 0xC9: case 0xC5: case 0xD5: case 0xCD:
        case 0xDD: case 0xD9: case 0xC1: case 0xD1:
            CMP(this); break;
        case 0xE0: case 0xE4: case 0xEC:
            CPX(this); break;
        case 0xC0: case 0xC4: case 0xCC:
            CPY(this); break;
        case 0x4A: case 0x46: case 0x56: case 0x4E:
        case 0x5E:
            LSR(this); break;
        case 0x0A: case 0x06: case 0x16: case 0x0E:
        case 0x1E:
            ASL(this); break;
        case 0x6A: case 0x66: case 0x76: case 0x6E:
        case 0x7E:
            ROR(this); break;
        case 0x2A: case 0x26: case 0x36: case 0x2E:
        case 0x3E:
            ROL(this); break;
        case 0xA9: case 0xA5: case 0xB5: case 0xAD:
        case 0xBD: case 0xB9: case 0xA1: case 0xB1:
            LDA(this); break;
        case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE:
            LDX(this); break;
        case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC:
            LDY(this); break;
        case 0x85: case 0x95: case 0x8D:
        case 0x9D: case 0x99: case 0x81: case 0x91:
            STA(this); break;
        case 0x86: case 0x96: case 0x8E:
            STX(this); break;
        case 0x84: case 0x94: case 0x8C:
            STY(this); break;
        case 0x90:
            BCC(this); break;
        case 0xB0:
            BCS(this); break;
        case 0xF0:
            BEQ(this); break;
        case 0xD0:
            BNE(this); break;
        case 0x10:
            BPL(this); break;
        case 0x30:
            BMI(this); break;
        case 0x50:
            BVC(this); break;
        case 0x70:
            BVS(this); break;
        case 0x4C: case 0x6C:
            JMP(this); break;
        case 0x20:
            JSR(this); break;
        case 0x60:
            RTS(this); break;
        case 0x40:
            RTI(this); break;
        case 0xEA: case 0x04: case 0x14: case 0x34:
        case 0x44: case 0x54: case 0x64: case 0x74:
        case 0x0C: case 0x1C: case 0x3C: case 0x5C:
        case 0x7C: case 0xDC: case 0xFC: case 0xD4:
        case 0xF4: case 0x1A: case 0x3A: case 0x5A: case 0x7A:
        case 0xDA: case 0xFA: case 0x80:
            NOP(this); break;
        case 0x24: case 0x2C:
            BIT(this); break;
        case 0xA7: case 0xB7: case 0xAF: case 0xBF:
        case 0xA3: case 0xB3:
            LAX(this); break;
        case 0x87: case 0x97: case 0x8F: case 0x83:
            SAX(this); break;
        case 0xC7: case 0xD7: case 0xCF: case 0xDF:
        case 0xDB: case 0xC3: case 0xD3:
            DCP(this); break;
        case 0xE7: case 0xF7: case 0xEF: case 0xFF:
        case 0xFB: case 0xE3: case 0xF3:
            ISB(this); break;
        case 0x07: case 0x17: case 0x0F: case 0x1F:
        case 0x1B: case 0x03: case 0x13:
            SLO(this); break;
        case 0x27: case 0x37: case 0x2F: case 0x3F:
        case 0x3B: case 0x23: case 0x33:
            RLA(this); break;
        case 0x47: case 0x57: case 0x4F: case 0x5F:
        case 0x5B: case 0x43: case 0x53:
            SRE(this); break;
        case 0x67: case 0x77: case 0x6F: case 0x7F:
        case 0x7B: case 0x63: case 0x73:
            RRA(this); break;
        default:
            //show_line_no = false;
            break;
        }
        this->cycles += INSTR_CYCLES[opcode];
        //std::cout << this->cycles << std::endl;
        //logger.str("");
    }
    //volatile int x = 0;
}

void CPU6502::nmi()
{
    push((PC >> 8) & 0x00FF);
    push(PC & 0x00FF);
    push(SR & ~BREAK);
    set_flag(INTERRUPT);
    PC = read16(0xFFFA);
    cycles += 8;
}

void CPU6502::irq()
{
    if ((SR & INTERRUPT) == 0) {
        push((PC >> 8) & 0x00FF);
        push(PC & 0x00FF);
        push(SR & ~BREAK);
        set_flag(INTERRUPT);
        PC = read16(0xFFFE);
        cycles += 7;
    }
}

void CPU6502::dma(uint8_t* dest, uint16_t src, size_t count) const
{
    for (auto i = 0; i < count; ++i) {
        *(dest + i) = read(src++);
    }
}

uint16_t CPU6502::read16(uint16_t address) const {
	return (uint16_t(read(address + 1)) << 8) | uint16_t(read(address));
}

uint8_t CPU6502::read(uint16_t address) const {
	uint8_t data = 0;
	if (address >= 0x8000 && address <= 0xFFFF) {
		data = cart->read(address);
	}
	else if (address >= 0x0000 && address <= 0x1FFF) {
		data = this->ram[address & 0x07FF];
	}
    else if (address >= 0x6000 && address <= 0x7FFF) {
        data = this->cart_ram[address - 0x6000];
    }
    else if (address >= 0x2000 && address <= 0x3FFF) {
        if (this->ppu_read != nullptr)
            data = this->ppu_read(address & 0x2007);
    }
    else if (address == 0x4016 || address == 0x4017) {
        assert(controller != nullptr);
        data = controller->read(address);
    }
    else if (address >= 0x4000 && address <= 0x4017) {
    }
	else {
		assert(false);
	}
	return data;
}

void  CPU6502::write(uint16_t address, uint8_t data) {
    if (address >= 0x8000 && address <= 0xFFFF) {
        cart->write(address, data);
    }
    else if (address >= 0x0000 && address <= 0x1FFF) {
        this->ram[address & 0x07FF] = data;
    }
    else if (address >= 0x6000 && address <= 0x7FFF) {
        this->cart_ram[address - 0x6000] = data;
    }
    else if (address >= 0x2000 && address <= 0x3FFF) {
        if (this->ppu_write != nullptr)
            this->ppu_write(address & 0x2007, data);
    }
    else if (address == 0x4014) {
        this->ppu_write(address, data);
    }
    else if (address == 0x4016 || address == 0x4017) {
        assert(controller != nullptr);
        controller->write(address, data);
    }
    else if (address >= 0x4000 && address <= 0x4017) {
    }
    else {
        assert(false);
    }
}

void CPU6502::push(uint8_t data)
{
    if (SP >= 0) {
        ram[0x0100 + SP] = data;
        SP -= 1;
    }
}

uint8_t CPU6502::pull()
{
    uint8_t data = 0;
    if (SP <= 0xFF) {
        SP += 1;
        data = ram[0x0100 + SP];
    }
    return data;
}

void CPU6502::set_flag(uint8_t mask)
{
    SR |= mask;
}

void CPU6502::clear_flag(uint8_t mask)
{
    SR &= ~mask;
}

void CPU6502::write_flag(uint8_t mask, bool value)
{
    if (value) {
        set_flag(mask);
    }
    else {
        clear_flag(mask);
    }
}

void CPU6502::updateZN_flags(uint8_t result)
{
    write_flag(ZERO, result == 0);
    write_flag(NEGATIVE, (result & 0x80) != 0);
}

static void addressing_mode(CPU6502* cpu) {
    switch (cpu->opcode) {
    case 0x65: case 0xA5: case 0xA6: case 0xA4:
    case 0x85: case 0x86: case 0x84: case 0x24:
    case 0x25: case 0xC5: case 0xE4: case 0xC4:
    case 0x05: case 0x45: case 0xE5: case 0xE6:
    case 0xC6: case 0x46: case 0x06: case 0x66:
    case 0x26: case 0x04: case 0x44: case 0x64:
    case 0xA7: case 0x87: case 0xC7: case 0xE7:
    case 0x07: case 0x27: case 0x47: case 0x67:
        ZP0(cpu); break;
    case 0x75: case 0xB5: case 0xB4: case 0x95:
    case 0x94: case 0x35: case 0xD5: case 0x15:
    case 0x55: case 0xF5: case 0xF6: case 0xD6:
    case 0x56: case 0x16: case 0x76: case 0x36:
    case 0x14: case 0x34: case 0x54: case 0x74:
    case 0xD4: case 0xF4: case 0xD7: case 0xF7:
    case 0x17: case 0x37: case 0x57: case 0x77:
        ZPX(cpu); break;
    case 0xB6: case 0x96: case 0xB7: case 0x97:
        ZPY(cpu); break;
    case 0x6D: case 0xAD: case 0xAE: case 0xAC:
    case 0x8D: case 0x8E: case 0x8C: case 0x4C:
    case 0x20: case 0x2C: case 0x2D: case 0xCD:
    case 0xEC: case 0xCC: case 0x0D: case 0x4D:
    case 0xED: case 0xEE: case 0xCE: case 0x4E:
    case 0x0E: case 0x6E: case 0x2E: case 0x0C:
    case 0xAF: case 0x8F: case 0xCF: case 0xEF:
    case 0x0F: case 0x2F: case 0x4F: case 0x6F:
        ABS(cpu); break;
    case 0x7D: case 0xBD: case 0xBC: case 0x9D:
    case 0x3D: case 0xDD: case 0x1D: case 0x5D:
    case 0xFD: case 0xFE: case 0xDE: case 0x5E:
    case 0x1E: case 0x7E: case 0x3E: case 0x1C:
    case 0x3C: case 0x5C: case 0x7C: case 0xDC:
    case 0xFC: case 0xDF: case 0xFF: case 0x1F:
    case 0x3F: case 0x5F: case 0x7F:
        ABX(cpu); break;
    case 0x79: case 0xB9: case 0xBE: case 0x99:
    case 0x39: case 0xD9: case 0x19: case 0x59:
    case 0xF9: case 0xBF: case 0xDB: case 0xFB:
    case 0x1B: case 0x3B: case 0x5B: case 0x7B:
        ABY(cpu); break;
    case 0x61: case 0xA1: case 0x81: case 0x21:
    case 0xC1: case 0x01: case 0x41: case 0xE1:
    case 0xA3: case 0x83: case 0xC3: case 0xE3:
    case 0x03: case 0x23: case 0x43: case 0x63:
        INDX(cpu); break;
    case 0x71: case 0xB1: case 0x91: case 0x31:
    case 0xD1: case 0x11: case 0x51: case 0xF1:
    case 0xB3: case 0xD3: case 0xF3: case 0x13:
    case 0x33: case 0x53: case 0x73:
        INDY(cpu); break;
    case 0x90: case 0xB0: case 0xF0: case 0xD0:
    case 0x10: case 0x30: case 0x50: case 0x70:
        REL(cpu); break;
    case 0x6C:
        IND(cpu); break;
    }
}

void BRK(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t hi = ((cpu->PC + 2) & 0xFF00) >> 8;
    uint8_t lo = ((cpu->PC + 2) & 0x00FF);
    cpu->push(hi);
    cpu->push(lo);
    cpu->push(cpu->SR | BREAK);
    cpu->set_flag(INTERRUPT);
    //IMM(cpu);
    cpu->PC = 0xFFFE;
}

void BIT(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    uint8_t value = cpu->read(cpu->address_bus);
    uint8_t result = cpu->A & value;
    cpu->write_flag(ZERO, result == 0);
    cpu->write_flag(NEGATIVE, (value & 0x80) != 0);
    cpu->write_flag(OVERFLOW_, (value & 0x40) != 0);
}

void SEI(CPU6502* cpu)
{
	LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->set_flag(INTERRUPT);
}

void SEC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->set_flag(CARRY);
}

void SED(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->set_flag(DECIMAL);
}

void PHP(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->push(cpu->SR | BREAK);
}

void PLP(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->SR = cpu->pull();
}

void PHA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->push(cpu->A);
}

void PLA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->A = cpu->pull();
    cpu->updateZN_flags(cpu->A);
}

void CLD(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->clear_flag(DECIMAL);
}

void CLC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->clear_flag(CARRY);
}

void CLI(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->clear_flag(INTERRUPT);
}

void CLV(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->clear_flag(OVERFLOW_);
}

static void adc(CPU6502* cpu) {
    int carry_in = ((cpu->SR & CARRY) != 0) ? 1 : 0;
    auto sum = int(int8_t(cpu->A)) + int(int8_t(cpu->data_bus)) + carry_in;
    auto carry_out = int(cpu->A) + int(cpu->data_bus) + carry_in;
    auto result = uint8_t(sum & 0x000000FF);

    cpu->write_flag(CARRY, carry_out > 255);
    cpu->write_flag(ZERO, result == 0);

    uint8_t overflow = (result ^ cpu->A) & (result ^ cpu->data_bus);
    cpu->A = result;

    cpu->write_flag(OVERFLOW_, (overflow & 0x80) != 0);
    cpu->write_flag(NEGATIVE, (result & 0x80) != 0);
}

void ADC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    switch (cpu->opcode) {
    case 0x69: 
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    adc(cpu);
}

void SBC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    switch (cpu->opcode) {
    case 0xE9: case 0xEB:
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    cpu->data_bus = ~cpu->data_bus;
    adc(cpu);
}

void AND(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    switch (cpu->opcode) {
    case 0x29:
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    cpu->A &= cpu->data_bus;
    cpu->updateZN_flags(cpu->A);
}

void ORA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    switch (cpu->opcode) {
    case 0x09:
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    cpu->A |= cpu->data_bus;
    cpu->updateZN_flags(cpu->A);
}

void EOR(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    switch (cpu->opcode) {
    case 0x49:
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    cpu->A ^= cpu->data_bus;
    cpu->updateZN_flags(cpu->A);
}

static void load(CPU6502* cpu, uint8_t* reg) {
    switch (cpu->opcode) {
    case 0xA9: case 0xA2: case 0xA0:
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    *reg = cpu->data_bus;
    cpu->updateZN_flags(cpu->data_bus);
}

void LDA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    load(cpu, &cpu->A);
}

void LDX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    load(cpu, &cpu->X);
}

void LDY(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    load(cpu, &cpu->Y);
}

void INC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    uint8_t result = cpu->read(cpu->address_bus) + 1;
    cpu->updateZN_flags(result);
    cpu->write(cpu->address_bus, result);
}

void INX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    uint8_t result = cpu->X + 1;
    cpu->updateZN_flags(result);
    cpu->X = result;
}

void INY(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    uint8_t result = cpu->Y + 1;
    cpu->updateZN_flags(result);
    cpu->Y = result;
}

void DEC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    uint8_t result = cpu->read(cpu->address_bus) - 1;
    cpu->updateZN_flags(result);
    cpu->write(cpu->address_bus, result);
}

void DEX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    uint8_t result = cpu->X - 1;
    cpu->updateZN_flags(result);
    cpu->X = result;
}

void DEY(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    uint8_t result = cpu->Y - 1;
    cpu->updateZN_flags(result);
    cpu->Y = result;
}

void TAX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->X = cpu->A;
    cpu->updateZN_flags(cpu->X);
}

void TAY(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->Y = cpu->A;
    cpu->updateZN_flags(cpu->Y);
}

void TSX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->X = (uint8_t)cpu->SP;
    cpu->updateZN_flags(cpu->X);
}

void TXA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->A = cpu->X;
    cpu->updateZN_flags(cpu->A);
}

void TYA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->A = cpu->Y;
    cpu->updateZN_flags(cpu->A);
}

static void compare(CPU6502* cpu, uint8_t* reg) {
    switch (cpu->opcode) {
    case 0xC9: case 0xE0: case 0xC0:
        IMM(cpu); break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        break;
    }
    uint8_t result = *reg - cpu->data_bus;
    cpu->updateZN_flags(result);
    cpu->write_flag(CARRY, *reg >= cpu->data_bus);
}

void CMP(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    compare(cpu, &cpu->A);
}

void CPX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    compare(cpu, &cpu->X);
}

void CPY(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    compare(cpu, &cpu->Y);
}

void STA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->write(cpu->address_bus, cpu->A);
}

void STX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->write(cpu->address_bus, cpu->X);
}

void STY(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->write(cpu->address_bus, cpu->Y);
}

void TXS(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->SP = cpu->X;
}

void BCC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & CARRY) == 0) {
        cpu->PC = cpu->address_bus;
    }
}

void BCS(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & CARRY) != 0) {
        cpu->PC = cpu->address_bus;
    }
}

void BEQ(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & ZERO) != 0) {
        cpu->PC = cpu->address_bus;
    }
}

void BNE(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & ZERO) == 0) {
        cpu->PC = cpu->address_bus;
    }
}

void BPL(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & NEGATIVE) == 0) {
        cpu->PC = cpu->address_bus;
    }
}

void BMI(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & NEGATIVE) != 0) {
        cpu->PC = cpu->address_bus;
    }
}

void BVS(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & OVERFLOW_) != 0) {
        cpu->PC = cpu->address_bus;
    }
}

void JMP(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->PC = cpu->address_bus;
}

void JSR(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t hi = ((cpu->PC + 2) & 0xFF00) >> 8;
    uint8_t lo = ((cpu->PC + 2) & 0x00FF);
    cpu->push(hi);
    cpu->push(lo);
    addressing_mode(cpu);
    cpu->PC = cpu->address_bus;
}

void RTS(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    auto lo = cpu->pull();
    auto hi = cpu->pull();
    cpu->PC = uint16_t(hi) << 8 | uint16_t(lo);
    cpu->PC += 1;
}

void RTI(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    IMP(cpu);
    cpu->SR = cpu->pull();
    auto lo = cpu->pull();
    auto hi = cpu->pull();
    cpu->PC = uint16_t(hi) << 8 | uint16_t(lo);
}

void NOP(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    switch (cpu->opcode) {
    case 0xEA: case 0x1A: case 0x3A: case 0x5A: case 0x7A:
    case 0xDA: case 0xFA:
        IMP(cpu); break;
    case 0x80:
        IMM(cpu); break;
    default:
        addressing_mode(cpu); break;
    }    
}

void LSR(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t carry = 0;
    switch (cpu->opcode) {
    case 0x4A:
        IMP(cpu); 
        carry = cpu->A & 0x01;
        cpu->A >>= 1;
        cpu->data_bus = cpu->A;
        break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        carry = cpu->data_bus & 0x01;
        cpu->data_bus >>= 1;
        cpu->write(cpu->address_bus, cpu->data_bus);
    }
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->data_bus);
}

void ASL(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t carry = 0;
    switch (cpu->opcode) {
    case 0x0A:
        IMP(cpu);
        carry = cpu->A & 0x80;
        cpu->A <<= 1;
        cpu->data_bus = cpu->A;
        break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        carry = cpu->data_bus & 0x80;
        cpu->data_bus <<= 1;
        cpu->write(cpu->address_bus, cpu->data_bus);
    }
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->data_bus);
}

void ROR(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t carry = 0;
    uint8_t flags = cpu->SR;
    switch (cpu->opcode) {
    case 0x6A:
        IMP(cpu);
        carry = cpu->A & 0x01;
        cpu->A >>= 1;
        cpu->A |= ((flags & CARRY) != 0) ? 0x80 : 0;
        cpu->data_bus = cpu->A;
        break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        carry = cpu->data_bus & 0x01;
        cpu->data_bus >>= 1;
        cpu->data_bus |= ((flags & CARRY) != 0) ? 0x80 : 0;
        cpu->write(cpu->address_bus, cpu->data_bus);
    }
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->data_bus);
}

void ROL(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t carry = 0;
    uint8_t flags = cpu->SR;
    switch (cpu->opcode) {
    case 0x2A:
        IMP(cpu);
        carry = cpu->A & 0x80;
        cpu->A <<= 1;
        cpu->A |= ((flags & CARRY) != 0) ? 0x01 : 0;
        cpu->data_bus = cpu->A;
        break;
    default:
        addressing_mode(cpu);
        cpu->data_bus = cpu->read(cpu->address_bus);
        carry = cpu->data_bus & 0x80;
        cpu->data_bus <<= 1;
        cpu->data_bus |= ((flags & CARRY) != 0) ? 0x01 : 0;
        cpu->write(cpu->address_bus, cpu->data_bus);
    }
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->data_bus);
}

void LAX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->A = cpu->read(cpu->address_bus);
    cpu->X = cpu->A;
    cpu->updateZN_flags(cpu->A);
}

void SAX(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    uint8_t result = cpu->A & cpu->X;
    cpu->write(cpu->address_bus, result);
}

void DCP(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->data_bus = cpu->read(cpu->address_bus) - 1;
    uint8_t result = cpu->A - cpu->data_bus;
    cpu->updateZN_flags(result);
    cpu->write_flag(CARRY, cpu->A >= cpu->data_bus);
    cpu->write(cpu->address_bus, cpu->data_bus);
}

void ISB(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->data_bus = cpu->read(cpu->address_bus) + 1;
    cpu->write(cpu->address_bus, cpu->data_bus);
    cpu->data_bus = ~cpu->data_bus;
    adc(cpu);
}

void SLO(CPU6502* cpu)
{
    uint8_t carry = 0;
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->data_bus = cpu->read(cpu->address_bus);
    carry = cpu->data_bus & 0x80;
    cpu->data_bus <<= 1;
    cpu->write(cpu->address_bus, cpu->data_bus);
    cpu->A |= cpu->data_bus;
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->A);
}

void RLA(CPU6502* cpu)
{
    uint8_t carry = 0;
    uint8_t flags = cpu->SR;
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->data_bus = cpu->read(cpu->address_bus);
    carry = cpu->data_bus & 0x80;
    cpu->data_bus <<= 1;
    cpu->data_bus |= ((flags & CARRY) != 0) ? 0x01 : 0;
    cpu->write(cpu->address_bus, cpu->data_bus);
    cpu->A &= cpu->data_bus;
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->A);
}

void SRE(CPU6502* cpu)
{
    uint8_t carry = 0;
    LOG_INSTRUCTION(cpu->PC, logger);
    addressing_mode(cpu);
    cpu->data_bus = cpu->read(cpu->address_bus);
    carry = cpu->data_bus & 0x01;
    cpu->data_bus >>= 1;
    cpu->write(cpu->address_bus, cpu->data_bus);
    cpu->A ^= cpu->data_bus;
    cpu->write_flag(CARRY, carry != 0);
    cpu->updateZN_flags(cpu->A);
}

void RRA(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    uint8_t carry = 0;
    uint8_t flags = cpu->SR;
    addressing_mode(cpu);
    cpu->data_bus = cpu->read(cpu->address_bus);
    carry = cpu->data_bus & 0x01;
    cpu->data_bus >>= 1;
    cpu->data_bus |= ((flags & CARRY) != 0) ? 0x80 : 0;
    cpu->write(cpu->address_bus, cpu->data_bus);
    cpu->write_flag(CARRY, carry != 0);
    adc(cpu);
}

void BVC(CPU6502* cpu)
{
    LOG_INSTRUCTION(cpu->PC, logger);
    REL(cpu);
    if ((cpu->SR & OVERFLOW_) == 0) {
        cpu->PC = cpu->address_bus;
    }
}


/////////////////////////////////////////////////////////////////////
///                    ADDRESSING MODES                       //////
////////////////////////////////////////////////////////////////////


void IMP(CPU6502* cpu) {
    LOG_ADDRESSING_MODE("", "", logger);
    cpu->PC += 1;
}

void IMM(CPU6502* cpu) {
    cpu->data_bus = cpu->read(cpu->PC + 1);
    LOG_ADDRESSING_MODE("#$", (int)cpu->data_bus, logger);
    cpu->PC += 2;
}

void REL(CPU6502* cpu) {
    uint8_t arg = cpu->read(cpu->PC + 1);
    int eaddr = int(cpu->PC + 2) + int(int8_t(arg));
    cpu->address_bus = uint16_t(eaddr);
    LOG_ADDRESSING_MODE("$", (int)eaddr, logger);
    cpu->PC += 2;
}

void ZPX(CPU6502* cpu) {
    uint8_t arg = cpu->read(cpu->PC + 1);
    cpu->address_bus = (uint16_t(arg) + uint16_t(cpu->X)) & 0x00FF;
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 2;
}

void ZPY(CPU6502* cpu) {
    uint8_t arg = cpu->read(cpu->PC + 1);
    cpu->address_bus = (uint16_t(arg) + uint16_t(cpu->Y)) & 0x00FF;
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 2;
}

void ABS(CPU6502* cpu) {
    cpu->address_bus = cpu->read16(cpu->PC + 1);
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 3;
}

void ABX(CPU6502* cpu) {
    uint16_t addr = cpu->read16(cpu->PC + 1);
    cpu->address_bus = addr + uint16_t(cpu->X);
    if ((cpu->address_bus & 0xFF00) != (addr & 0xFF00)) {
        cpu->cycles += 1;
    }
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 3;
}

void ABY(CPU6502* cpu) {
    uint16_t addr = cpu->read16(cpu->PC + 1);
    cpu->address_bus = addr + uint16_t(cpu->Y);
    if ((cpu->address_bus & 0xFF00) != (addr & 0xFF00)) {
        cpu->cycles += 1;
    }
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 3;
}
//($FF,X)
void INDX(CPU6502* cpu) {
    uint8_t arg = cpu->read(cpu->PC + 1);
    uint16_t temp = (uint16_t(arg) + uint16_t(cpu->X)) & 0x00FF;
    uint8_t lo_addr = cpu->read(temp);
    temp = (temp + 1) & 0x00FF;
    uint8_t hi_addr = cpu->read(temp);
    cpu->address_bus = uint16_t(hi_addr) << 8 | uint16_t(lo_addr);
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 2;
}

void INDY(CPU6502* cpu) {
    uint8_t arg = cpu->read(cpu->PC + 1);
    uint8_t lo_addr = cpu->read(uint16_t(arg));
    uint8_t hi_addr = cpu->read(uint16_t(arg + 1) & 0x00FF);
    cpu->address_bus = (uint16_t(hi_addr) << 8 | uint16_t(lo_addr)) + uint16_t(cpu->Y);
   
    if ((cpu->address_bus & 0xFF00) != (uint16_t(hi_addr) << 8)) {
        cpu->cycles += 1;
    }
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 2;
}

void IND(CPU6502* cpu) {
    uint16_t addr = cpu->read16(cpu->PC + 1);
    cpu->address_bus = 0;
    if ((addr & 0x00FF) == 0x00FF) {
        uint8_t hi = cpu->read(addr & 0xFF00);
        uint8_t lo = cpu->read(addr);
        cpu->address_bus = uint16_t(hi) << 8 | uint16_t(lo);
    }
    else {
        cpu->address_bus = cpu->read16(addr);
    }
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 3;
}

void ZP0(CPU6502* cpu) {
    cpu->address_bus = uint16_t(cpu->read(cpu->PC + 1));
    LOG_ADDRESSING_MODE("$", (int)cpu->address_bus, logger);
    cpu->PC += 2;
}
