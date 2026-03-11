#include "Game_Refactored.h"
#include "Utils.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>
#include <sstream>

using namespace std::chrono;

namespace Game_Refactored
{
	RunID RunGame()
	{
		GameInstance game(std::filesystem::current_path());
		game.OnInit();

		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 8) };
		GameTime drawTime{ duration<float>(1.0f / 24) };
		Rendering::RendBuffer renderBuffer(3600);

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

		LoadLevel();
	}

	void GameInstance::OnTick(const GameTime& pTime)
	{
		if (Input::GetKey(VK_ESCAPE)) mRunId = EXIT;
		if (Input::GetKey(VK_F1)) LoadLevel();
		if (Input::GetKey('R')) mRunId = RESTART;

		if (mState == GameState::STARTING)
		{
			Vector2Int input = { 0,0 };
			for (auto& player : mPlayers)
			{
				if (!IsPlayerActive(player) && player.mInput.IsInputting())
					InitNewSnake(player);
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
			snake.OnTickCollisions(pTime);

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
					pBuffer.DrawCharCoords(tile->mSymbol, x, y, tile->mColor);
			}

		pBuffer.RenderBuffers();
	}

	RunID GameInstance::OnExit()
	{
		return mRunId;
	}

	void GameInstance::LoadLevel()
	{
		std::fstream file(std::filesystem::current_path().string() +
			"\\LevelData.txt", std::ios::in | std::ios::out);

		std::string line;
		std::string content;

		UINT32 height = 0, width = 0;
		while (std::getline(file, line))
		{
			if (line.empty()) break;

			if (height == 0) width = line.size();

			height++;
			content.append(line);
		}

		mTileMap = TileMap(width, height);

		for (size_t i = 0; i < content.size(); i++)
			if (content[i] != ' ')
				mTileMap[i] = &mWallTile;
	}

	void GameInstance::InitNewSnake(Player& pPlayer)
	{
		Vector2Int pos = { 1,1 };

		mActiveSnakes.push_back(PlayerSnake(pPlayer, *this, pos));
		mTileMap.SetTile(&pPlayer.mTile, pos);
		mLivingSnakes++;
	}

	bool GameInstance::IsPlayerActive(Player& pPlayer)
	{
		for (auto& snake : mActiveSnakes)
			if (snake.mPlayer.mId == pPlayer.mId)
				return true;
		return false;
	}

	void PlayerSnake::OnTick(const GameTime& pTime)
	{
		mPlayer.mInput.GetAxis(mDirection);
	}

	void PlayerSnake::OnFixedTick(const GameTime& pTime)
	{
		if (mPositions.empty()) return;

		if (!mIsAlive)
		{
			mGame.GetTileMap().SetTile(nullptr, Head());
			mPositions.pop_front();

			return;
		}

		if (mGame.mState != PLAYING) return;

		Vector2Int oldTail = Tail();
		mGame.GetTileMap().SetTile(nullptr, oldTail);
		mPositions.pop_back();

		Vector2Int newHead = mGame.GetTileMap()(mPositions.front() + mDirection);
		mGame.GetTileMap().SetTile(&mPlayer.mTile, newHead);
		mPositions.push_front(newHead);
	}

	void PlayerSnake::OnTickCollisions(const GameTime& pTime)
	{
		if (mGame.mState != PLAYING)
			return;

		Tile* collidedTile = mGame.mTileMap[mGame.mTileMap(
			Head() + mDirection)];

		if (collidedTile)
			collidedTile->mColEvent(mGame, *this, collidedTile);
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

	const bool PlayerInput::IsInputting()
	{
		return Input::GetKey(mRight) + Input::GetKey(mLeft) +
			Input::GetKey(mDown) + Input::GetKey(mUp);
	}
}