#pragma once

#include "Game.h"
#include <iostream>
#include <chrono>

using namespace std::chrono;

void main()
{
	while (Game::RunGame() == Game::RunID::Restart) {}

	// CONTROLS: F1 -restart ESC QUIT Player 1: WASD Player 2: IJKL Player 3: ARROWS
	// PRESS SPACE TO START
	// 
}