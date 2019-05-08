#pragma once
#include "common.h"
#include <fstream>
namespace GamePak
{
	int loadFromFile(std::string filename);
	u8 readCHRROM(u16 addr);
	u8 readPRGROM(u16 addr);
}
