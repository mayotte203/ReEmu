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

	enum State
	{
		MainMenu,

		Options,
		Emulator,
		PauseMenu
	};

	void start()
	{
		sf::RenderWindow window(sf::VideoMode(512, 480), "ReEmu");
		//GamePak::loadFromFile("Super_Mario_Bros._(E).nes");
		GamePak::loadFromFile("Donkey_Kong.nes");
		//GamePak::loadFromFile("color_test.nes");
		//GamePak::loadFromFile("Arkanoid (U).nes");
		//GamePak::loadFromFile("Tetris (USA).nes");
		//GamePak::loadFromFile("Ice Climber (USA, Europe).nes");
		//GamePak::loadFromFile("battle-city.nes");
		//GamePak::loadFromFile("Pac - Man(USA) (Namco).nes");
		CPU::init();
		sf::Event event;
		sf::Clock clock;
		sf::Time elapsed;
		clock.restart();
		elapsed = sf::Time::Zero;
		bool renderState = false;
		while (window.isOpen())
		{
			State currentState = State::Emulator;
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}
			switch (currentState)
			{
			case MainMenu:
			{
				break;
			}
			case Options:
			{
				break;
			}
			case Emulator:
			{
				if (renderState)
				{
					while (!PPU::isVBlankOccured())
					{
						CPU::op();
					}
					renderState = false;
				}
				break;
			}
			case PauseMenu:
			{
				break;
			}
			}
			elapsed += clock.restart();
			if (elapsed.asMilliseconds() >= 1000.0 / 60.0)
			{
				elapsed = sf::Time::Zero;
				window.clear();
				window.draw(*PPU::getRenderSprite());
				window.display();
				renderState = true;
			}
		}
		window.close();
	}
}