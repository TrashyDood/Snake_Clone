#pragma once

#include "Utils.h"
#include "Tile.h"
#include <chrono>
#include <array>
#include <vector>
#include <deque>
#include <cmath>
#include <fstream>
#include <filesystem>

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
		Vector2Int operator-(const Vector2Int& other) const { return { x - other.x, y - other.y }; }

		template<typename T>
		Vector2Int operator*(const T pMult) const { return { x * pMult, y * pMult }; }
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
	public:
		uint32_t mWidth = 0, mHeight = 0;

	private:
		std::vector<Tile*> mTiles{};
		uint32_t mCount = 0;

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

		TileIndex operator()(TileIndex pIndex) const
		{
			pIndex %= static_cast<int>(Size());
			if (pIndex < 0) pIndex += Size();	

			return pIndex;
		}

		Tile*& operator[](const TileIndex pIndex) { return mTiles[pIndex]; }
		Tile*& operator[](const Vector2Int pVector) { return operator[](pVector.x + pVector.y * mWidth); }

		TileMap(const uint32_t pWidth, const uint32_t pHeight) :
			mWidth(pWidth), mHeight(pHeight), mTiles(pWidth * pHeight, nullptr) {
		}

		TileMap(const uint32_t pSize = 25) : mTiles(pSize, nullptr) {}

		const inline uint32_t Count() const { return mCount; }
		const inline uint32_t Size() const { return mTiles.size(); }
		const inline std::vector<Tile*>& GetTiles() const { return mTiles; }
		const inline Tile* GetTile(Vector2Int pVector) const { return mTiles[operator()(pVector.x, pVector.y)]; }
		const inline void SetTile(Tile* pTile, Vector2Int pVector) { mTiles[operator()(pVector.x, pVector.y)] = pTile; }
	};

	struct PlayerInput
	{
		KeyID mRight, mUp,
			mLeft, mDown;

		const void GetAxis(Vector2Int& pAxis) const;
		const bool IsInputting() const;
	};

	struct Player
	{
		PlayerInput mInput;
		Tile mTile;
		ID mId = 0;
	};

	class PlayerSnake
	{
	public:
		Player* mPlayer;
		Vector2Int mDirection;

	private:
		std::deque<Vector2Int> mPositions;
		bool mIsAlive = true;
		int mPoints = -1,
			mSpeed = 2;

	public:
		PlayerSnake(const PlayerSnake&) = delete;
		PlayerSnake& operator=(const PlayerSnake&) = delete;

		PlayerSnake(PlayerSnake&&) = default;
		PlayerSnake& operator=(PlayerSnake&&) = default;

		PlayerSnake(Player& pPlayer, Vector2Int pStartPos = {0, 0}, Vector2Int pStartDir = {1, 0}) :
			mPlayer(&pPlayer), mPositions(2, pStartPos), mDirection(pStartDir) { }

		const inline Vector2Int Head() const { return mPositions.front(); }
		const inline Vector2Int Tail() const { return mPositions.back(); }
		const inline bool IsAlive() const { return mIsAlive; }

		void OnTick(const GameTime& pTime, GameInstance& pGame);
		void OnFixedTick(const GameTime& pTime, GameInstance& pGame);
		void OnTickCollisions(const GameTime& pTime, GameInstance& pGame);

		void OnKilled();
	};

	class GameInstance
	{
	public:
		RunID mRunId = RUNNING;
		GameState mState = STARTING;
		TileMap mTileMap;

	private:
		uint32_t mLivingSnakes = 0;
		std::vector<PlayerSnake> mActiveSnakes{};

		Tile mWallTile = { '#', BACKGROUND_GREEN | BACKGROUND_BLUE },
			mFruitTile = { '*', FOREGROUND_GREEN, CollisionEvents::FruitCol },
			mSuperFruitTile = { '*', FOREGROUND_RED | FOREGROUND_GREEN, CollisionEvents::SuperFruitCol },
			mMegaFruitTile = { '*', FOREGROUND_RED | FOREGROUND_BLUE, CollisionEvents::MegaFruitCol };

		std::array<Player, 4> mPlayers
		{
			Player{{ 'D', 'W', 'A', 'S' }, { '0', BACKGROUND_RED | BACKGROUND_INTENSITY }},
			Player{{ 'L', 'I', 'J', 'K' }, { '0', BACKGROUND_GREEN | BACKGROUND_INTENSITY }},
			Player{{ VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN }, { '0', BACKGROUND_BLUE | BACKGROUND_INTENSITY }},
			Player{{ VK_NUMPAD6, VK_NUMPAD8, VK_NUMPAD4, VK_NUMPAD5 }, { '0', BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY }}
		};

	public:
		const inline bool PVPEnabled() const { return mActiveSnakes.size() > 1; }
		inline TileMap& GetTileMap() { return mTileMap; }

		inline void KillSnake(PlayerSnake& pSnake);
		inline void RemoveSnake(PlayerSnake& pSnake);

		void OnInit();
		void OnTick(const GameTime& pTime);
		void OnFixedTick(const GameTime& pTime);
		void OnDraw(const GameTime& pTime, Rendering::RendBuffer& pBuffer);
		RunID OnExit();
	private:
		void LoadLevel();
		void InitNewSnake(Player& pPlayer);
		bool IsPlayerActive(Player& pPlayer);
	};

	RunID RunGame();
}