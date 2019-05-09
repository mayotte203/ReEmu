#include "GamePak.h"

namespace GamePak
{
	u8 PRGROMSize = 0;
	u8 CHRROMSize = 0;
	bool mirroring = HORIZONTAL;
	bool mirroringIgnore = false;
	u8 mapper = 0;
	char PRGROM[8192 * 4];
	char CHRROM[8192 * 2];
	bool getMirroring()
	{
		return mirroring;
	}

	u8 readCHRROM(u16 addr)
	{
		if (addr > 8192 * 2)
		{
			return 0;
		}
		return CHRROM[addr];
	}

	u8 readPRGROM(u16 addr)
	{
		if (addr > 8192 * 4)
		{
			return 0;
		}
		return PRGROM[addr];
	}	

	int loadFromFile(std::string filename)
	{
		std::ifstream ROMFile = std::ifstream(filename, std::ios::binary);
		if (!ROMFile.is_open())
		{
			return 1;
		}
		char header[16];
		if (!ROMFile.read(header, 16))
		{
			return 2;
		}
		if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S')
		{
			return 3;
		}
		mirroring = header[6] & 0b00000001;
		mirroringIgnore = header[6] & 0b00001000;
		mapper = ((header[6] & 0b11110000) >> 4) + (header[7] & 0b11110000);
		if (mapper != 0)
		{
			return 3;
		}
		PRGROMSize = header[4];
		CHRROMSize = header[5];
		if (PRGROMSize > 2 || CHRROMSize > 2)
		{
			return 4;
		}
		if (!ROMFile.read(PRGROM, 2 * 8192 * PRGROMSize))
		{
			return 2;
		}
		if (!ROMFile.read(CHRROM, 8192 * CHRROMSize))
		{
			return 2;
		}
		if (PRGROMSize == 1)
		{
			for (int i = 0; i < 16384; i++)
			{
				PRGROM[i + 16384] = PRGROM[i];
			}
		}
		ROMFile.close();
		return 0;
	}
}