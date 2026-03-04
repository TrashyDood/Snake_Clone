#pragma once

#include "console_renderer.h"
#include <chrono>
#include <array>
#include <vector>
#include <deque>

using namespace std::chrono;

namespace Game
{
	struct GameTime
	{
		duration<float> mTimeStep;
		steady_clock::time_point mTime{};
		float mDeltaTime = 0;
	};

	//template<typename T>
	//struct Vector2
	//{
	//	T x, y;

	//	template<typename U>
	//	auto operator-(const Vector2<U>& pOther) -> Vector2<decltype(x - pOther.x)> { return { x - pOther.x, y - pOther.y }; }
	//	template<typename U>
	//	auto operator+(const Vector2<U>& pOther) -> Vector2<decltype(x + pOther.x)> { return { x + pOther.x, y + pOther.y }; }
	//	template<typename U>
	//	auto operator*(const U pMultiplier) const -> Vector2<decltype(x* pMultiplier)> { return { x * pMultiplier, y * pMultiplier }; }
	//	template<typename U>
	//	auto operator/(const U pDivisor) const -> Vector2<decltype(x / pDivisor)> { return { x / pDivisor, y / pDivisor }; }

	//	Vector2(T pX = 0, T pY = 0) : x(pX), y(pY) {}
	//};

	//struct Vector2I32 : Vector2<int>
	//{
	//	using Vector2::Vector2;
	//};

	//struct Vector2F64 : Vector2<float>
	//{
	//	using Vector2::Vector2;
	//};

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

	// Heap allocated tile map.
	struct TileMap
	{
		std::vector<Tile*> mTiles;
		unsigned int mWidth = 0, mHeight = 0;

		Tile*& operator()(unsigned int x, unsigned int y) { return mTiles[x + y * mWidth]; }

		TileMap(unsigned int pWidth, unsigned int pHeight) :
			mWidth(pWidth), mHeight(pHeight) { mTiles.resize(pWidth * pHeight, nullptr); }

		// Constructor from 2D initializer list of Tile*
		TileMap(std::initializer_list<std::initializer_list<Tile*>> rows) :
			mWidth(rows.begin()->size()), mHeight(rows.size())
		{
			if (mHeight == 0)
				throw std::invalid_argument("TileMap cannot have zero height");

			mTiles.resize(mWidth * mHeight);

			unsigned int y = 0;
			for (auto& row : rows)
			{
				if (row.size() != mWidth)
					throw std::invalid_argument("All rows must have the same width");

				unsigned int x = 0;
				for (Tile* t : row)
					mTiles[x++ + y * mWidth] = t;

				++y;
			}
		}
	};

	struct PlayerPos
	{
		float x = 0,
			y = 0;

		const PlayerPos operator+(PlayerPos& pOther) const { return { x + pOther.x, y + pOther.y }; }
	};

	struct GameState
	{
		struct
		{
			float mSpeed;
			PlayerPos mVelocity = { 1, 0 };
			std::deque<PlayerPos> mPositions;
		} mPlayer;
		struct
		{
			float mHorizontal = 0,
				mVertical = -1;
		} mInput;

		bool mIsRunning = true;
		TileMap mTileMap = TileMap(1, 1);
		std::array<unsigned long, 6> mColors =
		{
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		};
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

	void Run();

	namespace Core
	{
		void OnInit(GameState& pGame);
		void OnTick(GameState& pGame, GameTime& pTime);
		void OnFixedTick(GameState& pGame, GameTime& pTime);
		void OnDraw(GameState& pGame, GameTime& pTime);
		void OnExit(GameState& pGame, GameTime& pTime);
	}

	namespace Draw
	{
		void DrawTile(Tile& pTile, unsigned int pX, unsigned int pY);
		void DrawTile(Tile* pTile, unsigned int pX, unsigned int pY);
		void DrawTileMap(TileMap& pTileMap);
	}

	namespace Mechanics
	{
		void InitTiles(GameState& pGame);
		void InitPlayer(GameState& pGame);
		TileMap GenTileMap(GameState& pGame);
		Tile& GetTileType(GameState& pGame, unsigned short pId);
	}
}

