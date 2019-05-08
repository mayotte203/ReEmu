#include "GUI.h"
using namespace std;

namespace GUI
{
	const static string windowTitle = "ReEmu";
	const static string logoString = "ReEmu";
	const static string fileErrorOpen = "Can't open file";
	const static string fileErrorRead = "File read error";
	const static string fileErrorInvalid = "Invalid file";
	const static string fileErrorNotSupported = "Unsupported ROM file";
	const static string defaultStatusString = "Enter - start, Esc - menu";
	const static string loadROMString = "Load ROM";
	const static string optionsString = "Options";
	const static string exitString = "Exit";
	const static string aboutString = "About";
	const static int FILE_MENU_POSITIONS = 10;
	const static int MAIN_MENU_POSITIONS = 4;
	const static float MAX_FPS = 60.0;
	const static int MAX_FILENAME_LENGTH = 22;

	unsigned int fileMenuArrowPosition = 0;
	unsigned int fileMenuPosition = 0;
	unsigned int mainMenuArrowPosition = 0;
	vector<string> ROMsFileNames;

	sf::Image mainMenuBackgroundImage;
	sf::Image arrowImage;
	sf::Texture mainMenuBackgroundTexture;
	sf::Texture arrowTexture;
	sf::Sprite mainMenuBackgroundSprite;
	sf::Sprite arrowSprite;
	sf::Font logoFont;
	sf::Font mainFont;
	sf::Text fileNameText[FILE_MENU_POSITIONS];
	sf::Text statusText;
	sf::Text logoText;
	sf::Text loadROMText;
	sf::Text optionsText;
	sf::Text exitText;
	sf::Text aboutText;

	bool loadResources()
	{
		if (!logoFont.loadFromFile("resources/still-time.ttf"))
		{
			return false;
		}
		if (!mainFont.loadFromFile("resources/press-start.ttf"))
		{
			return false;
		}
		if (!mainMenuBackgroundImage.loadFromFile("resources/background.jpg"))
		{
			return false;
		}
		if (!arrowImage.loadFromFile("resources/arrow.png"))
		{
			return false;
		}
		return true;
	}
	void initResources()
	{
		PPU::getRenderSprite()->setScale(2.0, 2.0);

		mainMenuBackgroundTexture.loadFromImage(mainMenuBackgroundImage);
		mainMenuBackgroundSprite.setTexture(mainMenuBackgroundTexture);
		mainMenuBackgroundSprite.setOrigin(mainMenuBackgroundImage.getSize().x / 2.0, mainMenuBackgroundImage.getSize().y / 2.0);
		mainMenuBackgroundSprite.setPosition(512 / 2, 480 / 2);

		arrowTexture.loadFromImage(arrowImage);
		arrowSprite.setTexture(arrowTexture);
		arrowSprite.setPosition(50, 160);
		for (int i = 0; i < FILE_MENU_POSITIONS; i++)
		{
			fileNameText[i].setCharacterSize(18);
			fileNameText[i].setFont(mainFont);
			fileNameText[i].setPosition(100, 40 + 40 * i);
			fileNameText[i].setString("");
		}
		statusText.setFont(mainFont);
		statusText.setCharacterSize(18);
		statusText.setPosition(50, 440);
		statusText.setString(defaultStatusString);

		logoText.setFont(logoFont);
		logoText.setFillColor(sf::Color::Red);
		logoText.setCharacterSize(96);
		logoText.setString(logoString);
		logoText.setOrigin(logoText.getLocalBounds().width / 2.0, logoText.getLocalBounds().height / 2.0);
		logoText.setPosition(512 / 2, 100);

		loadROMText.setFont(mainFont);
		loadROMText.setCharacterSize(18);
		loadROMText.setString(loadROMString);
		loadROMText.setOrigin(loadROMText.getLocalBounds().width / 2.0, loadROMText.getLocalBounds().height / 2.0);
		loadROMText.setPosition(512 / 2, 200);

		optionsText.setFont(mainFont);
		optionsText.setCharacterSize(18);
		optionsText.setString(optionsString);
		optionsText.setOrigin(optionsText.getLocalBounds().width / 2.0, optionsText.getLocalBounds().height / 2.0);
		optionsText.setPosition(512 / 2, 250);

		aboutText.setFont(mainFont);
		aboutText.setCharacterSize(18);
		aboutText.setString(aboutString);
		aboutText.setOrigin(aboutText.getLocalBounds().width / 2.0, aboutText.getLocalBounds().height / 2.0);
		aboutText.setPosition(512 / 2, 300);
		
		exitText.setFont(mainFont);
		exitText.setCharacterSize(18);
		exitText.setString(exitString);
		exitText.setOrigin(exitText.getLocalBounds().width / 2.0, exitText.getLocalBounds().height / 2.0);
		exitText.setPosition(512 / 2, 350);
	}

