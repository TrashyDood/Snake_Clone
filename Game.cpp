#include "Game.h"
#include <iostream>

#define WRITELINE(x) std::cout << x << "\n" 
#define WRITE(x) std::cout << x;

using namespace std::chrono;

namespace Game
{
	void Run()
	{
		using namespace Core;

		GameTime time(duration<float>(1.0f / 30));
		GameState game{};

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

				OnFixedTick(game, time);
				OnDraw(game, time);
			}

			OnTick(game, time);
		}

		OnExit(game, time);
	}

	namespace Core
	{
		using namespace Game::Draw;

		void OnInit(GameState& pGame)
		{
			// initialize tile types.
			pGame.mTileLookup.clear();
			for (size_t i = 0; i < pGame.mTileTypes.size(); i++)
			{
				Tile& tileType = pGame.mTileTypes[i];

				std::string symbol(1, tileType.mSymbol);
				pGame.mTileLookup[symbol] = i;
				pGame.mTileLookup[tileType.mName] = i;
			}

			setupCustomConsole();
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
				pTime.mFixedDeltaTime * pGame.mPlayer1.mSpeed;

			pGame.mPlayer1.mPosY += pGame.mPlayer1.mDirY *
				pTime.mFixedDeltaTime * pGame.mPlayer1.mSpeed;
		}

		void OnDraw(GameState& pGame, GameTime& pTime)
		{
			clearBuffer();

			DrawTileMap(pGame.mTileMap, screen_width, screen_height);
			Mechanics::GetTileType(pGame, TileID::Snake);

			renderBuffers({3, 0});
		}

		void OnExit(GameState& pGame, GameTime& pTime) { deleteCustomConsole(); }
	}

	namespace Draw
	{
		void DrawTile(Tile& pTile, size_t x, size_t y) { drawTile(x, y, pTile.mSymbol, pTile.mColor); }

		void DrawTileMap(TileMap& mTileMap, size_t width, size_t height, size_t x, size_t y)
		{
			for (size_t x = 0; x < width; x++)
				for (size_t y = 0; y < height; y++)
					DrawTile(mTileMap[(x + width * y)], x, y);
		}
	}

	namespace Mechanics
	{
		Tile& GetTileType(GameState& pGame, unsigned short pId) { return pGame.mTileTypes[pId]; }
		Tile& GetTileType(GameState& pGame, std::string pName) { return GetTileType(pGame, pGame.mTileLookup[pName]); }
	}
}