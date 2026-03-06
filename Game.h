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
	struct Player;
	struct PlayerSnake;
	struct Tile;
	typedef bool (*CollisionEvent)(GameInstance&, PlayerSnake&, Tile*);

	namespace CollisionEvents
	{
		bool FruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile);
		bool SuperFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile);
		bool MegaFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile);
		bool DeathCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile);
	}
#pragma endregion

	enum RunID : ID
	{
		Exit = 0,
		Running = 1,
		Restart = 2
	};

	enum GameState : ID
	{
		Playing = 0,
		Starting = 1,
		Paused = 2,
		GameOver = 3
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
		UINT32 mWidth, mHeight, mSize, mCount = 0;

		Vector2Int operator()(Vector2Int pVector) const
		{
			pVector.x %= static_cast<int>(mWidth);
			if (pVector.x < 0) pVector.x += mWidth;

			pVector.y %= static_cast<int>(mHeight);
			if (pVector.y < 0) pVector.y += mHeight;

			return pVector;
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

	struct Player
	{
		PlayerInput mInput;
		Tile mTile;
		ID mId = 0;

		Player(PlayerInput pInput, Tile pTile)
			: mInput(pInput), mTile(pTile) {
		}
	};

	struct PlayerSnake
	{
		Player* mPlayer = nullptr;
		Vector2Int mDirection{};
		bool mIsAlive = true;
		int mPoints = -1;
		std::deque<Vector2Int> mPositions{};
	};

	struct GameInstance
	{
		UINT32 mSeed;
		TileMap mTileMap;
		RunID mRunId = RunID::Running;
		GameState mState = GameState::Starting;
		UINT32 mLivingSnakes;
		bool mPvpEnabled = false;

		std::vector<PlayerSnake> mActiveSnakes{};

		Tile mWallTile = { '#', FOREGROUND_GREEN | FOREGROUND_BLUE },
			mFruitTile = { '*', FOREGROUND_GREEN, CollisionEvents::FruitCol },
			mSuperFruitTile = { '*', FOREGROUND_RED | FOREGROUND_GREEN, CollisionEvents::SuperFruitCol },
			mMegaFruitTile = { '*', FOREGROUND_RED | FOREGROUND_BLUE, CollisionEvents::MegaFruitCol };

		std::array<Player, 4> mPlayers
		{
			Player({ 'D', 'W', 'A', 'S' }, { '0', FOREGROUND_RED | FOREGROUND_INTENSITY }),
			Player({ 'L', 'I', 'J', 'K' }, { '0', FOREGROUND_BLUE | FOREGROUND_INTENSITY }),
			Player({ VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN }, { '0', FOREGROUND_GREEN | FOREGROUND_INTENSITY }),
			Player({ VK_NUMPAD6, VK_NUMPAD8, VK_NUMPAD4, VK_NUMPAD5 }, { '0', FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY })
		};
	};

	RunID RunGame();

	namespace Core
	{
		// Initialize.
		void OnInit(GameInstance& pGame);
		// Handles input.
		void OnTick(GameInstance& pGame, const GameTime& pTime);
		// Handles game logic.
		void OnFixedTick(GameInstance& pGame, const GameTime& pTime);
		// Handles rendering logic.
		void OnDraw(GameInstance& pGame, const GameTime& pTime);
		RunID OnExit(GameInstance& pGame, const GameTime& pTime);
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const UINT32 pX, const UINT32 pY);
		void DrawTile(Tile& pTile, const UINT32 pX, const UINT32 pY);
		void DrawTileMap(TileMap& pTileMap, const UINT32 pOffsetX = 0, const UINT32 pOffsetY = 0);

		void DrawCenteredString(const TileMap& pTileMap, const std::string& pStr, UINT32 pOffsetX = 0, UINT32 pOffsetY = 0, Color pColor = white);

		void DrawSideInfo(GameInstance& pGame, const GameTime& pTime);
	}

	namespace Mechanics
	{
		void InitTileMap(GameInstance& pGame);
		TileIndex GetRandomTilePos(GameInstance& pGame);
		void PlaceTileRandom(GameInstance& pGame, Tile* pTile);
		void SpawnFruit(GameInstance& pGame, unsigned short pCount = 1);

		void PlaceTile(TileMap& pTileMap, Tile* pTile, TileIndex pIndex);
		void PlaceTile(TileMap& pTileMap, Tile* pTile, Vector2Int pPosition);

		Vector2Int GetCornerPos(const TileMap& pTileMap, UINT32 pCorner);
		Vector2Int GetInwardDirection(const TileMap& pTileMap, Vector2Int pPos);

		bool PlayerIsActive(GameInstance& pGame, const Player* pPlayer);

		UINT32 GetWinningPlayerID(GameInstance& pGame);
	}

	namespace PlayerMechanics
	{
		void InitNewSnake(GameInstance& pGame, ID pPlayerId);
		void AddToSnake(GameInstance& pGame, PlayerSnake& pSnake, unsigned short pCount = 1);
		void TickPlayer(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pSnake);
		void TickInput(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pSnake);
		void KillSnake(GameInstance& pGame, PlayerSnake& pSnake);
	}
}

