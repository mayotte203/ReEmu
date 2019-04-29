#include <iostream>
#include <fstream>
#include <cstdint>
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "CPU.h"
#include "PPU.h"
#include "common.h"
using namespace std;

int scounter = 0;

int main()
{
	sf::RenderWindow window(sf::VideoMode(512, 480), "ReEmu");
	CPU::init();
	PPU::assignWindow(&window);
	sf::Event event;
	sf::Clock clock;
	sf::Time elapsed;
	bool isVBlankOccured = false;
	while (window.isOpen())
	{
		sf::Event event; 
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		CPU::op();
		scounter++;
		if (scounter == 20000)
		{
			window.clear();
			PPU::draw();
			window.display();
			scounter = false;
		}
	}
	return 0;
}