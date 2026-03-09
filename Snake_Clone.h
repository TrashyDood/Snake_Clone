#pragma once

#include "Game_Refactored.h"
#include <iostream>
#include <chrono>

using namespace std::chrono;
using namespace Game_Refactored;

void main()
{
	while (RunGame() == RunID::RESTART) {}
}