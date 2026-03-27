#include "Game.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>
#include "Game_Refactored.h"

#define WRITELINE(x) std::cout << x << "\n" 
#define WRITE(x) std::cout << x;

using namespace std::chrono;
using namespace Rendering;

namespace Game
{
	RunID RunGame()
	{
		using namespace Core;

		GameInstance game{steady_clock::now().time_since_epoch().count()};
		GameTime time{};
		GameTime fixedTime{ duration<float>(1.0f / 8) };
		GameTime drawTime{ duration<float>(1.0f / 24) };

		OnInit(game);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> fixedAccumulator(0.0f), drawAccumulator(0.0f);

		while (game.mRunId == RunID::RUNNING) // Game loop.
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			time.mDeltaTime = time.mTimeStep.count();
			previousTime = time.mTime;

			fixedAccumulator += time.mTimeStep;
			drawAccumulator += time.mTimeStep;

			OnTick(game, time); // Normal tick.

			while (fixedAccumulator >= fixedTime.mTimeStep) // Fixed tick.
			{
				fixedTime.mTime = steady_clock::now();
				fixedTime.mDeltaTime = fixedTime.mTimeStep.count();

				OnFixedTick(game, fixedTime);
				//OnDraw(game, fixedTime);

				fixedAccumulator -= fixedTime.mTimeStep;
			}

			while (drawAccumulator >= drawTime.mTimeStep) // Draw tick.
			{
				drawTime.mTime = steady_clock::now();
				drawTime.mDeltaTime = drawTime.mTimeStep.count();

				OnDraw(game, drawTime);

				drawAccumulator -= drawTime.mTimeStep;
			}
		}

