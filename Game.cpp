#include "Game.h"
#include "Utils.h"
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
			InitPlayer(pGame);
			pGame.mTileMap = GenTileMap(pGame);
		}

		void OnTick(GameState& pGame, GameTime& pTime)
		{
			int inputX =
				Math::Clamp(getIfRightKeyIsCurrentlyDown())
				- Math::Clamp(getIfLeftKeyIsCurrentlyDown());
			int inputY =
				Math::Clamp(getIfUpKeyIsCurrentlyDown())
				- Math::Clamp(getIfDownKeyIsCurrentlyDown());

			if (inputX != 0)
			{
				pGame.mInput.mVertical = 0;
				pGame.mInput.mHorizontal = inputX;
			}
			if (inputY != 0)
			{
				pGame.mInput.mHorizontal = 0;
				pGame.mInput.mVertical = inputY;
			}
		}

		void OnFixedTick(GameState& pGame, GameTime& pTime)
		{
			pGame.mPlayer.mVelocity = { pGame.mInput.mHorizontal * pTime.mDeltaTime * pGame.mPlayer.mSpeed,
				pGame.mInput.mVertical * pTime.mDeltaTime * pGame.mPlayer.mSpeed };

			PlayerPos newPos = pGame.mPlayer.mPositions.back() + pGame.mPlayer.mVelocity;
			pGame.mPlayer.mPositions.push_back(newPos);
			pGame.mPlayer.mPositions.pop_front();
		}

		void OnDraw(GameState& pGame, GameTime& pTime)
		{
			clearBuffer();

			DrawTileMap(pGame.mTileMap);
			
			for (auto& pos : pGame.mPlayer.mPositions)
				DrawTile(GetTileType(pGame, TileID::Snake), pos.x, pos.y);

			renderBuffers();
		}

		void OnExit(GameState& pGame, GameTime& pTime) { deleteCustomConsole(); }
	}

	namespace Draw
	{
		void DrawTile(Tile& pTile, unsigned int pX, unsigned int pY) { drawTile(pX, pY, pTile.mSymbol, pTile.mColor); }

		void DrawTile(Tile* pTile, unsigned int pX, unsigned int pY) { if (pTile) drawTile(pX, pY, pTile->mSymbol, pTile->mColor); }

		void DrawTileMap(TileMap& pTileMap)
		{
			for (int y = 0; y < pTileMap.mWidth; y++)
				for (int x = 0; x < pTileMap.mHeight; x++)
					DrawTile(pTileMap(x, y), x, y);
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

		void InitPlayer(GameState& pGame)
		{
			pGame.mPlayer.mPositions.push_back({ 0, 0 });
			pGame.mPlayer.mPositions.push_back({ 1, 0 });
			pGame.mPlayer.mPositions.push_back({ 2, 0 });
		}

		TileMap GenTileMap(GameState& pGame)
		{
			TileMap map = TileMap(20, 20);
			Tile* wall = &(pGame.mTileTypes[TileID::Wall]);

			for (size_t i = 0; i < map.mWidth; i++)
			{
				map(i, 0) = wall;
				map(i, map.mHeight - 1) = wall;
			}

			for (size_t i = 0; i < map.mHeight; i++)
			{
				map(0, i) = wall;
				map(map.mWidth - 1, i) = wall;
			}

			return map;
		}

		Tile& GetTileType(GameState& pGame, unsigned short pId) { return pGame.mTileTypes[pId]; }
	}
}