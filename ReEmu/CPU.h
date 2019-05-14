#pragma once
#include "common.h"
#include "PPU.h"
#include "GamePak.h"
#include "joystick.h"
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>

namespace CPU
{
	void init();
	void op();
	void handleNMI();
}