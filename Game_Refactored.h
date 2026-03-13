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
#include <random>

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
		int mPoints = 0;
		int32_t mId;

	public:
		PlayerSnake(const PlayerSnake&) = delete;
		PlayerSnake(PlayerSnake&&) = default;

		PlayerSnake(Player& pPlayer, uint32_t pId, Vector2Int pStartPos = { 0, 0 }, Vector2Int pStartDir = { 1, 0 }) :
			mPlayer(&pPlayer), mId(pId), mPositions(2, pStartPos), mDirection(pStartDir) { }

		PlayerSnake& operator=(const PlayerSnake&) = delete;
		PlayerSnake& operator=(PlayerSnake&&) = default;

		const inline Vector2Int Head() const { return mPositions.front(); }
		const inline Vector2Int Tail() const { return mPositions.back(); }
		const inline bool IsAlive() const { return mIsAlive; }

		void OnTick(const GameTime& pTime, LevelInstance* pLevel);
		void OnFixedTick(const GameTime& pTime, LevelInstance* pLevel);
		void OnTickCollisions(const GameTime& pTime, LevelInstance* pLevel);

		void OnKilled();
		void Add(LevelInstance* pLevel, int32_t pAmount = 1);
	};

	struct LevelInstance
	{
		GameInstance& mGame;
		TileMap mTileMap;
		uint32_t mLivingSnakes = 0;
		std::vector<PlayerSnake> mActiveSnakes{};
		std::mt19937 mRand;

		inline void KillSnake(PlayerSnake& pSnake);
		inline void RemoveSnake(PlayerSnake& pSnake);
		inline void InitNewSnake(Player& pPlayer);

		const inline TileIndex RandomTileIndex();
		const inline TileIndex RandomEmptyTileIndex();

		inline void SpawnRandomFruit(GameInstance& pGame);

		LevelInstance(GameInstance& pGame, std::filesystem::path pFilePath) : mGame(pGame)
		{
			//std::filesystem::path filePath = std::filesystem::current_path() / "GameData\\Levels\\Level1.txt";
			std::ifstream file(pFilePath);

			if (!file.is_open())
				throw std::runtime_error("Failed to open LevelData.txt");

			std::string line;
			uint32_t width = 0, height = 0;

			// First pass: find width and height
			while (std::getline(file, line))
			{
				if (line.size() > width) width = line.size();
				height++;
			}

			file.clear();
			file.seekg(0);

			mTileMap = TileMap(width, height);

			// Second pass: set tiles
			int y = 0;
			while (std::getline(file, line))
			{
				for (int x = 0; x < line.size(); x++)
					if (line[x] != ' ')
						mTileMap.SetTile(&pGame.mWallTile, { x, y });
				y++;
			}
		}
	};

	class GameInstance
	{
	public:
		RunID mRunId = RUNNING;
		GameState mState = STARTING;
		LevelInstance* mLevel = nullptr;

		Tile mWallTile = { '#', BACKGROUND_GREEN | BACKGROUND_BLUE },
			mFruitTile = { '+', FOREGROUND_GREEN, CollisionEvents::FruitCol },
			mSuperFruitTile = { '+', FOREGROUND_RED | FOREGROUND_GREEN, CollisionEvents::SuperFruitCol },
			mMegaFruitTile = { '+', FOREGROUND_RED | FOREGROUND_BLUE, CollisionEvents::MegaFruitCol };

		std::array<Player, 4> mPlayers
		{
			Player{{ 'D', 'W', 'A', 'S' }, { '0', BACKGROUND_RED | BACKGROUND_INTENSITY }},
			Player{{ 'L', 'I', 'J', 'K' }, { '0', BACKGROUND_GREEN | BACKGROUND_INTENSITY }},
			Player{{ VK_RIGHT, VK_UP, VK_LEFT, VK_DOWN }, { '0', BACKGROUND_BLUE | BACKGROUND_INTENSITY }},
			Player{{ VK_NUMPAD6, VK_NUMPAD8, VK_NUMPAD4, VK_NUMPAD5 }, { '0', BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY }}
		};

	public:
		void OnInit();
		void OnTick(const GameTime& pTime);
		void OnFixedTick(const GameTime& pTime);
		void OnDraw(const GameTime& pTime, Rendering::RendBuffer& pBuffer);
		RunID OnExit();
	private:
		bool IsPlayerActive(Player& pPlayer);
	};

	RunID RunGame();
}