		return OnExit(game, time);
	}

	namespace Core
	{
		using namespace Draw;
		using namespace Mechanics;
		using namespace PlayerMechanics;

		void OnInit(GameInstance& pGame)
		{
			InitTileMap(pGame);
		}

		void OnTick(GameInstance& pGame, const GameTime& pTime)
		{
			if (Input::GetKey(VK_ESCAPE)) pGame.mRunId = RunID::EXIT;
			if (Input::GetKey(VK_F1)) pGame.mRunId = RunID::RESTART;

			if (pGame.mCurrentState == GameState::STARTING)
			{
				for (size_t i = 0; i < pGame.mPlayers.size(); i++)
				{
					Player& player = pGame.mPlayers[i];

					if (!PlayerIsActive(pGame, &player) &&
						(Input::GetKey(player.mInput.mRight) || Input::GetKey(player.mInput.mUp)
							|| Input::GetKey(player.mInput.mLeft) || Input::GetKey(player.mInput.mDown)))
						InitNewSnake(pGame, i);
				}

				if (Input::GetKey(VK_SPACE) && !pGame.mActiveSnakes.empty())
					pGame.mCurrentState = GameState::PLAYING;
			}

			for (auto& snake : pGame.mActiveSnakes)
				TickInput(pGame, pTime, snake);
		}

		void OnFixedTick(GameInstance& pGame, const GameTime& pTime)
		{
			for (auto& snake : pGame.mActiveSnakes)
				TickCollision(pGame, pTime, snake);

			for (auto& snake : pGame.mActiveSnakes)
				TickPlayer(pGame, pTime, snake);

			if (pGame.mCurrentState == PLAYING && pGame.mLivingSnakes < ((pGame.mPvpEnabled) ? 2 : 1))
				pGame.mCurrentState = GAMEOVER;
		}

		void OnDraw(GameInstance& pGame, const GameTime& pTime)
		{
			//renderBuffer.ClearBuffers();

			//renderBuffer.DrawOutline('+', pGame.mTileMap.mWidth + 2, pGame.mTileMap.mHeight + 2, 0, 0);
			DrawTileMap(pGame.mTileMap, 1, 1);
			DrawSideInfo(pGame, pTime);

			if (pGame.mCurrentState == GameState::STARTING)
			{
				DrawCenteredString(pGame.mTileMap, "Waiting for players...", 2);
				DrawCenteredString(pGame.mTileMap, "- Press 'Space' to start -", 1, 1);
			}

			if (pGame.mCurrentState == GameState::GAMEOVER)
			{
				DrawCenteredString(pGame.mTileMap, "Game Over", 2);
				DrawCenteredString(pGame.mTileMap, "- Press 'F1' to restart -", 1, 1);
			}

			//renderBuffer.RenderBuffers();
		}

		RunID OnExit(GameInstance& pGame, const GameTime& pTime)
		{ 
			return pGame.mRunId;
		}
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const UINT32 pX, const UINT32 pY) { /*if (pTile) renderBuffer.DrawCharCoords(pTile->mSymbol, pX, pY, pTile->mColor);*/ }
		void DrawTile(Tile& pTile, const UINT32 pX, const UINT32 pY) { /*renderBuffer.DrawCharCoords(pTile.mSymbol, pX, pY, pTile.mColor);*/ }

		void DrawTileMap(TileMap& pTileMap, const UINT32 pOffsetX, const UINT32 pOffsetY)
		{
			for (size_t x = 0; x < pTileMap.mWidth; x++)
				for (size_t y = 0; y < pTileMap.mHeight; y++)
					DrawTile(pTileMap[pTileMap(x, y)], x + pOffsetX, y + pOffsetY);
		}

		void DrawCenteredString(const TileMap& pTileMap, const std::string& pStr, UINT32 pOffsetX, UINT32 pOffsetY, Color pColor)
		{
			int x = pOffsetX + pTileMap.mWidth / 2 - pStr.size() / 2;
			int y = pOffsetY + pTileMap.mHeight / 2;
			//renderBuffer.DrawString(pStr, x, y, pColor);
		}

		void DrawSideInfo(GameInstance& pGame, const GameTime& pTime)
		{
			int offsetX = pGame.mTileMap.mWidth + 3;
			int offsetY = 1;

			//renderBuffer.DrawString(std::string("PvP: ") + (pGame.mPvpEnabled ? "Enabled" : "Disabled"), offsetX, 0);

			for (size_t i = 0; i < pGame.mActiveSnakes.size(); i++)
			{
				PlayerSnake& snake = pGame.mActiveSnakes[i];
				int y = offsetY + i * 3;

				//renderBuffer.DrawString("P" + std::to_string(i + 1), offsetX, y, snake.mPlayer->mTile.mColor);
				//renderBuffer.DrawString(snake.mIsAlive ? "Alive" : "Dead", offsetX + 4, y, snake.mPlayer->mTile.mColor);
				//renderBuffer.DrawString("Score: " + std::to_string(snake.mPoints), offsetX, y + 1, snake.mPlayer->mTile.mColor);
			}
		}
	}

	namespace Mechanics
	{
		using namespace PlayerMechanics;

		void InitTileMap(GameInstance& pGame)
		{
			TileMap map = TileMap(45, 25);
			Tile* wall = &pGame.mWallTile;
			std::mt19937 rng(pGame.mSeed);

			int gapSize = 3;
			int gapH = std::uniform_int_distribution<int>(gapSize + 1, map.mWidth - gapSize - 2)(rng);
			int gapV = std::uniform_int_distribution<int>(gapSize + 1, map.mHeight - gapSize - 2)(rng);

			for (int i = 0; i < (int)map.mWidth; i++)
			{
				if (abs(i - gapH) > gapSize) PlaceTile(map, wall, { i, 0 });
				if (abs(i - gapH) > gapSize) PlaceTile(map, wall, { i, (int)map.mHeight - 1 });
			}
			for (int i = 0; i < (int)map.mHeight; i++)
			{
				if (abs(i - gapV) > gapSize) PlaceTile(map, wall, { 0, i });
				if (abs(i - gapV) > gapSize) PlaceTile(map, wall, { (int)map.mWidth - 1, i });
			}

			// Edge walls
			int wallCount = std::uniform_int_distribution<int>(8, 14)(rng);
			for (int w = 0; w < wallCount; w++)
			{
				int x = std::uniform_int_distribution<int>(3, map.mWidth - 4)(rng);
				int y = std::uniform_int_distribution<int>(3, map.mHeight - 4)(rng);
				int len = std::uniform_int_distribution<int>(4, 12)(rng);
				bool horizontal = std::uniform_int_distribution<int>(0, 1)(rng);
				int gapPos = std::uniform_int_distribution<int>(0, len - 1)(rng);
				int gapSize2 = std::uniform_int_distribution<int>(1, 3)(rng);

				for (int i = 0; i < len; i++)
				{
					if (abs(i - gapPos) <= gapSize2) continue;
					PlaceTile(map, wall, horizontal ? Vector2Int{ x + i, y } : Vector2Int{ x, y + i });
				}
			}

			// Middle walls
			int midX = map.mWidth / 2, midY = map.mHeight / 2;
			int midWallCount = std::uniform_int_distribution<int>(4, 7)(rng);
			for (int w = 0; w < midWallCount; w++)
			{
				int x = std::uniform_int_distribution<int>(midX - 10, midX + 10)(rng);
				int y = std::uniform_int_distribution<int>(midY - 5, midY + 5)(rng);
				int length = std::uniform_int_distribution<int>(4, 10)(rng);
				bool horizontal = std::uniform_int_distribution<int>(0, 1)(rng);
				int gapPos = std::uniform_int_distribution<int>(0, length - 1)(rng);
				int gapSize2 = std::uniform_int_distribution<int>(1, 2)(rng);

				for (int i = 0; i < length; i++)
				{
					if (abs(i - gapPos) <= gapSize2) continue;
					PlaceTile(map, wall, horizontal ? Vector2Int{ x + i, y } : Vector2Int{ x, y + i });
				}
			}

			pGame.mTileMap = map;
			SpawnFruit(pGame, 2);
		}

		TileIndex GetRandomTilePos(GameInstance& pGame)
		{
			TileIndex targetIndex;
			std::mt19937 rng(pGame.mSeed++);

			do
			{
				std::uniform_int_distribution<int> dist(0, pGame.mTileMap.mSize - 1);
				targetIndex = (TileIndex)dist(rng);
			} while (pGame.mTileMap[pGame.mTileMap(targetIndex)]);

			return targetIndex;
		}

		void PlaceTileRandom(GameInstance& pGame, Tile* pTile) { PlaceTile(pGame.mTileMap, pTile, GetRandomTilePos(pGame)); }

		void SpawnFruit(GameInstance& pGame, unsigned short pCount)
		{
			std::mt19937 rng(pGame.mSeed++);
			while (pCount--)
			{
				int dist = std::uniform_int_distribution<int>(0, 7)(rng);
				Tile* fruit = dist == 0 ? &pGame.mMegaFruitTile
					: dist == 1 ? &pGame.mSuperFruitTile
					: &pGame.mFruitTile;
				PlaceTileRandom(pGame, fruit);
			}
		}

		void PlaceTile(TileMap& pTileMap, Tile* pTile, TileIndex pIndex)
		{
			pIndex = pTileMap(pIndex);
			Tile* current = pTileMap[pIndex];

			pTileMap.mCount += (!current && pTile) - (current && !pTile);
			pTileMap[pIndex] = pTile;
		}

		void PlaceTile(TileMap& pTileMap, Tile* pTile, Vector2Int pPosition) { PlaceTile(pTileMap, pTile, pTileMap(pPosition.x, pPosition.y)); }

		Vector2Int GetCornerPos(const TileMap& pTileMap, UINT32 pCorner)
		{
			int x = (pCorner == 1 || pCorner == 3) ? pTileMap.mWidth - 2 : 1;
			int y = (pCorner == 2 || pCorner == 3) ? pTileMap.mHeight - 2 : 1;
			return { x, y };
		}

		Vector2Int GetInwardDirection(const TileMap& pTileMap, Vector2Int pPos)
		{
			bool leftRight = pPos.x == 1 || pPos.x == pTileMap.mWidth - 2;
			if (leftRight)
				return { (pPos.x < (int)pTileMap.mWidth / 2) ? 1 : -1, 0 };
			else
				return { 0, (pPos.y < (int)pTileMap.mHeight / 2) ? 1 : -1 };
		}

		bool PlayerIsActive(GameInstance& pGame, const Player* pPlayer)
		{
			for (const PlayerSnake& snake : pGame.mActiveSnakes)
				if (pPlayer && snake.mPlayer == pPlayer)
					return true;
			return false;
		}

		UINT32 GetWinningPlayerID(GameInstance& pGame)
		{
			UINT32 result = 0;
			for (auto& player : pGame.mActiveSnakes)
				if (player.mPositions.size() > result)
					result = player.mPositions.size();

			return result;
		}
	}

	namespace PlayerMechanics
	{
		using namespace Game;
		using namespace Mechanics;

		void InitNewSnake(GameInstance& pGame, ID pPlayerId)
		{
			PlayerSnake snake{ &pGame.mPlayers[pPlayerId] };
			snake.mPositions.push_front(GetCornerPos(pGame.mTileMap, pPlayerId));
			snake.mDirection = GetInwardDirection(pGame.mTileMap, snake.mPositions.front());
			AddToSnake(pGame, snake);
			pGame.mActiveSnakes.push_back(snake);

			pGame.mLivingSnakes++;
			if (pGame.mActiveSnakes.size() > 1) pGame.mPvpEnabled = true;

			PlaceTile(pGame.mTileMap, &pGame.mPlayers[pPlayerId].mTile, snake.mPositions.front());
			SpawnFruit(pGame);
		}

		void AddToSnake(GameInstance& pGame, PlayerSnake& pSnake, unsigned short pCount)
		{
			pSnake.mPoints += pCount;

			while (pCount-- > 0)
				pSnake.mPositions.push_back(pSnake.mPositions.back());
		}

		void TickCollision(GameInstance& pGame, const GameTime pTime, PlayerSnake& pSnake)
		{
			if (pGame.mCurrentState != PLAYING)
				return;

			Tile* collidedTile = pGame.mTileMap[pGame.mTileMap(
				pSnake.mPositions.front() + pSnake.mDirection)];

			if (collidedTile)
				collidedTile->mColEvent(pGame, pSnake, collidedTile);
		}

		void TickPlayer(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pSnake)
		{
			if (pSnake.mPositions.empty()) return;

			if (!pSnake.mIsAlive)
			{
				PlaceTile(pGame.mTileMap, nullptr, pSnake.mPositions.front());
				pSnake.mPositions.pop_front();

				return;
			}

			if (pGame.mCurrentState != PLAYING) return;

			Vector2Int oldTail = pSnake.mPositions.back();
			PlaceTile(pGame.mTileMap, nullptr, oldTail);
			pSnake.mPositions.pop_back();

			Vector2Int newHead = pGame.mTileMap(pSnake.mPositions.front() + pSnake.mDirection);
			pSnake.mPositions.push_front(newHead);
			PlaceTile(pGame.mTileMap, &pSnake.mPlayer->mTile, newHead);
		}

		void TickInput(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pSnake)
		{
			if (pGame.mCurrentState != PLAYING)
				return;

			int inputX =
				Math::Clamp(Input::GetKey(pSnake.mPlayer->mInput.mRight))
				- Math::Clamp(Input::GetKey(pSnake.mPlayer->mInput.mLeft));
			int inputY =
				Math::Clamp(Input::GetKey(pSnake.mPlayer->mInput.mDown))
				- Math::Clamp(Input::GetKey(pSnake.mPlayer->mInput.mUp));

			if (inputX != 0 && pSnake.mDirection.x == 0)
			{
				pSnake.mDirection.y = 0;
				pSnake.mDirection.x = inputX;
			}

			if (inputY != 0 && pSnake.mDirection.y == 0)
			{
				pSnake.mDirection.x = 0;
				pSnake.mDirection.y = inputY;
			}
		}

		void KillSnake(GameInstance& pGame, PlayerSnake& pSnake)
		{
			if (!pSnake.mIsAlive)
				return;

			pGame.mLivingSnakes--;
			pSnake.mIsAlive = false;
		}
	}

	namespace CollisionEvents
	{
		using namespace Mechanics;
		using namespace PlayerMechanics;

		bool FruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			AddToSnake(pGame, pSnake);
			SpawnFruit(pGame);

			return false;
		}

		bool SuperFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			AddToSnake(pGame, pSnake, 2);
			SpawnFruit(pGame);

			return false;
		}

		bool MegaFruitCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			AddToSnake(pGame, pSnake, 1);
			SpawnFruit(pGame);

			return false;
		}

		bool DeathCol(GameInstance& pGame, PlayerSnake& pSnake, Tile* pTile)
		{
			KillSnake(pGame, pSnake);
			return true;
		}
	}
}