#include "Game_Refactored.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>

using namespace std::chrono;

namespace Game_Refactored
{
	RunID RunGame()
	{
		GameInstance game{};
		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 8) };
		GameTime drawTime{ duration<float>(1.0f / 24) };

		game.OnInit();

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> fixedAccumulator(0.0f), drawAccumulator(0.0f);

		while (game.mRunId == RunID::RUNNING) // Game loop.
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			time.mDeltaTime = time.mTimeStep.count();
			previousTime = time.mTime;

			fixedAccumulator += time.mTimeStep;
			drawAccumulator += time.mTimeStep;

			game.OnTick(time);

			while (fixedAccumulator >= fixedTime.mTimeStep) // Fixed tick.
			{
				fixedTime.mTime = steady_clock::now();
				fixedTime.mDeltaTime = fixedTime.mTimeStep.count();

				game.OnFixedTick(fixedTime);

				fixedAccumulator -= fixedTime.mTimeStep;
			}

			while (drawAccumulator >= drawTime.mTimeStep) // Draw tick.
			{
				drawTime.mTime = steady_clock::now();
				drawTime.mDeltaTime = drawTime.mTimeStep.count();

				game.OnDraw(drawTime);

				drawAccumulator -= drawTime.mTimeStep;
			}
		}

		return game.OnExit();
	}

	void GameInstance::OnInit()
	{

	}

	void GameInstance::OnTick(const GameTime& pTime)
	{
		if (Input::GetKey(VK_ESCAPE)) mRunId = RunID::EXIT;
		if (Input::GetKey(VK_F1)) mRunId = RunID::RESTART;

		if (mState == GameState::STARTING)
		{
			for (auto& player : mPlayers)
			{

			}

			if (Input::GetKey(VK_SPACE) && !mActiveSnakes.empty())
				mState = GameState::PLAYING;
		}

		for (auto& snake : mActiveSnakes)
			snake.OnFixedTick(pTime);
	}

	void GameInstance::OnFixedTick(const GameTime& pTime)
	{

	}

	void GameInstance::OnDraw(const GameTime& pTime)
	{

	}

	RunID GameInstance::OnExit()
	{
		return mRunId;
	}

	void PlayerSnake::OnTick(const GameTime& pTime)
	{

	}

	void PlayerSnake::OnFixedTick(const GameTime& pTime)
	{

	}

	void PlayerSnake::TickInput(const GameTime& pTime)
	{

	}

	void PlayerSnake::TickCollisions(const GameTime& pTime)
	{

	}
}