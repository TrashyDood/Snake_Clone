#include "Game_Refactored.h"
#include "Utils.h"
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

		Rendering::RendBuffer renderBuffer(3600);

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

				game.OnDraw(drawTime, renderBuffer);

				drawAccumulator -= drawTime.mTimeStep;
			}
		}

		return game.OnExit();
	}

	void GameInstance::OnInit()
	{
		for (size_t i = 0; i < mPlayers.size(); i++)
			mPlayers[i].mId = i;


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
			snake.OnTick(pTime);
	}

	void GameInstance::OnFixedTick(const GameTime& pTime)
	{
		for (auto& snake : mActiveSnakes)
			snake.OnFixedTick(pTime);
	}

	void GameInstance::OnDraw(const GameTime& pTime, Rendering::RendBuffer& pBuffer)
	{
		pBuffer.ClearBuffers();

		Vector2Int pos = { 0,0 };
		Tile* tile = nullptr;
		for (size_t x = 0; x < mTileMap.GetWidth(); x++)
			for (size_t y = 0; y < mTileMap.GetHeight(); y++)
			{
				pos.x = x;
				pos.y = y;
				tile = mTileMap[pos];

				if (tile)
					Rendering::renderBuffer.DrawCharCoords(tile->mSymbol, x, y, tile->mColor);
			}

		pBuffer.RenderBuffers();
	}

	RunID GameInstance::OnExit()
	{
		return mRunId;
	}

	void PlayerSnake::OnTick(const GameTime& pTime)
	{
		mPlayer.mInput.GetAxis(mDirection);
	}

	void PlayerSnake::OnFixedTick(const GameTime& pTime)
	{

	}

	void PlayerSnake::TickCollisions(const GameTime& pTime)
	{

	}

	namespace CollisionEvents
	{
		bool FruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			return true;
		}

		bool SuperFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			return true;
		}

		bool MegaFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			return true;
		}

		bool DeathCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			return true;
		}
	}

	const void PlayerInput::GetAxis(Vector2Int& pAxis)
	{
		int inputX =
			Math::Clamp(Input::GetKey(mRight))
			- Math::Clamp(Input::GetKey(mLeft));
		int inputY =
			Math::Clamp(Input::GetKey(mDown))
			- Math::Clamp(Input::GetKey(mUp));

		if (inputX != 0 && pAxis.x == 0)
		{
			pAxis.y = 0;
			pAxis.x = inputX;
		}

		if (inputY != 0 && pAxis.y == 0)
		{
			pAxis.x = 0;
			pAxis.y = inputY;
		}
	}
}