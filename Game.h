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
		duration<float> mFixedTimeStep, mTimeStep;
		steady_clock::time_point mFixedTime{}, mTime{};
		float mFixedDeltaTime = 0, mDeltaTime = 0;

		GameTime(duration<float> pTargetFixedTimeStep) :
			mFixedTimeStep(pTargetFixedTimeStep), mTimeStep() {}
	};

	struct Tile
	{
		std::string mName;
		char mSymbol;
		unsigned short mColor;
		bool mIsPassable = false;

		bool operator==(Tile& pOther) {}
	};

	struct TileMap
	{
		std::array<Tile, buffer_size> mTiles;

		Tile& operator[](size_t pIndex) { return mTiles[pIndex]; }
	};

	enum TileID : unsigned short
	{
		Wall = 0,
		Fruit = 1,
		Snake = 2
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
		TileMap mTileMap;
		std::unordered_map<std::string, unsigned short> mTileLookup{};
		std::array<Tile, 3> mTileTypes =
		{
			Tile{ "Wall", '#', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
			Tile{ "Fruit", 'P', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, true },
			Tile{ "Snake", '0', FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE },
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
		void DrawTile(Tile& pTile, size_t x, size_t y);
		void DrawTileMap(TileMap& mTileMap, size_t width, size_t height, size_t x = 0, size_t y = 0);
	}

	namespace Mechanics
	{
		Tile& GetTileType(GameState& pGame, unsigned short pId);
		Tile& GetTileType(GameState& pGame, std::string pName);
	}
}

