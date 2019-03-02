#pragma once
#include "common.h"



namespace PPU
{
	void write(u16 addr, u8 data);
	u8 read(u16 addr);
	void setvstat(bool data);
	void writePPUCTRL(u8 data);
	void writePPUMASK(u8 data);
	u8 readPPUSTATUS();
	void writeOAMADDR(u8 data);
	u8 readOAMADDR();
	void writeOAMDATA(u8 data);
	u8 readOAMDATA();
	void writePPUSCROLL(u8 data);
	void writePPUADDR(u8 data);
	void writePPUDATA(u8 data);
	u8 readPPUDATA();
	void OAMDMA(u8 data[0x100]);
	void setsprite(bool data);
}