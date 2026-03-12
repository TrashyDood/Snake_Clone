#pragma once

#include "Game_Refactored.h"
#include <iostream>
#include <chrono>

#define WRITE(x) std::cout << x
#define WRITELINE(x) WRITE(x) << "\n";

using namespace std::chrono;
using namespace Game_Refactored;

void main()
{
	try
	{
		while (RunGame() == RESTART) {}
	}
	catch (const std::exception& e)
	{
		WRITELINE(e.what());
	}
}