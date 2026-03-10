#pragma once

#include "Utils.h"
#include "Tile.h"
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
	public:
		char mSymbol;
		Color mColor;
		CollisionEvent mColEvent = CollisionEvents::DeathCol;

		const bool operator==(const Tile& pOther) const
		{
			return mSymbol == pOther.mSymbol &&
				mColor == pOther.mColor;
		}
	};

	class TileMap
	{
		std::vector<Tile*> mTiles{};
		UINT32 mWidth = 0, mHeight = 0, mSize = 0, mCount = 0;

	public:
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

		Tile*& operator[](const TileIndex pIndex) { return mTiles[pIndex]; }
		Tile*& operator[](const Vector2Int pVector) { return operator[](pVector.x + pVector.y * mWidth); }

		TileMap(const UINT32 pWidth = 20, const UINT32 pHeight = 20) :
			mWidth(pWidth), mHeight(pHeight), mSize(pWidth* pHeight), mTiles(pWidth* pHeight, nullptr) {
		}

		const UINT32 GetCount() const { return mCount; }
		const UINT32 GetSize() const { return mSize; }
		const UINT32 GetWidth() const { return mWidth; }
		const UINT32 GetHeight() const { return mHeight; }
		const std::vector<Tile*>& GetTiles() const { return mTiles; }
		const Tile* GetTile(Vector2Int pVector) const { return mTiles[operator()(pVector.x, pVector.y)]; }
		const void SetTile(Tile* pTile, Vector2Int pVector) { mTiles[operator()(pVector.x, pVector.y)] = pTile; }
	};

	struct PlayerInput
	{
		KeyID mRight, mUp,
			mLeft, mDown,
			mActivate;

		const void GetAxis(Vector2Int& pAxis);
	};

	class Player
	{
	public:
		PlayerInput mInput;
		Tile mTile;
		ID mId = 0;
	};

	class PlayerSnake
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
		void TickCollisions(const GameTime& pTime);
	};

	class GameInstance
	{
	public:
		RunID mRunId = RunID::RUNNING;
		GameState mState = GameState::STARTING;

	private:
		TileMap mTileMap{};
		UINT32 mLivingSnakes = 0;
		std::vector<PlayerSnake> mActiveSnakes{};

		std::array<Player, 4> mPlayers
		{
			Player({ 'D', 'W', 'A', 'S' }, {'0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE}),
			Player({ 'L', 'I', 'J', 'K' }, {'0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE}),
			Player({ VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN }, {'0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE}),
			Player({ VK_NUMPAD6, VK_NUMPAD8, VK_NUMPAD4, VK_NUMPAD5 }, {'0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE})
		};

	public:
		void OnInit();
		void OnTick(const GameTime& pTime);
		void OnFixedTick(const GameTime& pTime);
		void OnDraw(const GameTime& pTime, Rendering::RendBuffer& pBuffer);
		RunID OnExit();
	};

	RunID RunGame();
	void GetAxis(Vector2Int& pAxis);
}