	vector<string> getROMsFileNames(const std::string& name)
	{
		vector<string> files;
		std::string pattern(name);
		pattern.append("\\*");
		WIN32_FIND_DATA data;
		HANDLE hFind;
		if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != ((HANDLE)(LONG_PTR)-1)) {
			do {
				if (regex_match(data.cFileName, regex(".*\\.nes$")))
				{
					files.push_back(data.cFileName);
				}
			} while (FindNextFileA(hFind, &data) != 0);
			FindClose(hFind);
		}
		return files;
	}

	enum State
	{
		MainMenu,
		FileMenu,
		Options,
		Emulator,
		PauseMenu
	};

	void switchFileMenu()
	{
		ROMsFileNames.clear();
		ROMsFileNames = getROMsFileNames("roms");
		for (int i = 0; i < FILE_MENU_POSITIONS & i < ROMsFileNames.size(); i++)
		{
			fileNameText[i].setString(ROMsFileNames.at(i));
		}
		fileMenuArrowPosition = 0;
		fileMenuPosition = 0;
		arrowSprite.setPosition(50, 40 + 40 * fileMenuArrowPosition);
		statusText.setString(defaultStatusString);
	}	
	void switchMainMenu()
	{
		arrowSprite.setPosition(100, loadROMText.getGlobalBounds().top);
	}

	bool checkDownArrowKeyState()
	{
		static bool keyState = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			if (!keyState)
			{
				keyState = true;
				return true;
			}
			return false;
		}
		keyState = false;
		return false;
	}
	bool checkUpArrowKeyState()
	{
		static bool keyState = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			if (!keyState)
			{
				keyState = true;
				return true;
			}
			return false;
		}
		keyState = false;
		return false;
	}
	bool checkEnterKeyState()
	{
		static bool keyState = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
		{
			if (!keyState)
			{
				keyState = true;
				return true;
			}
			return false;
		}
		keyState = false;
		return false;
	}
	bool checkEscapeKeyState()
	{
		static bool keyState = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		{
			if (!keyState)
			{
				keyState = true;
				return true;
			}
			return false;
		}
		keyState = false;
		return false;
	}

	void start()
	{
		loadResources();
		initResources();
		sf::RenderWindow window(sf::VideoMode(512, 480), "ReEmu");
		sf::View view = window.getDefaultView();
		CPU::init();
		sf::Event event;
		sf::Clock clock;
		sf::Time elapsed;
		clock.restart();
		elapsed = sf::Time::Zero;
		bool renderState = false;
		unsigned int menuPosition = 0;
		switchFileMenu();
		switchMainMenu();
		State currentState = State::MainMenu;
		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
				if (event.type == sf::Event::Resized)
				{
					window.setSize(sf::Vector2u(event.size.width > 512 ? event.size.width : 512,
						event.size.height > 480 ? event.size.height : 480));
					view.setSize(event.size.width, event.size.height);
					window.setView(view);
					if (event.size.width > 1280 || event.size.height > 720)
					{
						if (event.size.width / 1280.0 > event.size.height / 720.0)
						{
							mainMenuBackgroundSprite.setScale(event.size.width / 1280.0, event.size.width / 1280.0);
						}
						else
						{
							mainMenuBackgroundSprite.setScale(event.size.height / 720.0, event.size.height / 720.0);
						}
					}
					else
					{
						mainMenuBackgroundSprite.setScale(1.0, 1.0);
					}
					if (event.size.width > 512 || event.size.height > 480)
					{
						if (event.size.width / 512.0 < event.size.height / 480.0)
						{
							PPU::getRenderSprite()->setScale(event.size.width / 256.0, event.size.width / 256.0);
						}
						else
						{
							PPU::getRenderSprite()->setScale(event.size.height / 240.0, event.size.height / 240.0);
						}
					}
					else
					{
						PPU::getRenderSprite()->setScale(2.0, 2.0);
					}
				}
			}
			switch (currentState)
			{
			case MainMenu:
			{
				if (checkDownArrowKeyState())
				{
					if (mainMenuArrowPosition < MAIN_MENU_POSITIONS - 1)
					{
						mainMenuArrowPosition++;
						switch (mainMenuArrowPosition)
						{
						case 0:
						{
							arrowSprite.setPosition(100, loadROMText.getGlobalBounds().top);
							break;
						}
						case 1:
						{
							arrowSprite.setPosition(100, optionsText.getGlobalBounds().top);
							break;
						}
						case 2:
						{
							arrowSprite.setPosition(100, aboutText.getGlobalBounds().top);
							break;
						}
						case 3:
						{
							arrowSprite.setPosition(100, exitText.getGlobalBounds().top);
							break;
						}
						}
					}
				}
				if (checkUpArrowKeyState())
				{
					if (mainMenuArrowPosition > 0)
					{
						mainMenuArrowPosition--;
						switch (mainMenuArrowPosition)
						{
						case 0:
						{
							arrowSprite.setPosition(100, loadROMText.getGlobalBounds().top);
							break;
						}
						case 1:
						{
							arrowSprite.setPosition(100, optionsText.getGlobalBounds().top);
							break;
						}
						case 2:
						{
							arrowSprite.setPosition(100, aboutText.getGlobalBounds().top);
							break;
						}
						case 3:
						{
							arrowSprite.setPosition(100, exitText.getGlobalBounds().top);
							break;
						}
						}
					}
				}
				if (checkEnterKeyState())
				{
					switch (mainMenuArrowPosition)
					{
					case 0:
					{
						switchFileMenu();
						currentState = FileMenu;
						break;
					}
					case 1:
					{
						break;
					}
					case 2:
					{
						break;
					}
					case 3:
					{
						window.close();
						break;
					}
					}
				}
				
				break;
			}
			case FileMenu:
			{
				if (checkDownArrowKeyState())
				{
					if (fileMenuPosition + fileMenuArrowPosition < ROMsFileNames.size() - 1)
					{
						if (fileMenuArrowPosition < FILE_MENU_POSITIONS - 1)
						{
							fileMenuArrowPosition++;
							arrowSprite.setPosition(50, 40 + 40 * fileMenuArrowPosition);
						}
						else
						{
							fileMenuPosition++;
							for (int i = 0; i < FILE_MENU_POSITIONS; i++)
							{
								fileNameText[i].setString(ROMsFileNames.at(fileMenuPosition + i));
							}
						}
					}
				}
				if (checkUpArrowKeyState())
				{
					if (fileMenuPosition + fileMenuArrowPosition > 0)
					{
						if (fileMenuArrowPosition > 0)
						{
							fileMenuArrowPosition--;
							arrowSprite.setPosition(50, 40 + 40 * fileMenuArrowPosition);
						}
						else
						{
							fileMenuPosition--;
							for (int i = 0; i < FILE_MENU_POSITIONS; i++)
							{
								fileNameText[i].setString(ROMsFileNames.at(fileMenuPosition + i));
							}
						}
					}
				}
				if (checkEnterKeyState())
				{
					string filePath = "roms/";
					filePath.append(ROMsFileNames.at(fileMenuPosition + fileMenuArrowPosition));
					switch (GamePak::loadFromFile(filePath))
					{
					case 1:
					{
						statusText.setString(fileErrorOpen);
						break;
					}
					case 2:
					{
						statusText.setString(fileErrorRead);
						break;
					}
					case 3:
					{
						statusText.setString(fileErrorNotSupported);
						break;
					}
					case 4:
					{
						statusText.setString(fileErrorInvalid);
						break;
					}
					case 0:
					{
						CPU::init();
						currentState = State::Emulator;
					}
					}
				}
				if (checkEscapeKeyState())
				{
					switchMainMenu();
					currentState = MainMenu;
				}
				break;
			}
			case Options:
			{
				break;
			}
			case Emulator:
			{
				if (checkEscapeKeyState())
				{
					switchFileMenu();
					currentState = FileMenu;
				}
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
			if (elapsed.asMilliseconds() >= 1000.0 / MAX_FPS)
			{
				elapsed = sf::Time::Zero;
				window.clear();
				switch (currentState)
				{
				case MainMenu:
				{
					window.draw(mainMenuBackgroundSprite);
					window.draw(logoText);
					window.draw(arrowSprite);
					window.draw(loadROMText);
					window.draw(optionsText);
					window.draw(aboutText);
					window.draw(exitText);
					break;
				}
				case FileMenu:
				{
					window.draw(mainMenuBackgroundSprite);
					window.draw(arrowSprite);
					for (int i = 0; i < FILE_MENU_POSITIONS; i++)
					{
						if (fileNameText[i].getString().getSize() > MAX_FILENAME_LENGTH)
						{
							string buff = fileNameText[i].getString().substring(0, MAX_FILENAME_LENGTH - 3);
							buff.append("...");
							fileNameText[i].setString(buff);
						}
						window.draw(fileNameText[i]);
					}
					window.draw(statusText);
					break;
				}
				case Emulator:
				{
					window.draw(mainMenuBackgroundSprite);
					window.draw(*PPU::getRenderSprite());
					break;
				}
				}				
				window.display();
				renderState = true;
			}
		}
		window.close();
	}
}