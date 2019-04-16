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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	// выборка и обработка сообщений
	switch (message)
	{
	case WM_LBUTTONUP:
		//реакция на сообщение
		MessageBox(hWnd, "Вы кликнули!", "событие", 0);
		break;
	case WM_DESTROY:
		//реакция на сообщение
		PostQuitMessage(0);
		break;
		//все необработанные сообщения обработает сама Windows
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		// switch (message)
	}
	return 0;
} // конец функции обработчика сообщений

int main()
{
	WNDCLASS WindowClass;
	WindowClass.style = 0;
	WindowClass.lpfnWndProc = &WndProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.hIcon = NULL;
	WindowClass.hCursor = 0;
	WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = "SFML App";
	RegisterClass(&WindowClass);

	HWND Window = CreateWindow("SFML App", "SFML Win32", WS_SYSMENU | WS_VISIBLE, 0, 0, 600, 300, NULL, NULL, GetModuleHandle(NULL), NULL);

	// Let's create two SFML views
	DWORD Style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
	HWND  View1 = CreateWindow("STATIC", NULL, Style, 0, 0, 512, 240, Window, NULL, GetModuleHandle(NULL), NULL);

	sf::RenderWindow window(View1);
	CPU::init();
	PPU::assignWindow(&window);
	sf::Event event;
	MSG Message;
	Message.message = ~WM_QUIT;
	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			// If a message was waiting in the message queue, process it
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{
			// SFML rendering code goes here


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
				PPU::draw();
				window.display();
				scounter = false;

			}


		}
	}
		DestroyWindow(Window);

		// Don't forget to unregister the window class
		UnregisterClass("SFML App", GetModuleHandle(NULL));
	return 0;
}