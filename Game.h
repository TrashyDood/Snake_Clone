#pragma once

#include "console_renderer.h"
#include <chrono>
#include <unordered_map>
#include <string>
#include <array>
#include <vector>

using namespace std::chrono;

namespace Game
{
	struct GameTime
	{
		duration<float> mTimeStep;
		steady_clock::time_point mTime{};
		float mDeltaTime = 0;
	};

	struct Coord
	{
		size_t x, y;

		Coord operator+(Coord& pOther) { return Coord(x + pOther.x, y + pOther.y); }
		Coord operator-(Coord& pOther) { return Coord(x - pOther.x, y - pOther.y); }

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
		bool mIsPassable = false;
		unsigned short mId = 0;
	};

	// Heap allocated tile map.
	struct TileMap
	{
		std::vector<Tile*> mTiles;
		size_t mWidth = 0, mHeight = 0;

		Tile*& operator()(size_t x, size_t y) { return mTiles[x + y * mWidth]; }
		Tile* operator()(size_t x, size_t y) const { return mTiles[x + y * mWidth]; }
		Tile*& operator[](Coord c) { return mTiles[c.x + c.y * mWidth]; }
		Tile* operator[](Coord c) const { return mTiles[c.x + c.y * mWidth]; }

		TileMap(size_t pWidth, size_t pHeight) :
			mWidth(pWidth), mHeight(pHeight) { mTiles.resize(pWidth * pHeight, nullptr); }

		// Constructor from 2D initializer list of Tile*
		TileMap(std::initializer_list<std::initializer_list<Tile*>> rows) :
			mWidth(rows.begin()->size()), mHeight(rows.size())
		{
			if (mHeight == 0)
				throw std::invalid_argument("TileMap cannot have zero height");

			mTiles.resize(mWidth * mHeight);

			size_t y = 0;
			for (auto& row : rows)
			{
				if (row.size() != mWidth)
					throw std::invalid_argument("All rows must have the same width");

				size_t x = 0;
				for (Tile* t : row)
					mTiles[x++ + y * mWidth] = t;

				++y;
			}
		}
	};

	struct GameState
	{
		struct
		{
			float mSpeed = 7,
				mDirX = 0, mDirY = 0,
				mPosX = 0, mPosY = 0;
		} mPlayer1, mPlayer2, mPlayer3, mPlayer4;

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
			Tile{ '#', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ 'P', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
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
		void DrawTile(Tile& pTile, Coord pCoords);
		void DrawTile(Tile* pTile, Coord pCoords);
		void DrawTileMap(TileMap& pTileMap);
	}

	namespace Mechanics
	{
		void InitTiles(GameState& pGame);
		TileMap GenTileMap(GameState& pGame);
		Tile& GetTileType(GameState& pGame, unsigned short pId);
	}
}

