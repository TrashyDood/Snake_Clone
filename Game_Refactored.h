#pragma once

#include "Utils.h"

#include <chrono>
#include <array>
#include <vector>
#include <deque>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <random>
#include <unordered_map>
#include <functional>

using namespace std::chrono;

typedef unsigned int TileIndex;
typedef unsigned short ID;

namespace Game_Refactored
{
	// Too lazy to make new files...
#pragma region Forward Declarations
	struct GameInstance;
	struct LevelInstance;
	struct Player;
	struct PlayerSnake;
	struct Tile;
	struct UIMenu;
	struct UIButton;

	using CollisionEvent = bool(*)(LevelInstance& pLevel, PlayerSnake&, Tile*);
	using UIButtonCallback = std::function<void(GameInstance&, UIMenu&, UIButton&)>;

	namespace CollisionEvents
	{
		bool FruitCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile);
		bool SuperFruitCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile);
		bool MegaFruitCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile);
		bool DeathCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile);
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
		LEVELSTART = 1,
		PAUSED = 2,
		GAMEOVER = 3,
		MAINMENU = 4,
		SETTINGS = 5
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

		GameTime(duration<float> pTimeStep) : mTimeStep(pTimeStep) { mDeltaTime = pTimeStep.count(); }
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
		Player* mPlayer = nullptr;
		Vector2Int mDirection{};

	private:
		std::deque<Vector2Int> mPositions;
		bool mIsAlive = true;
		int mPoints = 0;
		int32_t mId = 0;

	public:
		const inline uint32_t Score() const { return mPoints; }
		const inline bool IsAlive() const { return mIsAlive; }
		const inline Vector2Int& Head() const { return mPositions.front(); }
		const inline Vector2Int& Tail() const { return mPositions.back(); }

		PlayerSnake(const PlayerSnake&) = delete;
		PlayerSnake(PlayerSnake&&) = default;

		PlayerSnake(Player& pPlayer, uint32_t pId, Vector2Int pStartPos = { 0, 0 }, Vector2Int pStartDir = { 1, 0 }) :
			mPlayer(&pPlayer), mId(pId), mPositions(2, pStartPos), mDirection(pStartDir) { }

		PlayerSnake& operator=(const PlayerSnake&) = delete;
		PlayerSnake& operator=(PlayerSnake&&) = default;

		void OnTick(GameTime& pTime, LevelInstance& pLevel);
		void OnFixedTick(GameTime& pTime, LevelInstance& pLevel);
		bool OnTickCollisions(GameTime& pTime, LevelInstance& pLevel);

		void OnKilled(LevelInstance& pLevel);
		void Add(LevelInstance& pLevel, int32_t pAmount = 1);
	};

	struct UIButton
	{
		std::string mText;
		UIButtonCallback mCallback;
	};

	struct UIMenu
	{
		std::string mTitle;
		std::vector<UIButton> mButtons;
		int mSelected = 0;

		bool Confirmed() { return Input::GetKeyDown(VK_SPACE); }

		void OnTick(GameInstance& pGame)
		{
			if (Input::GetKeyDown(VK_DOWN)) mSelected = (mSelected + 1) % mButtons.size();
			if (Input::GetKeyDown(VK_UP)) mSelected = (mSelected - 1 + mButtons.size()) % mButtons.size();

			if (Confirmed())
				mButtons[mSelected].mCallback(pGame, *this, mButtons[mSelected]);
		}

		void OnDraw(Rendering::RendBuffer& pBuffer, int pX, int pY) const
		{
			pBuffer.DrawString(mTitle, pX, pY, FOREGROUND_RED | FOREGROUND_INTENSITY);

			for (int i = 0; i < mButtons.size(); i++)
			{
				Color col = (i == mSelected) ? FOREGROUND_GREEN | FOREGROUND_INTENSITY : white;
				std::string line = (i == mSelected ? "> " : "  ") + mButtons[i].mText;
				pBuffer.DrawString(line, pX, pY + 2 + i, col);
			}
		}
	};

	// Represents the entire game.
	class GameInstance
	{
	private:
		LevelInstance* mCurrentLevel = nullptr;
		LevelInstance* mPrevLevel = nullptr;
		std::vector<std::string> mLevels{};
		GameState mCurrentState = MAINMENU,
			mPrevState = MAINMENU;

		UIMenu mLevelSelectMenu, mPauseMenu, mGameOverMenu;

	public:
		RunID mRunId = RUNNING;
		const std::filesystem::path mWorkingDir,
			mLevelsPath = mWorkingDir / "Levels";

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
		GameInstance(std::filesystem::path pWorkingDir) :
			mWorkingDir(pWorkingDir) {}

		void OnInit(GameTime& pTime, GameTime& pFixedTime, GameTime& pDrawTime);
		void OnTick(GameTime& pTime);
		void OnFixedTick(GameTime& pTime);
		void OnDraw(GameTime& pTime, Rendering::RendBuffer& pBuffer);
		RunID OnExit();

		void RegisterLevels();
		std::filesystem::path GetLevelFromIndex(uint32_t pIndex);
		void LoadLevel(std::filesystem::path pPath = "");
		Tile* PickFruit(std::mt19937& pRand);

		inline const GameState GetState() const { return mCurrentState; }
		inline const GameState GetPrevState() const { return mPrevState; }
		inline const GameState SetState(GameState pNewState)
		{
			mPrevState = mCurrentState;
			return (mCurrentState = pNewState);
		}
	};

	// Represents a loaded level.
	struct LevelInstance
	{
		GameInstance& mGame;
		TileMap mTileMap;
		uint32_t mLivingSnakes = 0,
			mHighScore = 0;
		std::vector<PlayerSnake> mActiveSnakes{};
		std::mt19937 mRand;
		const std::filesystem::path mPath;

		void OnTick(GameTime& pTime);
		void OnFixedTick(GameTime& pTime);
		void OnDraw(GameTime& pTime, Rendering::RendBuffer& pBuffer);

		const inline bool PVPEnabled() const { return mActiveSnakes.size() > 1; }

		inline void KillSnake(PlayerSnake& pSnake);
		inline void RemoveSnake(PlayerSnake& pSnake);
		inline void InitNewSnake(Player& pPlayer);
		inline bool IsPlayerActive(Player& pPlayer);
		inline const PlayerSnake& GetHighestScoreSnake();
		inline const PlayerSnake& GetLastSnake();

		const inline TileIndex RandomTileIndex();
		const inline TileIndex RandomEmptyTileIndex();
		inline void SpawnRandomFruit();

		void OnGameOver();

		LevelInstance(GameInstance& pGame, std::filesystem::path pFilePath);
	};

	RunID RunGame(std::string startLevel = "", float pTargetFixedTimestep = 8, float pTargetDrawTimeStep = 8);
}