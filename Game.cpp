#include "Game.h"
#include "Utils.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

#define WRITELINE(x) std::cout << x << "\n" 
#define WRITE(x) std::cout << x;

using namespace std::chrono;

namespace Game
{
	RunState RunGame()
	{
		using namespace Core;

		GameState game{};
		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 8) };

		OnInit(game);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> accumulator(0.0f);

		while (game.pRunState == 0) // Game loop.
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

	void ExitGame(GameState& pGame, RunState pExitState)
	{
		pGame.pRunState = pExitState;
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
			InitPlayer(pGame.mPlayer);
			GenTileMap(pGame);
		}

		void OnTick(GameState& pGame, const GameTime& pTime)
		{
			TickInput(pGame, pTime);
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

		RunState OnExit(GameState& pGame, const GameTime& pTime)
		{ 
			deleteCustomConsole();
			return pGame.pRunState;
		}
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const unsigned int pPosition) { if (pTile) drawTile(pPosition, pTile->mSymbol, pTile->mColor); }
		void DrawTile(Tile* pTile, const unsigned int pX, const unsigned int pY) { if (pTile)drawTile(pX, pY, pTile->mSymbol, pTile->mColor); }
		
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
			TileMap map = TileMap(60, 30);
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
		}

		Tile& GetTileType(GameState& pGame, const unsigned short pId) { return pGame.mTileTypes[pId]; }
		Tile* GetTile(GameState& pGame, const Vector2F pCoords) { return pGame.mTileMap(pCoords); }
		void SetTile(GameState& pGame, const Vector2F pCoords, Tile* pTile) { pGame.mTileMap(pCoords) = pTile; }
	}

	namespace Player
	{
		using namespace Game::Mechanics;

		void InitPlayer(PlayerSnake& pPlayer)
		{
			pPlayer.mPositions.push_back({ 5, 5 });
			pPlayer.mPositions.push_back({ 5, 6 });
			pPlayer.mPositions.push_back({ 5, 7 });
		}

		void TickPlayer(GameState& pGame, const GameTime& pTime, PlayerSnake& pPlayer)
		{

		}

		void AddToPlayer(PlayerSnake& pPlayer, unsigned short amount)
		{

		}

		void TickInput(GameState& pGame, const GameTime& pTime)
		{
			int inputX =
				Math::Clamp(getIfRightKeyIsCurrentlyDown())
				- Math::Clamp(getIfLeftKeyIsCurrentlyDown());
			int inputY =
				Math::Clamp(getIfDownKeyIsCurrentlyDown())
				- Math::Clamp(getIfUpKeyIsCurrentlyDown());

			if (inputX != 0)
			{
				pGame.mInput.y = 0;
				pGame.mInput.x = inputX;
			}

			if (inputY != 0)
			{
				pGame.mInput.x = 0;
				pGame.mInput.y = inputY;
			}

			if (getIfSpaceKeyIsCurrentlyDown())
				AddToPlayer(pGame.mPlayer);
		}
	}
}