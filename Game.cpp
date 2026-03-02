#include "Game.h"
#include "console_renderer.h"
#include <iostream>

#define WRITELINE(x) std::cout << x << "\n" 
#define WRITE(x) std::cout << x;

using namespace std::chrono;

namespace Game
{
	void Run()
	{
		GameTime time(duration<float>(1.0f / 30));
		GameState game(time);

		OnInit(game);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> accumulator(0.0f);

		while (game.mIsRunning)
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			time.mDeltaTime = time.mTimeStep.count();

			previousTime = time.mTime;
			accumulator += time.mTimeStep;

			if (time.mTimeStep.count() > 0.25)
				time.mTimeStep = duration<float>(0.25);

			while (accumulator >= time.mFixedTimeStep)
			{
				time.mFixedTime = steady_clock::now();
				time.mFixedDeltaTime = time.mFixedTimeStep.count();

				accumulator -= time.mFixedTimeStep;

				OnFixedTick(game);
				OnDraw(game);
			}

			OnTick(game);
		}

		OnExit(game);
	}

	void OnInit(GameState& pGame)
	{
		// initialize

		setupCustomConsole();
	}

	void OnTick(GameState& pGame)
	{
		// input logic.

		if (getIfRightKeyIsCurrentlyDown())
			pGame.mPlayer.mDirX = 1;
		else if (getIfLeftKeyIsCurrentlyDown())
			pGame.mPlayer.mDirX = -1;
		else
			pGame.mPlayer.mDirX = 0;

		if (getIfUpKeyIsCurrentlyDown())
			pGame.mPlayer.mDirY = -1;
		else if (getIfDownKeyIsCurrentlyDown())
			pGame.mPlayer.mDirY = 1;
		else
			pGame.mPlayer.mDirY = 0;
	}

	void OnFixedTick(GameState& pGame)
	{
		pGame.mPlayer.mPosX += pGame.mPlayer.mDirX *
			pGame.mTime.mFixedDeltaTime * pGame.mPlayer.mSpeed;

		pGame.mPlayer.mPosY += pGame.mPlayer.mDirY *
			pGame.mTime.mFixedDeltaTime * pGame.mPlayer.mSpeed;
	}

	void OnDraw(GameState& pGame)
	{
		clearBuffer();

		// add to buffer.
		drawTile((size_t)pGame.mPlayer.mPosX, (size_t)pGame.mPlayer.mPosY, '0');

		renderBuffer();
	}

	void OnExit(GameState& pGame)
	{
		// cleanup.
		deleteCustomConsole();
	}
}