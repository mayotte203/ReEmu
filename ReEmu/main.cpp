#include <iostream>
#include <fstream>
#include <cstdint>
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "CPU.h"
#include "PPU.h"
#include "common.h"
#include <dshow.h>
using namespace std;

extern u8 OAM[0x100];
extern sf::Sprite patternSprite[256];
extern sf::Sprite backgroundSprite[256];
int scounter = 0;

int main()
{
	sf::RenderWindow window(sf::VideoMode(256, 240), "ReEmu");
	CPU::init();
	sf::Event event;
	while (true)
	{
		
		CPU::op();
		scounter++;
		if (scounter == 7000)
		{
			window.pollEvent(event);
			if (event.type == sf::Event::Closed)
				window.close();
			window.clear();
			for (int i = 0; i < 960; i++)
			{				
				u8 spriteNumber = PPU::read(0x2000 + i);
				int x = 8 * (i % 32);
				int y = 8 * (int)(i / 32);
				backgroundSprite[spriteNumber].setPosition(x, y);
				window.draw(backgroundSprite[spriteNumber]);
			}
			
			for (int i = 0; i < 64; i++)
			{
				if (OAM[4 * i + 2] & 0x40)
				{
					patternSprite[OAM[4 * i + 1]].setScale(-1.0f, 1.0f);
					patternSprite[OAM[4 * i + 1]].setPosition(OAM[4 * i + 3] + 8, OAM[4 * i]);
				}
				else
				{
					patternSprite[OAM[4 * i + 1]].setScale(1.0f, 1.0f);
					patternSprite[OAM[4 * i + 1]].setPosition(OAM[4 * i + 3], OAM[4 * i]);
				}
				window.draw(patternSprite[OAM[4 * i + 1]]);
			}
			/*for (int i = 0; i < 256; i++)
			{
				backgroundSprite[i].setPosition(8 * (i % 16), 8 * (i / 16));
				window.draw(backgroundSprite[i]);
			}*/
			
			window.display();
			scounter = false;

		}
		
		
	}	
	return 0;
}