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
		GameInstance game;
		game.OnInit();

		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 8) };
		GameTime drawTime{ duration<float>(1.0f / 24) };
		Rendering::RendBuffer renderBuffer(4500);

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

	inline void GameInstance::KillSnake(PlayerSnake& pSnake)
	{
		if (!pSnake.IsAlive())
			return;

		pSnake.OnKilled();
		mLivingSnakes--;
	}

	inline void GameInstance::RemoveSnake(PlayerSnake& pSnake)
	{
		auto it = std::find_if(mActiveSnakes.begin(), mActiveSnakes.end(),
			[&pSnake](const PlayerSnake& s) { return &s == &pSnake; });

		if (it != mActiveSnakes.end())
			mActiveSnakes.erase(it);
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

		if (mState == STARTING)
		{
			Vector2Int input = { 0,0 };
			for (auto& player : mPlayers)
			{
				if (!IsPlayerActive(player) && player.mInput.IsInputting())
					InitNewSnake(player);
			}

			if (Input::GetKey(VK_SPACE) && !mActiveSnakes.empty())
				mState = PLAYING;
		}

		for (auto& snake : mActiveSnakes)
			snake.OnTick(pTime, *this);
	}

	void GameInstance::OnFixedTick(const GameTime& pTime)
	{
		for (auto& snake : mActiveSnakes)
			snake.OnTickCollisions(pTime, *this);

		for (auto& snake : mActiveSnakes)
			snake.OnFixedTick(pTime, *this);
	}

	void GameInstance::OnDraw(const GameTime& pTime, Rendering::RendBuffer& pBuffer)
	{
		pBuffer.ClearBuffers();

		int centerX = pBuffer.mWidth / 2 - mTileMap.mWidth / 2,
			centerY = pBuffer.mHeight / 2 - mTileMap.mHeight / 2;

		Tile* tile = nullptr;
		for (int x = 0; x < mTileMap.mWidth; x++)
			for (int y = 0; y < mTileMap.mHeight; y++)
			{
				tile = mTileMap[{ x, y }];

				if (tile) // Draw tile map.
					pBuffer.DrawCharCoords(tile->mSymbol, x + centerX, y + centerY, tile->mColor);
				else if (x > 0 && x < mTileMap.mWidth - 1 && y > 0 && y < mTileMap.mHeight - 1) // Draw floor.
					pBuffer.DrawCharCoords('.', x + centerX, y + centerY, white);
				else // Draw borders.
					pBuffer.DrawCharCoords('+', x + centerX, y + centerY, FOREGROUND_RED | FOREGROUND_BLUE);
			}

		pBuffer.RenderBuffers();
	}

	RunID GameInstance::OnExit()
	{
		return mRunId;
	}

	void GameInstance::LoadLevel()
	{
		std::filesystem::path filePath = std::filesystem::current_path() / "LevelData.txt";
		std::ifstream file(filePath);

		if (!file.is_open())
			throw std::runtime_error("Failed to open LevelData.txt");

		std::string line;
		uint32_t width = 0, height = 0;

		// First pass: find width and height
		while (std::getline(file, line))
		{
			if (line.size() > width) width = line.size();
			height++;
		}

		file.clear();
		file.seekg(0);

		mTileMap = TileMap(width, height);

		// Second pass: set tiles
		int y = 0;
		while (std::getline(file, line))
		{
			for (int x = 0; x < line.size(); x++)
				if (line[x] != ' ')
					mTileMap.SetTile(&mWallTile, {x, y});
			y++;
		}
	}

	void GameInstance::InitNewSnake(Player& pPlayer)
	{
		Vector2Int pos = { 1,1 };

		mLivingSnakes++;
		mActiveSnakes.push_back(PlayerSnake(pPlayer, pos));
		mTileMap.SetTile(&pPlayer.mTile, pos);
	}

	bool GameInstance::IsPlayerActive(Player& pPlayer)
	{
		for (auto& snake : mActiveSnakes)
			if (snake.mPlayer == &pPlayer)
				return true;
		return false;
	}

	void PlayerSnake::OnTick(const GameTime& pTime, GameInstance& pGame)
	{
		if (!mIsAlive) return;

		mPlayer->mInput.GetAxis(mDirection);
	}

	void PlayerSnake::OnFixedTick(const GameTime& pTime, GameInstance& pGame)
	{
		if (mPositions.empty())
		{
			pGame.RemoveSnake(*this);
			return;
		}

		if (!mIsAlive)
		{
			pGame.GetTileMap().SetTile(nullptr, Head());
			mPositions.pop_front();

			return;
		}

		if (pGame.mState != PLAYING) return;

		pGame.GetTileMap().SetTile(nullptr, Tail());
		mPositions.pop_back();

		Vector2Int newHead = pGame.GetTileMap()(Head() + mDirection);
		pGame.GetTileMap().SetTile(&mPlayer->mTile, newHead);
		mPositions.push_front(newHead);
	}

	void PlayerSnake::OnTickCollisions(const GameTime& pTime, GameInstance& pGame)
	{
		if (pGame.mState != PLAYING || !mIsAlive)
			return;

		Tile* collidedTile = pGame.mTileMap[pGame.mTileMap(
			Head() + mDirection)];

		if (collidedTile)
			collidedTile->mColEvent(pGame, *this, collidedTile);
	}

	void PlayerSnake::OnKilled()
	{
		mIsAlive = false;
	}

	const void PlayerInput::GetAxis(Vector2Int& pAxis) const
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

	const bool PlayerInput::IsInputting() const
	{
		return Input::GetKey(mRight) + Input::GetKey(mLeft) +
			Input::GetKey(mDown) + Input::GetKey(mUp);
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
			pGame.KillSnake(pSnake);
			return true;
		}
	}
}