#include "CPU.h"
#include "PPU.h"
#include "GamePak.h"
#include <fstream>
#include <iostream>
#include "APU.h"
#include <SFML/Main.hpp>

namespace CPU
{
	bool strobe = false;
	u8 currentkey = 0;
	u8 joystate = 0;
	u8 joystate2 = 0;
	bool VBNMI = true; //Allow vertical blank NMI
	//Number of cycles for each opcode
	u8 cycleCount[256] =
	{
		//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
			7, 6, 1, 1, 1, 3, 5, 1, 3, 2, 2, 1, 1, 4, 6, 1, //0
			2, 5, 1, 1, 1, 4, 6, 1, 2, 4, 1, 1, 1, 4, 7, 1, //1
			6, 1, 1, 1, 3, 3, 5, 1, 4, 2, 2, 1, 4, 4, 6, 1, //2
			2, 6, 1, 1, 1, 4, 6, 1, 2, 4, 1, 1, 1, 4, 7, 1, //3
			6, 6, 1, 1, 1, 3, 5, 1, 3, 2, 2, 1, 3, 4, 6, 1, //4
			2, 5, 1, 1, 1, 4, 6, 1, 2, 4, 1, 1, 1, 4, 7, 1, //5
			6, 6, 1, 1, 1, 3, 5, 1, 4, 2, 2, 1, 5, 4, 6, 1, //6
			2, 5, 1, 1, 1, 4, 6, 1, 2, 4, 1, 1, 1, 4, 7, 1, //7
			1, 6, 1, 1, 3, 3, 3, 1, 2, 1, 2, 1, 4, 1, 4, 1, //8
			2, 6, 1, 1, 4, 4, 4, 1, 2, 5, 2, 1, 5, 1, 1, 1, //9
			2, 6, 2, 1, 3, 3, 3, 1, 2, 2, 2, 1, 4, 4, 4, 1, //A
			2, 5, 1, 1, 4, 3, 4, 1, 2, 1, 2, 4, 4, 4, 4, 1, //B
			2, 6, 1, 1, 3, 3, 5, 1, 2, 2, 2, 1, 4, 4, 6, 1, //C
			2, 5, 1, 1, 1, 4, 6, 1, 2, 4, 1, 1, 1, 4, 7, 1, //D
			2, 6, 1, 1, 3, 5, 1, 1, 2, 2, 2, 1, 4, 4, 6, 1, //E
			2, 5, 1, 1, 1, 6, 1, 1, 2, 4, 1, 1, 1, 4, 7, 1, //F
	};
	//Registers
	u8 accRegister = 0;
	u8 xRegister = 0;
	u8 yRegister = 0;
	u16 programCounter = 0;
	u8 stackPointer = 0;

	u8 memory[0xFFFF];

	struct statusFlag
	{
		bool N = 0;
		bool V = 0;
		bool B = 0;
		bool D = 0;
		bool I = 1;
		bool Z = 0;
		bool C = 0;
	}statusFlag;

	void checkNZFlags(u8 data)
	{
		statusFlag.N = data & 0x80;
		statusFlag.Z = data == 0;
	}

	u8 getFlags()
	{
		return 128 * statusFlag.N + 64 * statusFlag.V + 32 + 16 * statusFlag.B + 8 * statusFlag.D + 4 * statusFlag.I + 2 * statusFlag.Z + 1 * statusFlag.C;
	}

	void setFlags(u8 state)
	{
		statusFlag.N = state & 0x80;
		statusFlag.V = state & 0x40;
		statusFlag.B = 0;
		statusFlag.D = state & 0x8;
		statusFlag.I = state & 0x4;
		statusFlag.Z = state & 0x2;
		statusFlag.C = state & 0x1;
	}

