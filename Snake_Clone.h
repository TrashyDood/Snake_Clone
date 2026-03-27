#pragma once

#include "Game_Refactored.h"
#include <iostream>
#include <chrono>

#define WRITE(x) std::cout << x
#define WRITELINE(x) WRITE(x) << "\n";

using namespace std::chrono;
using namespace Game_Refactored;

void main(int argc, char* argv[])
{
	if (argc > 1) while (RunGame(argv[1]) == RESTART) {}
	else while (RunGame() == RESTART) {}

	WRITELINE("Game has exited.")
}