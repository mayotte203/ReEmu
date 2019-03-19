#pragma once
#include "common.h"
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

namespace CPU
{
	void write(u16 addr, u8 data);
	u8 read(u16 addr);
	void init();
	void op();
	void handleNMI();
}