#pragma once

#include "console_renderer.h"
#include <chrono>
#include <array>
#include <vector>
#include <deque>
#include <cmath>

using namespace std::chrono;

namespace Game
{
	struct GameTime
	{
		duration<float> mTimeStep;
		steady_clock::time_point mTime{};
		float mDeltaTime = 0;
	};

	struct Vector2F
	{
		float x = 0,
			y = 0;

		template<typename T>
		const Vector2F operator*(const T& pMultiplier) const { return { x * pMultiplier, y * pMultiplier }; }
		const Vector2F operator+(const Vector2F& pOther) const { return { x + pOther.x, y + pOther.y }; }
		const Vector2F operator-(const Vector2F& pOther) const { return { x - pOther.x, y - pOther.y }; }

		const Vector2F& operator+=(const Vector2F& pOther)
		{
			x += pOther.x;
			y += pOther.y;
			return *this;
		}

		template<typename T>
		const Vector2F& operator*=(const T& pMultiplier)
		{
			x *= pMultiplier;
			y *= pMultiplier;
			return *this;
		}
	};

	enum RunState : int
	{
		Exit = -1,
		Running = 0,
		Restart = 1
	};

	enum ColorID : unsigned short
	{
		Primary = 0,
		Secondary = 1,
		Tertiary = 2,
		Quaternary = 3
	};

	enum TileID : unsigned short
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
		unsigned short mId = 0;

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

		constexpr unsigned int operator()(unsigned int pX, unsigned int pY)
		{ 
			pX = (((pX) % mWidth) + mWidth) % mWidth;
			pY = (((pY) % mHeight) + mHeight) % mHeight;

			return (pY * mWidth) + pX;
		}

		Tile*& operator[](const unsigned int pIndex) { return mTiles[pIndex]; }
		Tile*& operator()(const Vector2F pCoords) { return mTiles[operator()((unsigned int)round(pCoords.x), (unsigned int)round(pCoords.y))]; }

		TileMap(const unsigned int pWidth = 20, const unsigned int pHeight = 20) :
			mWidth(pWidth), mHeight(pHeight), mSize(pWidth * pHeight), mTiles(pWidth* pHeight, nullptr) { }
	};

	struct PlayerSnake
	{
		float mSpeed = 3;
		Vector2F mDirection{};
		std::deque<Vector2F> mPositions;
	};

	struct GameState
	{
		PlayerSnake mPlayer;
		Vector2F mInput{ 0, 1 };

		RunState pRunState = RunState::Running;

		TileMap mTileMap;
		std::array<Tile, 6> mTileTypes =
		{
			Tile{ '#', FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ 'P', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
		};
	};

	RunState RunGame();
	void ExitGame(GameState& pGame, RunState pExitState = RunState::Exit);

	namespace Core
	{
		void OnInit(GameState& pGame);
		void OnTick(GameState& pGame, const GameTime& pTime);
		void OnFixedTick(GameState& pGame, const GameTime& pTime);
		void OnDraw(GameState& pGame, const GameTime& pTime);
		RunState OnExit(GameState& pGame, const GameTime& pTime);
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const unsigned int pPosition);
		void DrawTile(Tile* pTile, const unsigned int pX, const unsigned int pY);
		void DrawTileMap(TileMap& pTileMap);
	}

	namespace Mechanics
	{
		void InitTiles(GameState& pGame);
		void GenTileMap(GameState& pGame);
		Tile& GetTileType(GameState& pGame, const unsigned short pId);
		Tile* GetTile(GameState& pGame, const Vector2F pCoords);
		void SetTile(GameState& pGame, const Vector2F pCoords, Tile* pTile);
	}

	namespace Player
	{
		void InitPlayer(PlayerSnake& pPlayer);
		void TickPlayer(GameState& pGame, const GameTime& pTime, PlayerSnake& pPlayer);
		void AddToPlayer(PlayerSnake& pPlayer, unsigned short amount = 1);
		void TickInput(GameState& pGame, const GameTime& pTime);
	}
}

