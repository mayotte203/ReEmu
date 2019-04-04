#include "PPU.h"

u8 OAM[0x100];
namespace PPU
{
	sf::RenderWindow *renderWindow;
	u8 spriteData[256];
	u8 renderBuffer[240][256];
	sf::Image renderImage;
	sf::Texture renderTexture;
	sf::Sprite renderSprite;
	u8 memory[0x3FFF];

	struct PPUCTRL
	{
		bool VBlankNMI = 0;
		bool P = 0;
		bool H = 0;
		bool B = 0;
		bool S = 0;
		bool I = 0;
		bool NX = 0;
		bool NY = 0;
	}PPUCTRL;

	struct PPUMASK
	{
		bool eBlue = 0;
		bool eGreen = 0;
		bool eRed = 0;
		bool showSprites = 0;
		bool showBackground = 0;
		bool showSprites8 = 0;
		bool showBackgorund8 = 0;
		bool greyScale = 0;
	}PPUMASK;

	struct PPUSTATUS
	{
		bool VBlank = 0;
		bool sprite0 = 0;
		bool spriteOverflow = 0;
	}PPUSTATUS;
	
	u8 OAMADDR = 0;
	u8 PPUSCROLLX = 0;
	u8 PPUSCROLLY = 0;
	u16 PPUADDR = 0;
	bool writeLatch = 0;

	void write(u16 addr, u8 data)
	{

	}

	u8 read(u16 addr)
	{
		if (addr < 0x4000)
		{
			return memory[addr];
		}
		return 0;
	}

	void writePPUCTRL(u8 data)
	{
		PPUCTRL.VBlankNMI = data & 0x80;
		PPUCTRL.P = data & 0x40;
		PPUCTRL.H = data & 0x20;
		PPUCTRL.B = data & 0x10;
		PPUCTRL.S = data & 0x8;
		PPUCTRL.I = data & 0x4;
		PPUCTRL.NX = data & 0x2;
		PPUCTRL.NY = data & 0x1;
	}

	void writePPUMASK(u8 data)
	{
		PPUMASK.eBlue = data & 0x80;
		PPUMASK.eGreen = data & 0x40;
		PPUMASK.eRed = data & 0x20;
		PPUMASK.showSprites = data & 0x10;
		PPUMASK.showBackground = data & 0x8;
		PPUMASK.showSprites8 = data & 0x4;
		PPUMASK.showBackgorund8 = data & 0x2;
		PPUMASK.greyScale = data & 0x1;
	}

	u8 readPPUSTATUS()
	{
		writeLatch = 0;
		bool oldVBlank = PPUSTATUS.VBlank;
		PPUSTATUS.VBlank = false;
		return 0x80 * oldVBlank + 0x40 * PPUSTATUS.spriteOverflow + 0x20 * PPUSTATUS.spriteOverflow;
	}

	void writeOAMADDR(u8 data)
	{
		OAMADDR = data;
	}

	u8 readOAMADDR()
	{
		return OAMADDR;
	}

	void writeOAMDATA(u8 data)
	{
		OAM[OAMADDR++] = data;
	}

	u8 readOAMDATA()
	{
		return OAM[OAMADDR];
	}

	void writePPUSCROLL(u8 data)
	{
		if (writeLatch)
		{
			PPUSCROLLY = data;
		}
		else
		{
			PPUSCROLLX = data;
		}
		writeLatch = !writeLatch;
	}

	void writePPUADDR(u8 data)
	{
		if (writeLatch)
		{
			PPUADDR = (PPUADDR & 0xFF00) + data;
		}
		else
		{
			PPUADDR = (PPUADDR & 0xFF) + (data * 0x100);
		}
		writeLatch = !writeLatch;
	}

	void setvstat(bool data)
	{
		PPUSTATUS.VBlank = data;
	}
	void setsprite(bool data)
	{
		PPUSTATUS.sprite0 = data;
	}
	void writePPUDATA(u8 data)
	{
		if (PPUADDR < 0x4000)
		{
			memory[PPUADDR] = data;
		}
		if (PPUCTRL.I)
		{
			PPUADDR += 32;
		}
		else
		{
			PPUADDR += 1;
		}
	}

	u8 readPPUDATA()
	{
		u8 buf;
		if (PPUADDR < 0x4000)
		{
			buf = memory[PPUADDR];
		}
		buf = 0;

		if (PPUCTRL.I)
		{
			PPUADDR += 32;
		}
		else
		{
			PPUADDR += 1;
		}

		return buf;
	}

	void OAMDMA(u8 data[0x100])
	{
		for (int i = 0; i < 256; i++)
		{
			OAM[OAMADDR++] = data[i];
		}
	}

	void assignWindow(sf::RenderWindow *window)
	{
		renderWindow = window;
		renderImage.create(256, 240, sf::Color::Red);
		renderTexture.loadFromImage(renderImage);
		renderSprite.setTexture(renderTexture);
		renderSprite.setPosition(sf::Vector2f(256.0, 0.0));
	}

	void tick()
	{

	}

	void draw()
	{
		renderWindow->draw(renderSprite);
	}
}