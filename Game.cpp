#include "Game.h"
#include <iostream>
#include <stdexcept>

#define WRITELINE(x) std::cout << x << "\n" 
#define WRITE(x) std::cout << x;

using namespace std::chrono;

namespace Game
{
	void Run()
	{
		using namespace Core;

		GameState game{};
		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 30) };

		OnInit(game);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> accumulator(0.0f);

		while (game.mIsRunning) // Game loop.
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			time.mDeltaTime = time.mTimeStep.count();

			previousTime = time.mTime;
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

		OnExit(game, time);
	}

	namespace Core
	{
		using namespace Game::Draw;
		using namespace Game::Mechanics;

		void OnInit(GameState& pGame)
		{
			setupCustomConsole();
			InitTiles(pGame);
			pGame.mTileMap = GenTileMap(pGame);
		}

		void OnTick(GameState& pGame, GameTime& pTime)
		{
			// input logic.

			if (getIfRightKeyIsCurrentlyDown())
				pGame.mPlayer1.mDirX = 1;
			else if (getIfLeftKeyIsCurrentlyDown())
				pGame.mPlayer1.mDirX = -1;
			else
				pGame.mPlayer1.mDirX = 0;

			if (getIfUpKeyIsCurrentlyDown())
				pGame.mPlayer1.mDirY = -1;
			else if (getIfDownKeyIsCurrentlyDown())
				pGame.mPlayer1.mDirY = 1;
			else
				pGame.mPlayer1.mDirY = 0;
		}

		void OnFixedTick(GameState& pGame, GameTime& pTime)
		{
			pGame.mPlayer1.mPosX += pGame.mPlayer1.mDirX *
				pTime.mDeltaTime * pGame.mPlayer1.mSpeed;

			pGame.mPlayer1.mPosY += pGame.mPlayer1.mDirY *
				pTime.mDeltaTime * pGame.mPlayer1.mSpeed;
		}

		void OnDraw(GameState& pGame, GameTime& pTime)
		{
			clearBuffer();

			DrawTileMap(pGame.mTileMap);
			//DrawTile(pGame.mTileTypes[TileID::Wall], { 5, 8 });

			renderBuffers();
		}

		void OnExit(GameState& pGame, GameTime& pTime) { deleteCustomConsole(); }
	}

	namespace Draw
	{
		void DrawTile(Tile& pTile, Coord pCoords) { drawTile(pCoords.x, pCoords.y, pTile.mSymbol, pTile.mColor); }

		void DrawTile(Tile* pTile, Coord pCoords)
		{
			if (pTile)
				drawTile(pCoords.x, pCoords.y, pTile->mSymbol, pTile->mColor);
		}

		void DrawTileMap(TileMap& pTileMap)
		{
			for (size_t y = 0; y < pTileMap.mWidth; y++)
				for (size_t x = 0; x < pTileMap.mHeight; x++)
					DrawTile(pTileMap(x, y), { x, y });
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

		TileMap GenTileMap(GameState& pGame)
		{
			TileMap map = TileMap(20, 20);
			Tile* tile = &(pGame.mTileTypes[TileID::Wall]);

			for (size_t i = 0; i < map.mWidth; i++)
			{
				map(i, 0) = tile;
				map(i, map.mHeight - 1) = tile;
			}

			for (size_t i = 0; i < map.mHeight; i++)
			{
				map(0, i) = tile;
				map(map.mWidth - 1, i) = tile;
			}

			return map;
		}

		Tile& GetTileType(GameState& pGame, unsigned short pId) { return pGame.mTileTypes[pId]; }
	}
}