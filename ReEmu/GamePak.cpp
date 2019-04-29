#include "GamePak.h"
#include <fstream>

namespace GamePak
{
	u8 PRGROMSize = 0;
	u8 CHRROMSize = 0;
	bool mirroring = HORIZONTAL;
	bool mirroringIgnore = false;
	u8 mapper = 0;
	char PRGROM[8192 * 4];
	char CHRROM[8192 * 2];

	u8 readCHRROM(u16 addr)
	{
		return CHRROM[addr];
	}
	u8 readPRGROM(u16 addr)
	{
		return PRGROM[addr];
	}	

	void loadFromFile(const char* filename)
	{
		std::ifstream ROMFile = std::ifstream(filename, std::ios::binary);
		char header[16];;
		ROMFile.read(header, 16);
		mirroring = header[6] & 0b00000001;
		mirroringIgnore = header[6] & 0b00001000;
		mapper = ((header[6] & 0b11110000) >> 4) + (header[7] & 0b11110000);
		PRGROMSize = header[4];
		CHRROMSize = header[5];
		ROMFile.read(PRGROM, 2 * 8192 * PRGROMSize);
		ROMFile.read(CHRROM, 8192 * CHRROMSize);
		if (PRGROMSize == 1)
		{
			for (int i = 0; i < 16384; i++)
			{
				PRGROM[i + 16384] = PRGROM[i];
			}
		}
		ROMFile.close();
	}
}