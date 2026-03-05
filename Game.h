#pragma once

#include "Utils.h"
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
	// Too lazy to make new files...
#pragma region Forward Declarations
	struct GameInstance;
	struct PlayerSnake;
	struct Tile;
	typedef bool (*CollisionEvent)(GameInstance&, PlayerSnake&, Tile*);

	namespace CollisionEvents
	{
		bool FruitCol(GameInstance& pGame, PlayerSnake& pPlayer, Tile* pTile);
		bool DeathCol(GameInstance& pGame, PlayerSnake& pPlayer, Tile* pTile);
	}
#pragma endregion

	enum PlayerID : ID
	{
		Player1 = 0,
		Player2 = 1,
		Player3 = 2,
		Player4 = 3
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
	};

	struct Vector2Int
	{
		int x = 0, y = 0;
		Vector2Int operator+(const Vector2Int& other) const { return { x + other.x, y + other.y }; }
	};

	struct GameTime
	{
		duration<float> mTimeStep;
		steady_clock::time_point mTime{};
		float mDeltaTime = 0;
	};

	struct Tile
	{
		char mSymbol;
		unsigned long mColor;
		CollisionEvent mColEvent = CollisionEvents::DeathCol;
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
		UINT32 mWidth, mHeight, mSize;

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

		TileMap(const UINT32 pWidth = 20, const UINT32 pHeight = 20) :
			mWidth(pWidth), mHeight(pHeight), mSize(pWidth * pHeight), mTiles(pWidth * pHeight, nullptr) { }
	};

	struct PlayerInput
	{
		KeyID mRight, mUp,
			mLeft, mDown,
			mActivate;
	};

	struct PlayerSnake
	{
		PlayerInput* mInput;
		Vector2Int mDirection;
		bool mIsAlive = false;
		std::deque<Vector2Int> mPositions{};
	};

	struct GameInstance
	{
		RunID pRunId = RunID::Running;
		PlayerSnake mPlayer;
		TileMap mTileMap;

		std::array<PlayerInput, 4> mInputs
		{
			PlayerInput{ 'D', 'W', 'A', 'S'},
			PlayerInput{ 'L', 'I', 'J', 'K'},
			PlayerInput{ VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN },
			PlayerInput{ VK_NUMPAD6, VK_NUMPAD8, VK_NUMPAD4, VK_NUMPAD5 },
		};

		std::array<Tile, 3> mTiles
		{
			Tile{ '#', FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '*', FOREGROUND_GREEN, CollisionEvents::FruitCol },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN },
		};
	};

	RunID RunGame();

	namespace Core
	{
		void OnInit(GameInstance& pGame);
		void OnTick(GameInstance& pGame, const GameTime& pTime);
		void OnFixedTick(GameInstance& pGame, const GameTime& pTime);
		void OnDraw(GameInstance& pGame, const GameTime& pTime);
		RunID OnExit(GameInstance& pGame, const GameTime& pTime);
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const UINT32 pX, const UINT32 pY);
		void DrawTile(Tile& pTile, const UINT32 pX, const UINT32 pY);
		void DrawTileMap(TileMap& pTileMap, const UINT32 pOffsetX = 0, const UINT32 pOffsetY = 0);
	}

	namespace Mechanics
	{
		void InitTiles(GameInstance& pGame);
		void InitTileMap(GameInstance& pGame);
		void SpawnTileRandom(TileMap& pTileMap, Tile* pTile);

		void OnLose(GameInstance& pGame);
		void OnWin(GameInstance& pGame, PlayerSnake& pPlayer);
	}

	namespace Player
	{
		void InitNewPlayer(GameInstance& pGame);
		void AddToPlayer(GameInstance& pGame, PlayerSnake& pPlayer, unsigned short amount = 1);
		void TickPlayer(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pPlayer);
		void TickInput(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pPlayer);
	}
}

