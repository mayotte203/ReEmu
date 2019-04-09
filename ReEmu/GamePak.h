#pragma once
#include "common.h"

namespace GamePak
{
	void loadFromFile(const char* filename);
	u8 readCHRROM(u16 addr);
	u8 readPRGROM(u16 addr);
}
