#include "GUI.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include "CPU.h"
#include "PPU.h"
#include "common.h"
#include <Windows.h>

using namespace std;

int scounter = 0;

namespace GUI
{
	void read_directory(const std::string& name)
	{
		std::string pattern(name);
		pattern.append("\\*");
		WIN32_FIND_DATA data;
		HANDLE hFind;
		if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != ((HANDLE)(LONG_PTR)-1)) {
			do {
				cout << data.cFileName << endl;
			} while (FindNextFileA(hFind, &data) != 0);
			FindClose(hFind);
		}
	}

	void start()
	{
		sf::Font font;
		font.loadFromFile("font.ttf");
		sf::RenderWindow window(sf::VideoMode(512, 480), "ReEmu");
		CPU::init();
		PPU::assignWindow(&window);
		sf::Event event;
		sf::Clock clock;
		sf::Time elapsed;
		sf::Text text;
		text.setFont(font);
		text.setString("Kal");
		text.setPosition(250.0, 300.0);
		text.setCharacterSize(24);
		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}
			window.clear();
			window.draw(text);
			window.display();
		}
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
			if (scounter == 7000)
			{
				window.clear();
				PPU::draw();
				window.display();
				scounter = false;
			}
		}
	}
}