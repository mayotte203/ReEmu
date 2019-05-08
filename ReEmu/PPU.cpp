#include "PPU.h"

namespace PPU
{
	bool VBlankOccured = false;
	bool mirroring = VERTICAL;
	bool writeLatch = 0;
	sf::RenderWindow *renderWindow;
	sf::Image renderImage;
	sf::Texture renderTexture;
	sf::Sprite renderSprite;
	u8 memory[0x4000];
	u8 OAM[0x100];
	int scanlineCount = 0;
	int cycleCount = 0;
	int renderX = 0;
	int renderY = 0;
	u8 OAMADDR = 0;
	u8 PPUSCROLLX = 0;
	u8 PPUSCROLLY = 0;
	u16 PPUADDR = 0;

	struct PPUCTRL
	{
		bool VBlankNMI = 0;
		bool MSSelect = 0;
		bool spriteSize = 0;
		bool backgroundPatternTable = 0;
		bool spritePaternTable = 0;
		bool VRAMIncrement = 0;
		bool NY = 0;
		bool NX = 0;
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
	
	void pixel();

	bool isVBlankOccured() 
	{
		bool oldVBlank = VBlankOccured;
		VBlankOccured = false;
		return oldVBlank;
	}

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
		PPUCTRL.NY = data & 0x2;
		PPUCTRL.NX = data & 0x1;
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

	void tick()
	{
		if (scanlineCount == 0)
		{
			renderY = 0;
		}
		else if (scanlineCount > 0 && scanlineCount < 241)
		{
			if (cycleCount == 0)
			{
				renderX = 0;
			}
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
		else if (scanlineCount == 241 && cycleCount == 0)
		{
			PPUSTATUS.VBlank = true;
			if (PPUCTRL.VBlankNMI)
			{
				CPU::handleNMI();
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
				renderTexture.update(renderImage);
				VBlankOccured = true;
				PPUSTATUS.VBlank = false;
				PPUSTATUS.sprite0 = false;
			}
		}
	}
	
	void init()
	{
		renderImage.create(256, 240);
		renderTexture.loadFromImage(renderImage);
		renderSprite.setTexture(renderTexture);
		renderSprite.setOrigin(128, 120);
		renderSprite.setPosition(256, 240);
	}

	sf::Sprite* getRenderSprite()
	{
		return &renderSprite;
	}

	void pixel()
	{
		int backgroundX = (PPUCTRL.NX * 256 + PPUSCROLLX + renderX) % 512;
		int backgroundY = (PPUCTRL.NY * 240 + PPUSCROLLY + renderY) % 480;
		int backgroundXOffset = backgroundX % 8;
		int backgroundYOffset = backgroundY % 8;
		int currentNametable = mirroring == HORIZONTAL ? 2 * (backgroundY / 240) : (backgroundX / 256);
		int currentNametableEntry = ((backgroundX % 256) / 8) + (((backgroundY % 240) / 8) * 32);
		int currentBackgroundSprite = read(0x2000 + currentNametable * 0x400 + currentNametableEntry);
		int currentBackgroundLine = 16 * currentBackgroundSprite + backgroundYOffset + PPUCTRL.backgroundPatternTable * 0x1000;
		int colorBackground = ((GamePak::readCHRROM(currentBackgroundLine) << backgroundXOffset) & 0x80)
			+ 2 * ((GamePak::readCHRROM(currentBackgroundLine + 8) << backgroundXOffset) & 0x80);
		int currentSprite = -1;
		for (int i = 0; i < 64; i++)
		{
			if (OAM[4 * i + 3] <= renderX && OAM[4 * i + 3] + 7 >= renderX && OAM[4 * i] <= renderY && OAM[4 * i] + 7 >= renderY)
			{
				currentSprite = i;
				break;
			}
		}
		int currentAttributeEntry = read(0x23C0 + currentNametable * 0x400 + (((currentNametableEntry) / 128) * 8)
										+ (((currentNametableEntry) % 32) / 4));
		int attributeOffset = 2 * ((((currentNametableEntry) % 4) / 2) + 2 * (((currentNametableEntry) / 32) % 2));
		int currentBackgroundPalette = (currentAttributeEntry >> attributeOffset) & 0x3;
		if (PPUMASK.showBackground)
		{
			switch (colorBackground)
			{
			case 0x180:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read(currentBackgroundPalette * 4 + 0x3F03)]);
				break;
			}
			case 0x100:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read(currentBackgroundPalette * 4 + 0x3F02)]);
				break;
			}
			case 0x80:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read(currentBackgroundPalette * 4 + 0x3F01)]);
				break;
			}
			case 0x0:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read(0x3F00)]);
				break;
			}
			}
		}
		else
		{
			renderImage.setPixel(renderX, renderY, NTSCPalette[0x3f]);
		}
		if (currentSprite > -1 && PPUMASK.showSprites)
		{
			int xOffset = renderX - OAM[4 * currentSprite + 3];
			int yOffset = renderY - OAM[4 * currentSprite];
			int spriteRenderLine = 16 * OAM[4 * currentSprite + 1] + (OAM[4 * currentSprite + 2] & 0x80 ? 7 - yOffset : yOffset) 
				+ PPUCTRL.spritePaternTable * 0x1000;
			int spritePixelOffset = OAM[4 * currentSprite + 2] & 0x40 ? 7 - xOffset : xOffset;
			int colorSprite = ((GamePak::readCHRROM(spriteRenderLine) << spritePixelOffset) & 0x80)
				+ 2 * ((GamePak::readCHRROM(spriteRenderLine + 8) << spritePixelOffset) & 0x80);
			switch (colorSprite)
			{
			case 0x180:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read((OAM[4 * currentSprite + 2] & 0x03) * 4 + 0x3F13)]);
				break;
			}
			case 0x100:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read((OAM[4 * currentSprite + 2] & 0x03) * 4 + 0x3F12)]);
				break;
			}
			case 0x80:
			{
				renderImage.setPixel(renderX, renderY, NTSCPalette[read((OAM[4 * currentSprite + 2] & 0x03) * 4 + 0x3F11)]);
				break;
			}
			case 0x0:
			{
				if (currentSprite == 0 && PPUMASK.showBackground && PPUMASK.showSprites && colorBackground == 0)
				{
					PPUSTATUS.sprite0 = true;
				}
			}
			}
		}
	}
}