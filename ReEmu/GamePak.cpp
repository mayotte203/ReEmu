#include "GamePak.h"

namespace GamePak
{
	//CPU ROM size
	u8 PRGROMSize = 0;
	//PPU ROM size
	u8 CHRROMSize = 0;
	//Mirroring type
	bool mirroring = HORIZONTAL;
	bool mirroringIgnore = false;
	//Mapper number
	u8 mapper = 0;
	//ROMs
	char PRGROM[8192 * 16];
	char CHRROM[8192 * 2];

	u8 currentBank = 0;

	bool getMirroring()
	{
		return mirroring;
	}

	u8 readCHRROM(u16 addr)
	{
		if (addr > 0x2000)
		{
			return 0;
		}
		return CHRROM[addr];
	}

	void writePRGROM(u16 addr,u8 data)
	{
		currentBank = data & 0x7;
	}

	void writeCHRROM(u16 addr, u8 data)
	{
		if(addr < 0x2000)
		CHRROM[addr] = data;
	}

	u8 readPRGROM(u16 addr)
	{
		if (mapper == 2)
		{
			if (addr >= 0xC000)
			{
				return PRGROM[14 * 8192 + addr - 0xC000];
			}
			else
			{
				return PRGROM[currentBank * 2 * 8192 + addr - 0x8000];
			}
		}
		if (addr - 0x8000 > 8192 * 4)
		{
			return 0;
		}
		return PRGROM[addr - 0x8000];
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
		mirroring = !(header[6] & 0b00000001);
		mirroringIgnore = header[6] & 0b00001000;
		mapper = ((header[6] & 0b11110000) >> 4) + (header[7] & 0b11110000);
		if (mapper != 0 && mapper != 2)
		{
			return 3;
		}
		PRGROMSize = header[4];
		CHRROMSize = header[5];
		/*
		if (PRGROMSize > 2 || CHRROMSize > 2)
		{
			return 4;
		}
		*/
		if (!ROMFile.read(PRGROM, 2 * 8192 * PRGROMSize))
		{
			return 2;
		}
		if (mapper == 0)
		{
			if (!ROMFile.read(CHRROM, 8192 * CHRROMSize))
			{
				return 2;
			}
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