	void write(u16 addr, u8 data)
	{
		if (addr < 0x2000)
		{
			if (addr > 1 && addr < 50)
			{
				memory[addr % 0x800] = data;
			}
			memory[addr % 0x800] = data;
		}
		else if (addr < 0x4000)
		{
			switch (addr % 0x8)
			{
			case 0:
			{
				PPU::writePPUCTRL(data);
				VBNMI = data & 0x80;
				break;
			}
			case 1:
			{
				PPU::writePPUMASK(data);
				break;
			}
			case 3:
			{
				PPU::writeOAMADDR(data);
				break;
			}
			case 4:
			{
				PPU::writeOAMDATA(data);
				break;
			}
			case 5:
			{
				PPU::writePPUSCROLL(data);
				break;
			}
			case 6:
			{
				PPU::writePPUADDR(data);
				break;
			}
			case 7:
			{
				PPU::writePPUDATA(data);
				break;
			}
			}
		}
		else if (addr < 0x4020)
		{
			switch (addr)
			{
			case 0x4014:
			{
				u8 OAMDATA[0x100];
				for (int i = 0; i < 0x100; i++)
				{
					OAMDATA[i] = read(data * 0x100 + i);
				}
				PPU::OAMDMA(OAMDATA);
				break;
			}
			case 0x4016:
			{
				if (strobe && !(data & 0x1))
				{
					joystate = sf::Keyboard::isKeyPressed(sf::Keyboard::K) * 0x01
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::L) * 0x02
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::G) * 0x04
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::H) * 0x08
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::W) * 0x10
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::S) * 0x20
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::A) * 0x40
						+ sf::Keyboard::isKeyPressed(sf::Keyboard::D) * 0x80;
					joystate2 = joystate;
				}
				strobe = data & 0x1;
				break;
			}
			}
		}
		else
		{
			memory[addr] = data;
		}
	}

	u8 read(u16 addr)
	{
		if (addr < 0x2000)
		{
			return memory[addr % 0x800];
		}
		else if (addr < 0x4000)
		{
			switch (addr % 0x8)
			{
			case 2:
			{
				return PPU::readPPUSTATUS();
			}
			case 3:
			{
				return PPU::readOAMADDR();
			}
			case 4:
			{
				return PPU::readOAMDATA();
			}
			case 7:
			{
				return PPU::readPPUDATA();
			}
			}
		}
		else if (addr < 0x4020)
		{
			switch (addr)
			{
			case 0x4016:
			{
				if (strobe)
				{
					return sf::Keyboard::isKeyPressed(sf::Keyboard::K) & 0x1;
				}
				u8 buf = joystate;
				joystate = 0x80 | (joystate >> 1);
				buf = (buf & 0x1) | 0x40;
				return buf;
			}
			}
			return 0;
		}
		else if (addr > 0x7FFF)
		{
			return GamePak::readPRGROM(addr - 0x8000);
		}
		return memory[addr];
	}

//Addresing modes
inline	u16 imm()
	{
		programCounter += 1;
		return programCounter;
	}
inline	u16 zp()
	{
		programCounter += 1;
		return read(programCounter);
	}
inline	u16 zpx()
	{
		programCounter += 1;
		return (read(programCounter) + xRegister) % 256;
	}
inline	u16 zpy()
	{
		programCounter += 1;
		return (read(programCounter) + yRegister) % 256;
	}
inline	u16 abs()
	{
		programCounter += 2;
		return read(programCounter - 1) + read(programCounter) * 256;
	}
inline	u16 abx()
	{
		programCounter += 2;
		return read(programCounter - 1) + read(programCounter) * 256 + xRegister;
	}
inline	u16 aby()
	{
		programCounter += 2;
		return read(programCounter - 1) + read(programCounter) * 256 + yRegister;
	}
inline	u16 inx()
	{
		programCounter += 1;
		return read((read(programCounter) + xRegister) % 256) + read((read(programCounter) + xRegister + 1) % 256) * 256;
	}
inline	u16 iny()
	{
		programCounter += 1;
		return read(read(programCounter)) + read((read(programCounter) + 1) % 256) * 256 + yRegister;
	}
inline	u16 rel()
	{
		return imm();
	}
inline	u16 ind()
	{
		programCounter += 2;
		return read(read(programCounter - 1) + read(programCounter) * 256) + read(((read(programCounter - 1) + 1) % 256) + read(programCounter) * 256) * 256;
	}

