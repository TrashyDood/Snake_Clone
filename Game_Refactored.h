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

namespace Game_Refactored
{
	enum RunID : ID
	{
		EXIT = 0,
		RUNNING = 1,
		RESTART = 2
	};

	enum GameState : ID
	{
		PLAYING = 0,
		STARTING = 1,
		PAUSED = 2,
		GAMEOVER = 3
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

		const bool operator==(const Tile& pOther) const
		{
			return mSymbol == pOther.mSymbol &&
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
		{
			return mTiles[pIndex];
		}

		Tile*& operator[](const Vector2Int pVector)
		{
			return operator[](pVector.x + pVector.y * mWidth);
		}

		TileMap(const UINT32 pWidth = 20, const UINT32 pHeight = 20) :
			mWidth(pWidth), mHeight(pHeight), mSize(pWidth* pHeight), mTiles(pWidth* pHeight, nullptr) {
		}
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
	public:
		Player& mPlayer;

	private:
		std::deque<Vector2Int> mPositions{};
		Vector2Int mDirection{};
		bool mIsAlive = true;
		int mPoints = -1,
			mSpeed = 2;

	public:
		void OnTick(const GameTime& pTime);
		void OnFixedTick(const GameTime& pTime);
		void TickInput(const GameTime& pTime);
		void TickCollisions(const GameTime& pTime);
	};

	class GameInstance
	{
	public:
		RunID mRunId = RunID::RUNNING;
		GameState mState = GameState::STARTING;

	private:
		TileMap mTileMap;
		UINT32 mLivingSnakes;
		std::vector<PlayerSnake> mActiveSnakes{};

		Tile mWallTile = { '@', FOREGROUND_GREEN | FOREGROUND_BLUE },
			mFruitTile = { '*', FOREGROUND_GREEN },
			mSuperFruitTile = { '*', FOREGROUND_RED | FOREGROUND_GREEN },
			mMegaFruitTile = { '*', FOREGROUND_RED | FOREGROUND_BLUE };

		std::array<Player, 4> mPlayers
		{
			Player({ 'D', 'W', 'A', 'S' }, { '0', FOREGROUND_RED | FOREGROUND_INTENSITY }),
			Player({ 'L', 'I', 'J', 'K' }, { '0', FOREGROUND_BLUE | FOREGROUND_INTENSITY }),
			Player({ VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN }, { '0', FOREGROUND_GREEN | FOREGROUND_INTENSITY }),
			Player({ VK_NUMPAD6, VK_NUMPAD8, VK_NUMPAD4, VK_NUMPAD5 }, { '0', FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY })
		};

	public:
		void OnInit();
		void OnTick(const GameTime& pTime);
		void OnFixedTick(const GameTime& pTime);
		void OnDraw(const GameTime& pTime);
		RunID OnExit();


	};

	RunID RunGame();
}



