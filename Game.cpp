#include "Game.h"
#include "Utils.h"
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

		GameState game{};
		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 8) };

		OnInit(game);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> accumulator(0.0f);

		while (game.pRunId == RunID::Running) // Game loop.
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			previousTime = time.mTime;
			time.mDeltaTime = time.mTimeStep.count();

			accumulator += time.mTimeStep;

			if (time.mTimeStep.count() > 0.25) // Guard against spiral of death.
				time.mTimeStep = duration<float>(0.25);

			OnTick(game, time);

			while (accumulator >= fixedTime.mTimeStep) // Latency compensation.
			{
				fixedTime.mTime = steady_clock::now();
				fixedTime.mDeltaTime = fixedTime.mTimeStep.count();

				OnFixedTick(game, fixedTime);
				OnDraw(game, fixedTime);

				accumulator -= fixedTime.mTimeStep;
			}
		}

		return OnExit(game, time);
	}

	namespace Core
	{
		using namespace Game::Draw;
		using namespace Game::Mechanics;
		using namespace Game::Player;

		void OnInit(GameState& pGame)
		{
			setupCustomConsole();
			InitTiles(pGame);
			InitPlayer(pGame, pGame.mPlayer);
			GenTileMap(pGame);
		}

		void OnTick(GameState& pGame, const GameTime& pTime)
		{
			TickInput(pGame, pTime, pGame.mPlayer);
		}

		void OnFixedTick(GameState& pGame, const GameTime& pTime)
		{
			TickPlayer(pGame, pTime, pGame.mPlayer);
		}

		void OnDraw(GameState& pGame, const GameTime& pTime)
		{
			clearBuffer();

			DrawTileMap(pGame.mTileMap);

			renderBuffers();
		}

		RunID OnExit(GameState& pGame, const GameTime& pTime)
		{ 
			deleteCustomConsole();
			return pGame.pRunId;
		}
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const unsigned int pX, const unsigned int pY) { if (pTile) drawCharCoord(pTile->mSymbol, pX, pY, pTile->mColor); }
		void DrawTileMap(TileMap& pTileMap)
		{
			for (size_t x = 0; x < pTileMap.mWidth; x++)
				for (size_t y = 0; y < pTileMap.mHeight; y++)
					DrawTile(pTileMap[pTileMap(x, y)], x, y);
		}
	}

	namespace Mechanics
	{
		void InitTiles(GameState& pGame)
		{
			// initialize tile types.
			for (size_t i = 0; i < pGame.mTileTypes.size(); i++)
			{
				Tile& tileType = pGame.mTileTypes[i];
				tileType.mId = i;
			}
		}

		void GenTileMap(GameState& pGame)
		{
			TileMap map = TileMap(20, 10);
			Tile* wall = &pGame.mTileTypes[TileID::Wall];

			for (size_t i = 0; i < map.mWidth; i++)
			{
				map[map(i, 0)] = wall;
				map[map(i, map.mHeight - 1)] = wall;
			}

			for (size_t i = 0; i < map.mHeight; i++)
			{
				map[map(0, i)] = wall;
				map[map(map.mWidth - 1, i)] = wall;
			}

			pGame.mTileMap = map;
			SpawnTileRandom(pGame.mTileMap, &pGame.mTileTypes[TileID::Fruit]);
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

		void InitPlayer(GameState& pGame, PlayerSnake& pPlayer)
		{
			pPlayer.mPositions.push_front({ 5,5 });
			AddToPlayer(pGame, pPlayer, 1);
		}

		void AddToPlayer(GameState& pGame, PlayerSnake& pPlayer, unsigned short amount)
		{
			while (amount-- > 0)
				pPlayer.mPositions.push_back(pPlayer.mPositions.back());
		}

		void TickPlayer(GameState& pGame, const GameTime& pTime, PlayerSnake& pPlayer)
		{
			if (pPlayer.mPositions.empty()) return;

			Tile* collidedTile = pGame.mTileMap[pGame.mTileMap(pPlayer.mPositions.front() + pPlayer.mDirection)];

			if (collidedTile)
			{
				if (collidedTile->mId == TileID::Fruit)
				{
					AddToPlayer(pGame, pPlayer);
					SpawnTileRandom(pGame.mTileMap, &pGame.mTileTypes[TileID::Fruit]);
				}
				else
				{
					pPlayer.isDead = true;
					pGame.pRunId = RunID::Restart;
					return;
				}
			}

			Vector2Int oldTail = pPlayer.mPositions.back();
			pGame.mTileMap[oldTail] = nullptr;
			pPlayer.mPositions.pop_back();

			Vector2Int newHead = pGame.mTileMap(pPlayer.mPositions.front() + pPlayer.mDirection);
			pPlayer.mPositions.push_front(newHead);
			pGame.mTileMap[newHead] = &pGame.mTileTypes[TileID::Snake];
		}

		void TickInput(GameState& pGame, const GameTime& pTime, PlayerSnake& pPlayer)
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

		void OnCollided(GameState& pGame, PlayerSnake& pPlayer, Tile* pTile)
		{

		}
	}
}