#include "Game_Refactored.h"
#include "Utils.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
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

	void GameInstance::OnInit()
	{
		for (size_t i = 0; i < mPlayers.size(); i++)
			mPlayers[i].mId = i;

		if (mLevel)
			delete(mLevel);

		mLevel = new LevelInstance(*this, std::filesystem::current_path() / "GameData\\Levels\\Level1.txt");
	}

	void GameInstance::OnTick(const GameTime& pTime)
	{
		if (Input::GetKey(VK_ESCAPE)) mRunId = EXIT;
		if (Input::GetKey(VK_F1)) OnInit();
		if (Input::GetKey('R')) mRunId = RESTART;

		if (mState == STARTING)
		{
			Vector2Int input = { 0,0 };
			for (auto& player : mPlayers)
			{
				if (!IsPlayerActive(player) && player.mInput.IsInputting())
					mLevel->InitNewSnake(player);
			}

			if (Input::GetKey(VK_SPACE) && !mLevel->mActiveSnakes.empty())
				mState = PLAYING;
		}

		for (auto& snake : mLevel->mActiveSnakes)
			snake.OnTick(pTime, mLevel);
	}

	void GameInstance::OnFixedTick(const GameTime& pTime)
	{
		for (auto& snake : mLevel->mActiveSnakes)
			snake.OnTickCollisions(pTime, mLevel);

		for (auto& snake : mLevel->mActiveSnakes)
			snake.OnFixedTick(pTime, mLevel);
	}

	void GameInstance::OnDraw(const GameTime& pTime, Rendering::RendBuffer& pBuffer)
	{
		pBuffer.ClearBuffers();

		LevelInstance level = *mLevel;
		int mapCenterX = pBuffer.mWidth / 2 - level.mTileMap.mWidth / 2,
			mapCenterY = pBuffer.mHeight / 2 - level.mTileMap.mHeight / 2;

		Tile* tile = nullptr;
		for (int x = 0; x < level.mTileMap.mWidth; x++)
			for (int y = 0; y < level.mTileMap.mHeight; y++)
			{
				tile = level.mTileMap[{ x, y }];

				if (tile) // Draw tile map.
					pBuffer.DrawCharCoords(tile->mSymbol, x + mapCenterX, y + mapCenterY, tile->mColor);
				else if (x > 0 && x < level.mTileMap.mWidth - 1 && y > 0 && y < level.mTileMap.mHeight - 1) // Draw floor.
					pBuffer.DrawCharCoords('.', x + mapCenterX, y + mapCenterY, white);
				else // Draw borders.
					pBuffer.DrawCharCoords('*', x + mapCenterX, y + mapCenterY, FOREGROUND_RED | FOREGROUND_BLUE);
			}

		pBuffer.RenderBuffers();
	}

	RunID GameInstance::OnExit()
	{
		return mRunId;
	}

	inline void LevelInstance::KillSnake(PlayerSnake& pSnake)
	{
		if (!pSnake.IsAlive())
			return;

		pSnake.OnKilled();
		mLivingSnakes--;
	}

	inline void LevelInstance::RemoveSnake(PlayerSnake& pSnake)
	{
		auto it = std::find_if(mActiveSnakes.begin(), mActiveSnakes.end(),
			[&pSnake](const PlayerSnake& s) { return &s == &pSnake; });

		if (it != mActiveSnakes.end())
			mActiveSnakes.erase(it);
	}

	void inline LevelInstance::InitNewSnake(Player& pPlayer)
	{
		static const Vector2Int corners[4] =
		{
			{ 1, 1 }, // Player 0: top-left
			{ mTileMap.mWidth - 2, mTileMap.mHeight - 1 }, // Player 3: bottom-right
			{ mTileMap.mWidth - 2, 1 }, // Player 1: top-right
			{ 1, mTileMap.mHeight - 2 }, // Player 2: bottom-left
		};

		mActiveSnakes.push_back(PlayerSnake(pPlayer, pPlayer.mId, corners[pPlayer.mId]));
		mTileMap.SetTile(&pPlayer.mTile, corners[pPlayer.mId]);
		mLivingSnakes++;
	}

	inline const TileIndex LevelInstance::RandomTileIndex()
	{ return std::uniform_int_distribution<int>(0, mTileMap.Size() - 1)(mRand); }

	inline const TileIndex LevelInstance::RandomEmptyTileIndex() // Brute force random.
	{
		TileIndex result = mTileMap(RandomTileIndex());

		for (size_t i = 0; i < 100 && mTileMap[result]; i++)
			result = mTileMap(RandomTileIndex());

		return result;
	}

	inline void LevelInstance::SpawnRandomFruit(GameInstance& pGame)
	{
		int rng = std::uniform_int_distribution<int>(0, 7)(mRand);

		Tile* fruit = (rng == 0) ? &pGame.mMegaFruitTile // 1/8 chance
			: (rng <= 2) ? &pGame.mSuperFruitTile        // 2/8 chance
			: &pGame.mFruitTile;                          // 5/8 chance

		mTileMap[RandomEmptyTileIndex()] = fruit;
	}

	bool GameInstance::IsPlayerActive(Player& pPlayer)
	{
		for (auto& snake : mActiveSnakes)
			if (snake.mPlayer == &pPlayer)
				return true;
		return false;
	}

	void PlayerSnake::OnTick(const GameTime& pTime, LevelInstance* pLevel)
	{
		if (!mIsAlive) return;

		mPlayer->mInput.GetAxis(mDirection);
	}

	void PlayerSnake::OnFixedTick(const GameTime& pTime, LevelInstance* pLevel)
	{
		if (mPositions.empty())
		{
			pLevel->RemoveSnake(*this);
			return;
		}

		if (!mIsAlive)
		{
			pLevel->mTileMap.SetTile(nullptr, Head());
			mPositions.pop_front();

			return;
		}

		if (pLevel->mGame.mState != PLAYING) return;

		pLevel->mTileMap.SetTile(nullptr, Tail());
		mPositions.pop_back();

		Vector2Int newHead = pLevel->mTileMap(Head() + mDirection);
		pLevel->mTileMap.SetTile(&mPlayer->mTile, newHead);
		mPositions.push_front(newHead);
	}

	void PlayerSnake::OnTickCollisions(const GameTime& pTime, LevelInstance* pLevel)
	{
		if (pLevel->mGame.mState != PLAYING || !mIsAlive)
			return;

		Tile* collidedTile = pLevel->mTileMap[pLevel->mTileMap(
			Head() + mDirection)];

		if (collidedTile)
			collidedTile->mColEvent(pLevel->mGame, *this, collidedTile);
	}

	void PlayerSnake::OnKilled()
	{
		mIsAlive = false;
	}

	void PlayerSnake::Add(LevelInstance* pLevel, int32_t pAmount)
	{
		mPoints += pAmount;

		while (pAmount--)
			mPositions.push_back(Tail());
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
			pSnake.Add(pGame);
			pGame.SpawnRandomFruit();

			return true;
		}

		bool SuperFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			pSnake.Add(pGame, 2);
			pGame.SpawnRandomFruit();

			return true;
		}

		bool MegaFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			pSnake.Add(pGame, 3);
			pGame.SpawnRandomFruit();

			return true;
		}

		bool DeathCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			pGame.KillSnake(pSnake);
			return true;
		}
	}
}