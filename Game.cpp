#include "Game.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>

#define WRITELINE(x) std::cout << x << "\n" 
#define WRITE(x) std::cout << x;

using namespace std::chrono;

namespace Game
{
	RunID RunGame()
	{
		using namespace Core;

		GameInstance game{};
		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 6) };
		GameTime drawTime{ duration<float>(1.0f / 24) };

		OnInit(game);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> fixedAccumulator(0.0f), drawAccumulator(0.0f);

		while (game.pRunId == RunID::Running) // Game loop.
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			time.mDeltaTime = time.mTimeStep.count();
			previousTime = time.mTime;

			fixedAccumulator += time.mTimeStep;
			drawAccumulator += time.mTimeStep;

			OnTick(game, time); // Normal tick.

			while (fixedAccumulator >= fixedTime.mTimeStep) // Fixed tick.
			{
				fixedTime.mTime = steady_clock::now();
				fixedTime.mDeltaTime = fixedTime.mTimeStep.count();

				OnFixedTick(game, fixedTime);
				//OnDraw(game, fixedTime);

				fixedAccumulator -= fixedTime.mTimeStep;
			}

			while (drawAccumulator >= drawTime.mTimeStep) // Draw tick.
			{
				drawTime.mTime = steady_clock::now();
				drawTime.mDeltaTime = drawTime.mTimeStep.count();

				OnDraw(game, drawTime);

				drawAccumulator -= drawTime.mTimeStep;
			}
		}

		return OnExit(game, time);
	}

	namespace Core
	{
		using namespace Game::Draw;
		using namespace Game::Mechanics;
		using namespace Game::Player;

		void OnInit(GameInstance& pGame)
		{
			setupCustomConsole();
			InitTiles(pGame);
			InitPlayer(pGame, pGame.mPlayer);
			GenTileMap(pGame);
		}

		void OnTick(GameInstance& pGame, const GameTime& pTime)
		{
			TickInput(pGame, pTime, pGame.mPlayer);
		}

		void OnFixedTick(GameInstance& pGame, const GameTime& pTime)
		{
			TickPlayer(pGame, pTime, pGame.mPlayer);
		}

		void OnDraw(GameInstance& pGame, const GameTime& pTime)
		{
			clearBuffer();

			Rendering::DrawOutline('+', pGame.mTileMap.mWidth + 2, pGame.mTileMap.mHeight + 2, 0, 0);
			DrawTileMap(pGame.mTileMap, 1, 1);

			renderBuffers();
		}

		RunID OnExit(GameInstance& pGame, const GameTime& pTime)
		{ 
			deleteCustomConsole();
			return pGame.pRunId;
		}
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const UINT32 pX, const UINT32 pY) { if (pTile) DrawCharCoords(pTile->mSymbol, pX, pY, pTile->mColor); }
		void DrawTile(Tile& pTile, const UINT32 pX, const UINT32 pY) { DrawCharCoords(pTile.mSymbol, pX, pY, pTile.mColor); }

		void DrawTileMap(TileMap& pTileMap, const UINT32 pOffsetX, const UINT32 pOffsetY)
		{
			for (size_t x = 0; x < pTileMap.mWidth; x++)
				for (size_t y = 0; y < pTileMap.mHeight; y++)
					DrawTile(pTileMap[pTileMap(x, y)], x + pOffsetX, y + pOffsetY);
		}
	}

	namespace Mechanics
	{
		void InitTiles(GameInstance& pGame)
		{
			// initialize tile types.
			for (size_t i = 0; i < pGame.mTiles.size(); i++)
			{
				Tile& tileType = pGame.mTiles[i];
				tileType.mId = i;
			}
		}

		void GenTileMap(GameInstance& pGame)
		{
			TileMap map = TileMap(15, 10);
			Tile* wall = &pGame.mTiles[TileID::Wall];

			//for (size_t i = 0; i < map.mWidth; i++)
			//{
			//	map[map(i, 0)] = wall;
			//	map[map(i, map.mHeight - 1)] = wall;
			//}

			//for (size_t i = 0; i < map.mHeight; i++)
			//{
			//	map[map(0, i)] = wall;
			//	map[map(map.mWidth - 1, i)] = wall;
			//}

			pGame.mTileMap = map;
			SpawnTileRandom(pGame.mTileMap, &pGame.mTiles[TileID::Fruit]);
		}

		void SpawnTileRandom(TileMap& pTileMap, Tile* pTile)
		{
			TileIndex targetIndex;

			do
			{
				std::mt19937 rng(std::random_device{}());
				std::uniform_int_distribution<int> dist(0, pTileMap.mSize);
				targetIndex = (TileIndex)dist(rng);
			} while (pTileMap[pTileMap(targetIndex)]);

			pTileMap[pTileMap(targetIndex)] = pTile;
		}
	}

	namespace Player
	{
		using namespace Game::Mechanics;
		using namespace Game;

		void InitPlayer(GameInstance& pGame, PlayerSnake& pPlayer)
		{
			pPlayer.mPositions.push_front({ 5,5 });
			AddToPlayer(pGame, pPlayer, 1);
		}

		void AddToPlayer(GameInstance& pGame, PlayerSnake& pPlayer, unsigned short amount)
		{
			while (amount-- > 0)
				pPlayer.mPositions.push_back(pPlayer.mPositions.back());
		}

		void TickPlayer(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pPlayer)
		{
			if (pPlayer.mPositions.empty()) return;

			Tile* collidedTile = pGame.mTileMap[pGame.mTileMap(
				pPlayer.mPositions.front() + pPlayer.mDirection)];

			if (collidedTile && collidedTile->mColEvent(pGame, pPlayer, collidedTile))
				return;

			Vector2Int oldTail = pPlayer.mPositions.back();
			pGame.mTileMap[oldTail] = nullptr;
			pPlayer.mPositions.pop_back();

			Vector2Int newHead = pGame.mTileMap(pPlayer.mPositions.front() + pPlayer.mDirection);
			pPlayer.mPositions.push_front(newHead);
			pGame.mTileMap[newHead] = &pGame.mTiles[TileID::Snake];
		}

		void TickInput(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pPlayer)
		{
			int inputX =
				Math::Clamp(getIfRightKeyIsCurrentlyDown())
				- Math::Clamp(getIfLeftKeyIsCurrentlyDown());
			int inputY =
				Math::Clamp(getIfDownKeyIsCurrentlyDown())
				- Math::Clamp(getIfUpKeyIsCurrentlyDown());

			if (inputX != 0 && pPlayer.mDirection.x == 0)
			{
				pPlayer.mDirection.y = 0;
				pPlayer.mDirection.x = inputX;
			}

			if (inputY != 0 && pPlayer.mDirection.y == 0)
			{
				pPlayer.mDirection.x = 0;
				pPlayer.mDirection.y = inputY;
			}
		}
	}

	namespace CollisionEvents
	{
		using namespace Mechanics;
		using namespace Player;

		bool FruitCol(GameInstance& pGame, PlayerSnake& pPlayer, Tile* pTile)
		{
			AddToPlayer(pGame, pPlayer);
			SpawnTileRandom(pGame.mTileMap, &pGame.mTiles[TileID::Fruit]);

			return false;
		}

		bool DeathCol(GameInstance& pGame, PlayerSnake& pPlayer, Tile* pTile)
		{
			pPlayer.isDead = true;
			pGame.pRunId = RunID::Restart;

			return true;
		}
	}
}