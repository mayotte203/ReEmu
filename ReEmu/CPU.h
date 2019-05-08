#pragma once
#include "common.h"
#include "PPU.h"
#include "GamePak.h"
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
namespace CPU
{
	void write(u16 addr, u8 data);
	u8 read(u16 addr);
	void init();
	void op();
	void handleNMI();
}