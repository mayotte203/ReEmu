#pragma once
#include "common.h"
#include "CPU.h"
#include "PPU.h"
#include <SFML/Main.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <cstdint>
#include <regex>
#include <vector>
#ifdef __linux__ 
#include <dirent.h>
#elif _WIN32
#include <Windows.h>
#endif

namespace GUI
{
	void start();
}