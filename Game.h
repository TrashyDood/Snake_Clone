#pragma once

#include <chrono>

using namespace std::chrono;

namespace Game
{
	struct GameTime
	{
		duration<float> mFixedTimeStep, mTimeStep;
		steady_clock::time_point mFixedTime{}, mTime{};
		float mFixedDeltaTime = 0, mDeltaTime = 0;

		GameTime(duration<float> pTargetFixedTimeStep) :
			mFixedTimeStep(pTargetFixedTimeStep), mTimeStep() {}
	};

	struct Tile
	{
		char symbol;
		unsigned short color;
	};

	struct GameState
	{
		struct
		{
			float mSpeed = 7,
				mDirX = 0, mDirY = 0,
				mPosX = 0, mPosY = 0;
		} mPlayer;

		GameTime& mTime;
		bool mIsRunning = true;
		Tile tiles[5] =
		{
			{'X'},
		};

		GameState(GameTime& pTime) : mTime(pTime) {}
	};

	 void Run();
	 void OnInit(GameState& pGame);
	 void OnTick(GameState& pGame);
	 void OnFixedTick(GameState& pGame);
	 void OnDraw(GameState& pGame);
	 void OnExit(GameState& pGame);
}

