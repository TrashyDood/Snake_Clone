#pragma once

#include "Game.h"
#include <iostream>
#include <chrono>

using namespace std::chrono;

void main()
{
	while (Game::RunGame() != Game::RunID::Exit) {}
}