//CPU instructions
	//-----------//
	u8 operand = 0;
	u16 result = 0;
	//-----------//

	void ADC(u16 addr)
	{
		operand = read(addr);
		result = accRegister + operand + statusFlag.C * 1;
		statusFlag.V = ~(accRegister ^ operand) & (accRegister ^ result) & 0x80;
		statusFlag.C = result > 0xFF;
		accRegister = result & 0xFF;
		checkNZFlags(accRegister);
	}
	void AND(u16 addr)
	{
		accRegister &= read(addr);
		checkNZFlags(accRegister);
	}
	void ASL(u16 addr)
	{
		operand = read(addr);
		statusFlag.C = operand & 0x80;
		operand = (operand << 1) & 0xFF;
		checkNZFlags(operand);
		write(addr, operand);
	}
	void ASL_A()
	{
		statusFlag.C = accRegister & 0x80;
		accRegister = (accRegister << 1) & 0xFF;
		checkNZFlags(accRegister);
	}
	void BCC(u16 addr)
	{
		if (!statusFlag.C)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BCS(u16 addr)
	{
		if (statusFlag.C)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BEQ(u16 addr)
	{
		if (statusFlag.Z)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BIT(u16 addr)
	{
		operand = read(addr);
		statusFlag.V = operand & 0x40;
		statusFlag.N = operand & 0x80;
		operand = operand & accRegister;
		statusFlag.Z = operand == 0;
	}
	void BMI(u16 addr)
	{
		if (statusFlag.N)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BNE(u16 addr)
	{
		if (!statusFlag.Z)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BPL(u16 addr)
	{
		if (!statusFlag.N)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BRK()
	{
		//not yet implemented
	}
	void BVC(u16 addr)
	{
		if (!statusFlag.V)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void BVS(u16 addr)
	{
		if (statusFlag.V)
		{
			operand = read(addr);
			if (operand > 127)
			{
				programCounter -= 256 - operand;
			}
			else
			{
				programCounter += operand;
			}
		}
	}
	void CLC()
	{
		statusFlag.C = false;
	}
	void CLD()
	{
		statusFlag.D = false;
	}
	void CLI()
	{
		statusFlag.I = false;
	}
	void CLV()
	{
		statusFlag.V = false;
	}
	void CMP(u16 addr)
	{
		operand = read(addr);
		statusFlag.Z = accRegister == operand;
		statusFlag.C = accRegister >= operand;
		if (accRegister >= operand)
		{
			statusFlag.C = true;
			operand = accRegister - operand;
		}
		else
		{
			statusFlag.C = false;
			operand = 256 - operand + accRegister;
		}
		statusFlag.N = operand & 0x80;
	}
	void CPX(u16 addr)
	{
		operand = read(addr);
		statusFlag.Z = xRegister == operand;
		statusFlag.C = xRegister >= operand;
		if (xRegister >= operand)
		{
			statusFlag.C = true;
			operand = xRegister - operand;
		}
		else
		{
			statusFlag.C = false;
			operand = 256 - operand + xRegister;
		}
		statusFlag.N = operand & 0x80;
	}
	void CPY(u16 addr)
	{
		operand = read(addr);
		statusFlag.Z = yRegister == operand;
		statusFlag.C = yRegister >= operand;
		if (yRegister >= operand)
		{
			statusFlag.C = true;
			operand = yRegister - operand;
		}
		else
		{
			statusFlag.C = false;
			operand = 256 - operand + yRegister;
		}
		statusFlag.N = operand & 0x80;
	}
	void DEC(u16 addr)
	{
		operand = read(addr);
		operand--;
		checkNZFlags(operand);
		write(addr, operand);
	}
	void DEX()
	{
		xRegister--;
		checkNZFlags(xRegister);
	}
	void DEY()
	{
		yRegister--;
		checkNZFlags(yRegister);
	}
	void EOR(u16 addr)
	{
		accRegister = accRegister ^ read(addr);
		checkNZFlags(accRegister);
	}
	void INC(u16 addr)
	{
		operand = read(addr);
		operand++;
		checkNZFlags(operand);
		write(addr, operand);
	}
	void INX()
	{
		xRegister++;
		checkNZFlags(xRegister);
	}
	void INY()
	{
		yRegister++;
		checkNZFlags(yRegister);
	}
	void JMP(u16 addr)
	{
		programCounter = addr - 1;
	}
	void JSR(u16 addr)
	{
		u8 a = (programCounter) & 0xFF;
		u8 b = (programCounter) >> 8;
		write(stackPointer + 0x100, b);
		stackPointer--;
		write(stackPointer + 0x100, a);
		stackPointer--;
		programCounter = addr - 1;
	}
	void LDA(u16 addr)
	{
		accRegister = read(addr);
		checkNZFlags(accRegister);
	}
	void LDX(u16 addr)
	{
		xRegister = read(addr);
		checkNZFlags(xRegister);
	}
	void LDY(u16 addr)
	{
		yRegister = read(addr);
		checkNZFlags(yRegister);
	}
	void LSR(u16 addr)
	{
		operand = read(addr);
		statusFlag.C = operand & 0x1;
		operand = operand >> 1;
		checkNZFlags(operand);
		write(addr, operand);
	}
	void LSR_A()
	{
		statusFlag.C = accRegister & 0x1;
		accRegister = accRegister >> 1;
		checkNZFlags(accRegister);
	}
	void NOP(u16 addr = 0)
	{
		//OOPS NOTHING TO DO HERE
	}
	void ORA(u16 addr)
	{
		accRegister = accRegister | read(addr);
		checkNZFlags(accRegister);
	}
	void PHA()
	{
		write(stackPointer + 0x100, accRegister);
		stackPointer--;
	}
	void PHP()
	{
		u8 buf = (getFlags() | 0x30);
		write(stackPointer + 0x100, buf);
		stackPointer--;
	}
	void PLA()
	{
		stackPointer++;
		accRegister = read(stackPointer + 0x100);
		checkNZFlags(accRegister);
	}
	void PLP()
	{
		stackPointer++;
		setFlags(read(stackPointer + 0x100));
	}
	void ROL(u16 addr)
	{
		operand = read(addr);
		bool oldFlag = statusFlag.C;
		statusFlag.C = operand & 0x80;
		operand = ((operand << 1) + oldFlag * 0x1) & 0xFF;
		checkNZFlags(operand);
		write(addr, operand);
	}
	void ROL_A()
	{
		operand = accRegister;
		bool oldFlag = statusFlag.C;
		statusFlag.C = operand & 0x80;
		operand = ((operand << 1) + oldFlag * 0x1) & 0xFF;
		accRegister = operand & 0xFF;
		checkNZFlags(accRegister);
	}
	void ROR(u16 addr)
	{
		operand = read(addr);
		bool oldFlagC = statusFlag.C;
		statusFlag.C = operand & 0x1;
		operand = (operand >> 1) + 128 * oldFlagC;
		checkNZFlags(operand);
		write(addr, operand);
	}
	void ROR_A()
	{
		bool oldFlagC = statusFlag.C;
		statusFlag.C = accRegister & 0x1;
		accRegister = (accRegister >> 1) + 128 * oldFlagC;
		checkNZFlags(accRegister);
	}
	void RTS()
	{
		stackPointer++;
		u8 a = read(stackPointer + 0x100);
		stackPointer++;
		u8 b = read(stackPointer + 0x100);
		programCounter = a + b * 256;
	}
	void RTI()
	{
		//TODO think about cycles
		PLP();
		RTS();
		programCounter--;
	}
	void SBC(u16 addr)
	{
		operand = read(addr);
		signed char value = operand ^ 0xFF;
		int oldAccRegister = accRegister;
		char result = accRegister + value + 1 * statusFlag.C;
		statusFlag.V = (oldAccRegister ^ result) & (value ^ result) & 0x80;
		int carryCheck = unsigned char(oldAccRegister) + unsigned char(value) + statusFlag.C;
		if (carryCheck > 255)
		{
			statusFlag.C = true;
		}
		else
		{
			statusFlag.C = false;
		}
		accRegister = result + 256;
		checkNZFlags(accRegister);
	}
	void SEC()
	{
		statusFlag.C = true;
	}
	void SED()
	{
		statusFlag.D = true;
	}
	void SEI()
	{
		statusFlag.I = true;
	}
	void STA(u16 addr)
	{
		write(addr, accRegister);
	}
	void STX(u16 addr)
	{
		write(addr, xRegister);
	}
	void STY(u16 addr)
	{
		write(addr, yRegister);
	}
	void TAX()
	{
		xRegister = accRegister;
		checkNZFlags(xRegister);
	}
	void TAY()
	{
		yRegister = accRegister;
		checkNZFlags(yRegister);
	}
	void TSX()
	{
		xRegister = stackPointer;
		checkNZFlags(xRegister);
	}
	void TXA()
	{
		accRegister = xRegister;
		checkNZFlags(accRegister);
	}
	void TXS()
	{
		stackPointer = xRegister;
	}
	void TYA()
	{
		accRegister = yRegister;
		checkNZFlags(accRegister);
	}

	void handleNMI()
	{
		statusFlag.B = false;
		SEI();
		JSR(read(0xFFFA) + read(0xFFFB) * 256 + 1);
		PHP();
	}

	void op()
	{
		int cycleBuf = cycleCount[read(programCounter)];	
		switch(read(programCounter))
		{
		case 0x69:
		{
			ADC(imm());
			break;
		}
		case 0x65:
		{
			ADC(zp());
			break;
		}
		case 0x75:
		{
			ADC(zpx());
			break;
		}
		case 0x6D:
		{
			ADC(abs());
			break;
		}
		case 0x7D:
		{
			ADC(abx());
			break;
		}
		case 0x79:
		{
			ADC(aby());
			break;
		}
		case 0x61:
		{
			ADC(inx());
			break;
		}
		case 0x71:
		{
			ADC(iny());
			break;
		}
		case 0x29:
		{
			AND(imm());
			break;
		}
		case 0x25:
		{
			AND(zp());
			break;
		}
		case 0x35:
		{
			AND(zpx());
			break;
		}
		case 0x2D:
		{
			AND(abs());
			break;
		}
		case 0x3D:
		{
			AND(abx());
			break;
		}
		case 0x39:
		{
			AND(aby());
			break;
		}
		case 0x21:
		{
			AND(inx());
			break;
		}
		case 0x31:
		{
			AND(iny());
			break;
		}
		case 0x0A:
		{
			ASL_A();
			break;
		}
		case 0x06:
		{
			ASL(zp());
			break;
		}
		case 0x16:
		{
			ASL(zpx());
			break;
		}
		case 0x0E:
		{
			ASL(abs());
			break;
		}
		case 0x1E:
		{
			ASL(abx());
			break;
		}
		case 0x90:
		{
			BCC(rel());
			break;
		}
		case 0xB0:
		{
			BCS(rel());
			break;
		}
		case 0xF0:
		{
			BEQ(rel());
			break;
		}
		case 0x24:
		{
			BIT(zp());
			break;
		}
		case 0x2C:
		{
			BIT(abs());
			break;
		}
		case 0x30:
		{
			BMI(rel());
			break;
		}
		case 0xD0:
		{
			BNE(rel());
			break;
		}
		case 0x10:
		{
			BPL(rel());
			break;
		}
		case 0x00:
		{
			BRK();
			break;
		}
		case 0x50:
		{
			BVC(rel());
			break;
		}
		case 0x70:
		{
			BVS(rel());
			break;
		}
		case 0x18:
		{
			CLC();
			break;
		}
		case 0xD8:
		{
			CLD();
			break;
		}
		case 0x58:
		{
			CLI();
			break;
		}
		case 0xB8:
		{
			CLV();
			break;
		}
		case 0xC9:
		{
			CMP(imm());
			break;
		}
		case 0xC5:
		{
			CMP(zp());
			break;
		}
		case 0xD5:
		{
			CMP(zpx());
			break;
		}
		case 0xCD:
		{
			CMP(abs());
			break;
		}
		case 0xDD:
		{
			CMP(abx());
			break;
		}
		case 0xD9:
		{
			CMP(aby());
			break;
		}
		case 0xC1:
		{
			CMP(inx());
			break;
		}
		case 0xD1:
		{
			CMP(iny());
			break;
		}
		case 0xE0:
		{
			CPX(imm());
			break;
		}
		case 0xE4:
		{
			CPX(zp());
			break;
		}
		case 0xEC:
		{
			CPX(abs());
			break;
		}
		case 0xC0:
		{
			CPY(imm());
			break;
		}
		case 0xC4:
		{
			CPY(zp());
			break;
		}
		case 0xCC:
		{
			CPY(abs());
			break;
		}
		case 0xC6:
		{
			DEC(zp());
			break;
		}
		case 0xD6:
		{
			DEC(zpx());
			break;
		}
		case 0xCE:
		{
			DEC(abs());
			break;
		}
		case 0xDE:
		{
			DEC(abx());
			break;
		}
		case 0xCA:
		{
			DEX();
			break;
		}
		case 0x88:
		{
			DEY();
			break;
		}
		case 0x49:
		{
			EOR(imm());
			break;
		}
		case 0x45:
		{
			EOR(zp());
			break;
		}
		case 0x55:
		{
			EOR(zpx());
			break;
		}
		case 0x4D:
		{
			EOR(abs());
			break;
		}
		case 0x5D:
		{
			EOR(abx());
			break;
		}
		case 0x59:
		{
			EOR(aby());
			break;
		}
		case 0x41:
		{
			EOR(inx());
			break;
		}
		case 0x51:
		{
			EOR(iny());
			break;
		}
		case 0xE6:
		{
			INC(zp());
			break;
		}
		case 0xF6:
		{
			INC(zpx());
			break;
		}
		case 0xEE:
		{
			INC(abs());
			break;
		}
		case 0xFE:
		{
			INC(abx());
			break;
		}
		case 0xE8:
		{
			INX();
			break;
		}
		case 0xC8:
		{
			INY();
			break;
		}
		case 0x4C:
		{
			JMP(abs());
			break;
		}
		case 0x6C:
		{
			JMP(ind());
			break;
		}
		case 0x20:
		{
			JSR(abs());
			break;
		}
		case 0xA9:
		{
			LDA(imm());
			break;
		}
		case 0xA5:
		{
			LDA(zp());
			break;
		}
		case 0xB5:
		{
			LDA(zpx());
			break;
		}
		case 0xAD:
		{
			LDA(abs());
			break;
		}
		case 0xBD:
		{
			LDA(abx());
			break;
		}
		case 0xB9:
		{
			LDA(aby());
			break;
		}
		case 0xA1:
		{
			LDA(inx());
			break;
		}
		case 0xB1:
		{
			LDA(iny());
			break;
		}
		case 0xA2:
		{
			LDX(imm());
			break;
		}
		case 0xA6:
		{
			LDX(zp());
			break;
		}
		case 0xB6:
		{
			LDX(zpy());
			break;
		}
		case 0xAE:
		{
			LDX(abs());
			break;
		}
		case 0xBE:
		{
			LDX(aby());
			break;
		}
		case 0xA0:
		{
			LDY(imm());
			break;
		}
		case 0xA4:
		{
			LDY(zp());
			break;
		}
		case 0xB4:
		{
			LDY(zpx());
			break;
		}
		case 0xAC:
		{
			LDY(abs());
			break;
		}
		case 0xBC:
		{
			LDY(abx());
			break;
		}
		case 0x4A:
		{
			LSR_A();
			break;
		}
		case 0x46:
		{
			LSR(zp());
			break;
		}
		case 0x56:
		{
			LSR(zpx());
			break;
		}
		case 0x4E:
		{
			LSR(abs());
			break;
		}
		case 0x5E:
		{
			LSR(abx());
			break;
		}
		case 0xEA:
		{
			NOP();
			break;
		}
		case 0x09:
		{
			ORA(imm());
			break;
		}
		case 0x05:
		{
			ORA(zp());
			break;
		}
		case 0x15:
		{
			ORA(zpx());
			break;
		}
		case 0x0D:
		{
			ORA(abs());
			break;
		}
		case 0x1D:
		{
			ORA(abx());
			break;
		}
		case 0x19:
		{
			ORA(aby());
			break;
		}
		case 0x01:
		{
			ORA(inx());
			break;
		}
		case 0x11:
		{
			ORA(iny());
			break;
		}
		case 0x48:
		{
			PHA();
			break;
		}
		case 0x08:
		{
			PHP();
			break;
		}
		case 0x68:
		{
			PLA();
			break;
		}
		case 0x28:
		{
			PLP();
			break;
		}
		case 0x2A:
		{
			ROL_A();
			break;
		}
		case 0x26:
		{
			ROL(zp());
			break;
		}
		case 0x36:
		{
			ROL(zpx());
			break;
		}
		case 0x2E:
		{
			ROL(abs());
			break;
		}
		case 0x3E:
		{
			ROL(abx());
			break;
		}
		case 0x6A:
		{
			ROR_A();
			break;
		}
		case 0x66:
		{
			ROR(zp());
			break;
		}
		case 0x76:
		{
			ROR(zpx());
			break;
		}
		case 0x6E:
		{
			ROR(abs());
			break;
		}
		case 0x7E:
		{
			ROR(abx());
			break;
		}
		case 0x40:
		{
			RTI();
			break;
		}
		case 0x60:
		{
			RTS();
			break;
		}
		case 0xE9:
		{
			SBC(imm());
			break;
		}
		case 0xE5:
		{
			SBC(zp());
			break;
		}
		case 0xF5:
		{
			SBC(zpx());
			break;
		}
		case 0xED:
		{
			SBC(abs());
			break;
		}
		case 0xFD:
		{
			SBC(abx());
			break;
		}
		case 0xF9:
		{
			SBC(aby());
			break;
		}
		case 0xE1:
		{
			SBC(inx());
			break;
		}
		case 0xF1:
		{
			SBC(iny());
			break;
		}
		case 0x38:
		{
			SEC();
			break;
		}
		case 0xF8:
		{
			SED();
			break;
		}
		case 0x78:
		{
			SEI();
			break;
		}
		case 0x85:
		{
			STA(zp());
			break;
		}
		case 0x95:
		{
			STA(zpx());
			break;
		}
		case 0x8D:
		{
			STA(abs());
			break;
		}
		case 0x9D:
		{
			STA(abx());
			break;
		}
		case 0x99:
		{
			STA(aby());
			break;
		}
		case 0x81:
		{
			STA(inx());
			break;
		}
		case 0x91:
		{
			STA(iny());
			break;
		}
		case 0x86:
		{
			STX(zp());
			break;
		}
		case 0x96:
		{
			STX(zpy());
			break;
		}
		case 0x8E:
		{
			STX(abs());
			break;
		}
		case 0x84:
		{
			STY(zp());
			break;
		}
		case 0x94:
		{
			STY(zpx());
			break;
		}
		case 0x8C:
		{
			STY(abs());
			break;
		}
		case 0xAA:
		{
			TAX();
			break;
		}
		case 0xA8:
		{
			TAY();
			break;
		}
		case 0xBA:
		{
			TSX();
			break;
		}
		case 0x8A:
		{
			TXA();
			break;
		}
		case 0x9A:
		{
			TXS();
			break;
		}
		case 0x98:
		{
			TYA();
			break;
		}
		}
		programCounter++;
		for (int i = 0; i < cycleBuf; i++)
		{
			PPU::tick();
			PPU::tick();
			PPU::tick();
		}
	}

	void init()
	{
		APU::init();
		PPU::init();
		stackPointer = 0xFD;
		setFlags(0x34);
		programCounter = read(0xFFFC) + read(0xFFFD) * 256;
	}
}