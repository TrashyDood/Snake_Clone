#include "Game.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>

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

		while (game.mRunId == RunID::Running) // Game loop.
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
		using namespace Game::Draw;
		using namespace Game::Mechanics;
		using namespace Game::PlayerMechanics;

		void OnInit(GameInstance& pGame)
		{
			InitTileMap(pGame);
		}

		void OnTick(GameInstance& pGame, const GameTime& pTime)
		{
			if (Input::GetKey(VK_ESCAPE))
			{
				pGame.mRunId = RunID::Exit;
				return;
			}

			if (Input::GetKey(VK_F1))
			{
				pGame.mRunId = RunID::Restart;
				return;
			}

			if (pGame.mState == GameState::Starting)
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
					pGame.mState = GameState::Playing;
				return;
			}

			for (auto& snake : pGame.mActiveSnakes)
				TickInput(pGame, pTime, snake);
		}

		void OnFixedTick(GameInstance& pGame, const GameTime& pTime)
		{
			if (pGame.mState == GameState::GameOver)
			{
				for (auto& snake : pGame.mActiveSnakes)
					TickPlayer(pGame, pTime, snake);
				if (pGame.mGameOverCounter > 0)
					pGame.mGameOverCounter--;
				else
					pGame.mRunId = RunID::Restart;
				return;
			}

			if (pGame.mState == GameState::Playing)
				for (auto& snake : pGame.mActiveSnakes)
					TickPlayer(pGame, pTime, snake);
		}

		void OnDraw(GameInstance& pGame, const GameTime& pTime)
		{
			renderBuffer.ClearBuffers();

			renderBuffer.DrawOutline('+', pGame.mTileMap.mWidth + 2, pGame.mTileMap.mHeight + 2, 0, 0);
			DrawTileMap(pGame.mTileMap, 1, 1);
			DrawSideInfo(pGame, pTime);

			if (pGame.mState == GameState::Starting)
				DrawCenteredString(pGame.mTileMap, "Waiting for players...");

			if (pGame.mState == GameState::GameOver)
				DrawCenteredString(pGame.mTileMap, "Game Over");

			renderBuffer.RenderBuffers();
		}

		RunID OnExit(GameInstance& pGame, const GameTime& pTime)
		{ 
			return pGame.mRunId;
		}
	}

	namespace Draw
	{
		void DrawTile(Tile* pTile, const UINT32 pX, const UINT32 pY) { if (pTile) renderBuffer.DrawCharCoords(pTile->mSymbol, pX, pY, pTile->mColor); }
		void DrawTile(Tile& pTile, const UINT32 pX, const UINT32 pY) { renderBuffer.DrawCharCoords(pTile.mSymbol, pX, pY, pTile.mColor); }

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
			renderBuffer.DrawString(pStr, x, y, pColor);
		}

		void DrawSideInfo(GameInstance& pGame, const GameTime& pTime)
		{
			int offsetX = pGame.mTileMap.mWidth + 3;
			int offsetY = 1;

			for (size_t i = 0; i < pGame.mActiveSnakes.size(); i++)
			{
				PlayerSnake& snake = pGame.mActiveSnakes[i];
				int y = offsetY + i * 3;

				renderBuffer.DrawString("P" + std::to_string(i + 1), offsetX, y, snake.mPlayer->mTile.mColor);
				renderBuffer.DrawString(snake.mIsAlive ? "Alive" : "Dead", offsetX + 3, y, snake.mPlayer->mTile.mColor);
				renderBuffer.DrawString("Score: " + std::to_string(snake.mPositions.size() - 2), offsetX, y + 1, snake.mPlayer->mTile.mColor);
			}
		}
	}

	namespace Mechanics
	{
		using namespace PlayerMechanics;

		void InitTileMap(GameInstance& pGame)
		{
			TileMap map = TileMap(60, 25);
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
				int len = std::uniform_int_distribution<int>(4, 10)(rng);
				bool horizontal = std::uniform_int_distribution<int>(0, 1)(rng);
				int gapPos = std::uniform_int_distribution<int>(0, len - 1)(rng);
				int gapSize2 = std::uniform_int_distribution<int>(1, 2)(rng);

				for (int i = 0; i < len; i++)
				{
					if (abs(i - gapPos) <= gapSize2) continue;
					PlaceTile(map, wall, horizontal ? Vector2Int{ x + i, y } : Vector2Int{ x, y + i });
				}
			}

			pGame.mTileMap = map;
			SpawnFruit(pGame);
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

		void SpawnTileRandom(GameInstance& pGame, Tile* pTile) { PlaceTile(pGame.mTileMap, pTile, GetRandomTilePos(pGame)); }

		void SpawnFruit(GameInstance& pGame)
		{
			std::mt19937 rng(pGame.mSeed++);
			int dist = std::uniform_int_distribution<int>(0, 7)(rng);
			Tile* fruit = dist == 0 ? &pGame.mMegaFruitTile
				: dist == 1 ? &pGame.mSuperFruitTile
				: &pGame.mFruitTile;
			SpawnTileRandom(pGame, fruit);
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
		using namespace Game::Mechanics;
		using namespace Game;

		void InitNewSnake(GameInstance& pGame, ID pPlayerId)
		{
			PlayerSnake snake{ &pGame.mPlayers[pPlayerId] };
			snake.mPositions.push_front(GetCornerPos(pGame.mTileMap, pPlayerId));
			snake.mDirection = GetInwardDirection(pGame.mTileMap, snake.mPositions.front());
			AddToSnake(pGame, snake);
			pGame.mActiveSnakes.push_back(snake);

			pGame.mLivingSnakes++;
			PlaceTile(pGame.mTileMap, &pGame.mPlayers[pPlayerId].mTile, snake.mPositions.front());

			//pGame.mPlayer.mInput = &pGame.mInputs[0];
			//pGame.mPlayer.mDirection = { 1,0 };
			//pGame.mPlayer.mIsAlive = true;
			//pGame.mPlayer.mPositions.push_front({ 5,5 });
			//AddToPlayer(pGame, pGame.mPlayer);
		}

		void AddToSnake(GameInstance& pGame, PlayerSnake& pSnake, unsigned short amount)
		{
			while (amount-- > 0)
				pSnake.mPositions.push_back(pSnake.mPositions.back());
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

			Tile* collidedTile = pGame.mTileMap[pGame.mTileMap(
				pSnake.mPositions.front() + pSnake.mDirection)];

			if (collidedTile && collidedTile->mColEvent(pGame, pSnake, collidedTile))
				return;

			Vector2Int oldTail = pSnake.mPositions.back();
			PlaceTile(pGame.mTileMap, nullptr, oldTail);
			pSnake.mPositions.pop_back();

			Vector2Int newHead = pGame.mTileMap(pSnake.mPositions.front() + pSnake.mDirection);
			pSnake.mPositions.push_front(newHead);
			PlaceTile(pGame.mTileMap, &pSnake.mPlayer->mTile, newHead);
		}

		void TickInput(GameInstance& pGame, const GameTime& pTime, PlayerSnake& pSnake)
		{
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

			if ((pGame.mActiveSnakes.size() > 1 && pGame.mLivingSnakes == 1) ||
				(pGame.mActiveSnakes.size() == 1 && pGame.mLivingSnakes < 1))
			{
				pGame.mState = GameState::GameOver;
				pGame.mGameOverCounter = 10;
			}
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
			AddToSnake(pGame, pSnake, 3);
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