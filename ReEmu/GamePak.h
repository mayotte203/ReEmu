#pragma once
#include "common.h"
#include <fstream>
namespace GamePak
{
	bool getMirroring();
	int loadFromFile(std::string filename);
	u8 readCHRROM(u16 addr);
	u8 readPRGROM(u16 addr);
}
