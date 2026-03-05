#pragma once

#include "console_renderer.h"
#include <chrono>
#include <array>
#include <vector>
#include <deque>
#include <cmath>

using namespace std::chrono;

typedef unsigned int TileIndex;
typedef unsigned short ID;

namespace Game
{
	struct Vector2Int
	{
		int x, y;
		Vector2Int operator+(const Vector2Int& other) const { return { x + other.x, y + other.y }; }
	};

	struct GameTime
	{
		duration<float> mTimeStep;
		steady_clock::time_point mTime{};
		float mDeltaTime = 0;
	};

	enum RunID : ID
	{
		Exit = 0,
		Running = 1,
		Restart = 2
	};

	enum TileID : ID
	{
		Wall = 0,
		Fruit = 1,
		Snake = 2,
		Snake1 = 3,
		Snake2 = 4,
		Snake4 = 5
	};

	struct Tile
	{
		char mSymbol;
		unsigned long mColor;
		ID mId = 0;

		const bool operator==(const Tile& pOther) const
		{
			return mId == pOther.mId &&
				mSymbol == pOther.mSymbol &&
				mColor == pOther.mColor;
		}
	};

	struct TileMap
	{
		std::vector<Tile*> mTiles;
		unsigned int mWidth, mHeight, mSize;

		Vector2Int operator()(Vector2Int pVector) const
		{
			int x = pVector.x % static_cast<int>(mWidth);
			x += mWidth * (x < 0);

			int y = pVector.y % static_cast<int>(mHeight);
			y += mHeight * (y < 0);

			return { x, y };
		}

		TileIndex operator()(int pX, int pY) const
		{
			pX %= static_cast<int>(mWidth);
			if (pX < 0) pX += mWidth;

			pY %= static_cast<int>(mHeight);
			if (pY < 0) pY += mHeight;

			return pY * mWidth + pX;
		}

		TileIndex operator()(int pIndex) const
		{
			pIndex %= static_cast<int>(mSize);
			if (pIndex < 0) pIndex += mSize;

			return pIndex;
		}

		Tile*& operator[](const TileIndex pIndex)
		{ return mTiles[pIndex]; }

		Tile*& operator[](const Vector2Int pVector)
		{ return operator[](pVector.x + pVector.y * mWidth); }

		TileMap(const unsigned int pWidth = 20, const unsigned int pHeight = 20) :
			mWidth(pWidth), mHeight(pHeight), mSize(pWidth * pHeight), mTiles(pWidth * pHeight, nullptr) { }
	};

	struct PlayerSnake
	{
		bool isDead = false;
		Vector2Int mDirection{ 0, 1 };
		std::deque<Vector2Int> mPositions{};
	};

	struct GameState
	{
		PlayerSnake mPlayer;
		RunID pRunId = RunID::Running;
		float fruitSpawnTimer = 0;

		TileMap mTileMap;
		std::array<Tile, 6> mTileTypes =
		{
			Tile{ '#', FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '*', FOREGROUND_GREEN },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
		};
	};

	RunID RunGame();

	namespace Core
	{
		void OnInit(GameState& pGame);
		void OnTick(GameState& pGame, const GameTime& pTime);
		void OnFixedTick(GameState& pGame, const GameTime& pTime);
		void OnDraw(GameState& pGame, const GameTime& pTime);
		RunID OnExit(GameState& pGame, const GameTime& pTime);
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const unsigned int pX, const unsigned int pY);
		void DrawTileMap(TileMap& pTileMap);
	}

	namespace Mechanics
	{
		void InitTiles(GameState& pGame);
		void GenTileMap(GameState& pGame);
		void SpawnTileRandom(TileMap& pTileMap, Tile* pTile);
	}

	namespace Player
	{
		void InitPlayer(GameState& pGame, PlayerSnake& pPlayer);
		void AddToPlayer(GameState& pGame, PlayerSnake& pPlayer, unsigned short amount = 1);
		void TickPlayer(GameState& pGame, const GameTime& pTime, PlayerSnake& pPlayer);
		void TickInput(GameState& pGame, const GameTime& pTime, PlayerSnake& pPlayer);
		void OnCollided(GameState& pGame, PlayerSnake& pPlayer, Tile* pTile);
	}
}

