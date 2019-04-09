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
	u8 memory[0x4000];
	int scanlineCount = 0;
	int cycleCount = 0;
	int renderX = 0;
	int renderY = 0;
	void pixel();

	struct PPUCTRL
	{
		bool VBlankNMI = 0;
		bool MSSelect = 0;
		bool spriteSize = 0;
		bool backgroundPatternTable = 0;
		bool spritePaternTable = 0;
		bool VRAMIncrement = 0;
		bool NX = 0;
		bool NY = 0;
	}PPUCTRL;

	struct PPUMASK
	{
		bool emphasizeBlue = 0;
		bool emphasizeGreen = 0;
		bool emphasizeRed = 0;
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
		PPUCTRL.MSSelect = data & 0x40;
		PPUCTRL.spriteSize = data & 0x20;
		PPUCTRL.backgroundPatternTable = data & 0x10;
		PPUCTRL.spritePaternTable = data & 0x8;
		PPUCTRL.VRAMIncrement = data & 0x4;
		PPUCTRL.NX = data & 0x2;
		PPUCTRL.NY = data & 0x1;
	}

	void writePPUMASK(u8 data)
	{
		PPUMASK.emphasizeBlue = data & 0x80;
		PPUMASK.emphasizeGreen = data & 0x40;
		PPUMASK.emphasizeRed = data & 0x20;
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
		return 0x80 * oldVBlank + 0x40 * PPUSTATUS.sprite0 + 0x20 * PPUSTATUS.spriteOverflow;
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
		PPUADDR += PPUCTRL.VRAMIncrement ? 32 : 1;
	}

	u8 readPPUDATA()
	{
		u8 buf;
		if (PPUADDR < 0x4000)
		{
			buf = memory[PPUADDR];
		}
		buf = 0;
		PPUADDR += PPUCTRL.VRAMIncrement ? 32 : 1;
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
		if (scanlineCount > 0 && scanlineCount < 241)
		{
			if (cycleCount > 0 && cycleCount < 257)
			{
				pixel();
				renderX++;
				if (renderX == 256)
				{
					renderX = 0;
					renderY++;
					if (renderY == 240)
					{
						renderY = 0;
					}
				}
			}
		}
		cycleCount++;
		if (cycleCount == 341)
		{
			cycleCount = 0;
			scanlineCount++;
			if (scanlineCount == 261)
			{
				scanlineCount = 0;
			}
		}
	}

	void draw()
	{
		renderTexture.loadFromImage(renderImage);
		renderWindow->draw(renderSprite);
	}

	void pixel()
	{
		int currentSprite = -1;
		for (int i = 0; i < 64; i++)
		{
			if (OAM[4 * i + 3] <= renderX && OAM[4 * i + 3] + 7 >= renderX && OAM[4 * i] <= renderY && OAM[4 * i] + 7 >= renderY)
			{
				currentSprite = i;
				break;
			}
		}
		if (currentSprite == -1)
		{
			renderImage.setPixel(renderX, renderY, sf::Color::Black);
			return;
		}
		int xOffset = renderX - OAM[4 * currentSprite + 3];
		int yOffset = renderY - OAM[4 * currentSprite];
		int color = ((GamePak::readCHRROM(16 * OAM[4 * currentSprite + 1] + yOffset) << xOffset) & 0x80)
			+ 2 * ((GamePak::readCHRROM(16 * OAM[4 * currentSprite + 1] + yOffset + 8) << xOffset) & 0x80);
		switch (color)
		{
		case 0x180:
		{
			renderImage.setPixel(renderX, renderY, sf::Color(255, 255, 255, 255));
			break;
		}
		case 0x100:
		{
			renderImage.setPixel(renderX, renderY, sf::Color(196, 196, 196, 255));
			break;
		}
		case 0x80:
		{
			renderImage.setPixel(renderX, renderY, sf::Color(128, 128, 128, 255));
			break;
		}
		case 0:
		{
			renderImage.setPixel(renderX, renderY, sf::Color(0, 0, 0, 0));
			break;
		}
		}
	}
}