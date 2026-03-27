#include "Game_Refactored.h"
#include "Utils.h"

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <format>

using namespace std::chrono;

namespace Game_Refactored
{
	RunID RunGame(std::string startLevel, float pTargetFixedTimeStep, float pTargetDrawTimestep)
	{
		GameTime time(duration<float>(0));
		GameTime fixedTime(duration<float>(1.0f / pTargetFixedTimeStep));
		GameTime drawTime(duration<float>(1.0f / pTargetDrawTimestep));
		Rendering::RendBuffer renderBuffer(10000);

		GameInstance game(std::filesystem::current_path() / "GameData");
		game.OnInit(time, fixedTime, drawTime);

		steady_clock::time_point previousTime = steady_clock::now();
		duration<float> fixedAccumulator(0.0f), drawAccumulator(0.0f);

		if (!startLevel.empty())
			game.LoadLevel(startLevel);

		while (game.mRunId == RunID::RUNNING) // Game loop.
		{
			time.mTime = steady_clock::now();
			time.mTimeStep = time.mTime - previousTime;
			time.mDeltaTime = time.mTimeStep.count();
			previousTime = time.mTime;

			fixedAccumulator += time.mTimeStep;
			drawAccumulator += time.mTimeStep;

			game.OnTick(time);

			while (fixedAccumulator >= fixedTime.mTimeStep) // Fixed tick.
			{
				fixedTime.mTime = steady_clock::now();
				fixedTime.mDeltaTime = fixedTime.mTimeStep.count();

				game.OnFixedTick(fixedTime);

				fixedAccumulator -= fixedTime.mTimeStep;
			}

			while (drawAccumulator >= drawTime.mTimeStep) // Draw tick.
			{
				drawTime.mTime = steady_clock::now();
				drawTime.mDeltaTime = drawTime.mTimeStep.count();

				game.OnDraw(drawTime, renderBuffer);

				drawAccumulator -= drawTime.mTimeStep;
			}
		}

		return game.OnExit();
	}

	void GameInstance::OnInit(GameTime& pTime, GameTime& pFixedTime, GameTime& pDrawTime)
	{
		for (size_t i = 0; i < mPlayers.size(); i++)
			mPlayers[i].mId = i;

		mLevelSelectMenu =
		{ "SNAKE ARENA",
			{{ std::format("Tickrate - {} t/s", (int)(1.0f / pFixedTime.mTimeStep.count())), [&pFixedTime](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton)
			{
				float tps = 1.0f / pFixedTime.mTimeStep.count();
				tps += 2.0f;
				if (tps > 12.0f) tps = 4.0f;
				pFixedTime.mTimeStep = duration<float>(1.0f / tps);

				pButton.mText = std::format("Tickrate - {} t/s", (int)(1.0f / pFixedTime.mTimeStep.count()));
			}},
			{ "Exit Game", [](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton) { pGame.mRunId = EXIT; }}
		}};

		mPauseMenu =
		{ "PAUSED",
			{{ "Resume", [](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton) { pGame.SetState(pGame.GetPrevState()); }},
			{ "Restart", [](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton) { pGame.LoadLevel(pGame.mCurrentLevel->mPath); }},
			{ "Exit To Menu", [](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton) { pGame.LoadLevel(); }}
		}};

		mGameOverMenu =
		{ "GAME OVER",
			{{ "Restart", [](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton) { pGame.LoadLevel(pGame.mCurrentLevel->mPath); }},
			{ "Exit To Menu", [](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton) { pGame.LoadLevel(); }}
		}};

		RegisterLevels();
	}

	void GameInstance::OnTick(GameTime& pTime)
	{
		if (Input::GetKey(VK_F1)) LoadLevel(GetLevelFromIndex(0));
		if (Input::GetKey(VK_F2)) LoadLevel(GetLevelFromIndex(1));
		if (Input::GetKey(VK_F3)) LoadLevel(GetLevelFromIndex(2));
		if (Input::GetKey(VK_F4)) LoadLevel(GetLevelFromIndex(3));
		if (Input::GetKey(VK_F5)) LoadLevel(GetLevelFromIndex(4));
		if (Input::GetKey(VK_F6)) LoadLevel(GetLevelFromIndex(5));
		if (Input::GetKey(VK_F7)) LoadLevel(GetLevelFromIndex(6));

		if (mCurrentState == MAINMENU) mLevelSelectMenu.OnTick(*this);
		else if (mCurrentState == PAUSED) mPauseMenu.OnTick(*this);
		else if (mCurrentState == GAMEOVER) mGameOverMenu.OnTick(*this);

		if (mCurrentLevel) mCurrentLevel->OnTick(pTime);

		if (mPrevLevel)
		{
			delete(mPrevLevel);
			mPrevLevel = nullptr;
		}
	}

	void GameInstance::OnFixedTick(GameTime& pTime)
	{
		if (mCurrentLevel)
			mCurrentLevel->OnFixedTick(pTime);
	}

	void GameInstance::OnDraw(GameTime& pTime, Rendering::RendBuffer& pBuffer)
	{
		pBuffer.ClearBuffers();  // moved here

		if (mCurrentLevel)
			mCurrentLevel->OnDraw(pTime, pBuffer);

		int cx = pBuffer.mWidth / 2 - 10;
		int cy = pBuffer.mHeight / 2 - 4;

		if (mCurrentState == MAINMENU) mLevelSelectMenu.OnDraw(pBuffer, cx, cy);
		else if (mCurrentState == PAUSED) mPauseMenu.OnDraw(pBuffer, cx, cy);
		else if (mCurrentState == GAMEOVER) mGameOverMenu.OnDraw(pBuffer, cx, cy);

		pBuffer.RenderBuffers();  // moved here
	}

	RunID GameInstance::OnExit()
	{
		return mRunId;
	}

	void GameInstance::RegisterLevels()
	{
		uint32_t index;

		mLevels.clear();
		mLevelSelectMenu.mButtons.clear();

		for (const auto& fileEntry : std::filesystem::directory_iterator(mLevelsPath))
			if (fileEntry.is_regular_file())
			{
				std::string firstLine;
				std::ifstream file(fileEntry.path());
				std::getline(file, firstLine);

				index = mLevels.size();
				UIButton button = { fileEntry.path().stem().string() + " [Highscore: " + firstLine + "]",
					[index](GameInstance& pGame, UIMenu& pMenu, UIButton& pButton)
					{
						pGame.LoadLevel(pGame.GetLevelFromIndex(index));
					} };
				mLevels.push_back(fileEntry.path().string());
				mLevelSelectMenu.mButtons.push_back(button);
			}
	}

	std::filesystem::path GameInstance::GetLevelFromIndex(uint32_t pIndex)
	{
		return mLevels[pIndex % mLevels.size()];
	}

	void GameInstance::LoadLevel(std::filesystem::path pPath)
	{
		if (mPrevLevel) return;

		mPrevLevel = mCurrentLevel;

		if (!pPath.empty())
		{
			mCurrentLevel = new LevelInstance(*this, pPath);
			mCurrentState = LEVELSTART;
			return;
		}

		mCurrentLevel = nullptr;
		mCurrentState = MAINMENU;
		RegisterLevels();
	}

	Tile* GameInstance::PickFruit(std::mt19937& pRand)
	{
		int rng = std::uniform_int_distribution<int>(0, 7)(pRand);

		Tile* fruit = (rng == 0) ? &mMegaFruitTile // 1/8 chance
			: (rng <= 2) ? &mSuperFruitTile        // 2/8 chance
			: &mFruitTile;                         // 5/8 chance

		return fruit;
	}


	void LevelInstance::OnTick(GameTime& pTime)
	{
		if (mGame.GetState() == LEVELSTART)
		{
			Vector2Int input = { 0,0 };
			for (auto& player : mGame.mPlayers)
				if (!IsPlayerActive(player) && player.mInput.IsInputting())
					InitNewSnake(player);

			if (Input::GetKey(VK_SPACE) && !mActiveSnakes.empty())
				mGame.SetState(PLAYING);
		}

		if (Input::GetKeyDown(VK_ESCAPE))
		{
			if (mGame.GetState() == PAUSED) mGame.SetState(mGame.GetPrevState());
			else mGame.SetState(PAUSED);
		}

		for (auto& snake : mActiveSnakes)
			snake.OnTick(pTime, *this);
	}

	void LevelInstance::OnFixedTick(GameTime& pTime)
	{
		for (auto& snake : mActiveSnakes)
			snake.OnFixedTick(pTime, *this);
	}

	void LevelInstance::OnDraw(GameTime& pTime, Rendering::RendBuffer& pBuffer)
	{
		int mapCenterX = pBuffer.mWidth / 2 - mTileMap.mWidth / 2,
			mapCenterY = pBuffer.mHeight / 2 - mTileMap.mHeight / 2;

		Tile* tile = nullptr;
		for (int x = 0; x < mTileMap.mWidth; x++)
			for (int y = 0; y < mTileMap.mHeight; y++)
			{
				tile = mTileMap[{ x, y }];

				if (tile) // Draw tile map.
					pBuffer.DrawCharCoords(tile->mSymbol, x + mapCenterX, y + mapCenterY, tile->mColor);
				else if (x > 0 && x < mTileMap.mWidth - 1 && y > 0 && y < mTileMap.mHeight - 1) // Draw floor.
					pBuffer.DrawCharCoords('.', x + mapCenterX, y + mapCenterY, white);
				else // Draw borders.
					pBuffer.DrawCharCoords('*', x + mapCenterX, y + mapCenterY, FOREGROUND_RED | FOREGROUND_BLUE);
			}

		int offsetX = mapCenterX + mTileMap.mWidth + 1;
		int offsetY = mapCenterY;

		pBuffer.DrawString(std::string("PvP: ") + (PVPEnabled() ? "Enabled" : "Disabled") +
			(", Highscore: ") + std::to_string(mHighScore), offsetX, offsetY - 1);

		for (size_t i = 0; i < mActiveSnakes.size(); i++)
		{
			PlayerSnake& snake = mActiveSnakes[i];
			int y = offsetY + i * 3;
			pBuffer.DrawString("P" + std::to_string(i + 1) + ":", offsetX, y, snake.mPlayer->mTile.mColor);
			pBuffer.DrawString(snake.IsAlive() ? "Alive" : "Dead ", offsetX + 4, y, snake.mPlayer->mTile.mColor);
			pBuffer.DrawString("Score: " + std::to_string(snake.Score()), offsetX + 4, y + 1, snake.mPlayer->mTile.mColor);
		}

		pBuffer.DrawString(mPath.string());
	}

	inline void LevelInstance::KillSnake(PlayerSnake& pSnake)
	{
		if (!pSnake.IsAlive())
			return;

		mLivingSnakes--;
		pSnake.OnKilled(*this);
		OnGameOver();
	}

	inline void LevelInstance::RemoveSnake(PlayerSnake& pSnake)
	{
		auto it = std::find_if(mActiveSnakes.begin(), mActiveSnakes.end(),
			[&pSnake](const PlayerSnake& s) { return &s == &pSnake; });

		if (it != mActiveSnakes.end())
			mActiveSnakes.erase(it);
	}

	void inline LevelInstance::InitNewSnake(Player& pPlayer)
	{
		Vector2Int corners[4] =
		{
			{ 1, 1 },
			{ mTileMap.mWidth - 2, mTileMap.mHeight - 2 },
			{ mTileMap.mWidth - 2, 1 },
			{ 1, mTileMap.mHeight - 2 },
		};

		Vector2Int directions[4] =
		{
			{ 1, 0 },
			{ -1, 0 },
			{ 0, 1 },
			{ 0, -1 },
		};

		int id = mActiveSnakes.size();
		mActiveSnakes.push_back(PlayerSnake(pPlayer, id, corners[id], directions[id]));
		mTileMap.SetTile(&pPlayer.mTile, corners[id]);
		mLivingSnakes++;

		SpawnRandomFruit();
	}

	inline const TileIndex LevelInstance::RandomTileIndex()
	{ return std::uniform_int_distribution<int>(0, mTileMap.Size() - 1)(mRand); }

	inline const TileIndex LevelInstance::RandomEmptyTileIndex() // Brute force random.
	{
		TileIndex result = mTileMap(RandomTileIndex());

		for (size_t i = 0; i < 100 && mTileMap[result]; i++)
			result = mTileMap(RandomTileIndex());

		return result;
	}

	inline void LevelInstance::SpawnRandomFruit()
	{ mTileMap[RandomEmptyTileIndex()] = mGame.PickFruit(mRand); }

	void LevelInstance::OnGameOver()
	{
		if (PVPEnabled() && mLivingSnakes >= 2)
			return;

		const PlayerSnake& highestScoreSnake = GetHighestScoreSnake();
		const PlayerSnake& lastSnake = GetLastSnake();

		if (highestScoreSnake.Score() > mHighScore)
		{
			mHighScore = highestScoreSnake.Score();
			std::ifstream fileIn(mPath);
			std::string content((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>());
			fileIn.close();

			// Replace first line with new highscore
			size_t newline = content.find('\n');
			content = std::to_string(mHighScore) + (newline != std::string::npos ? content.substr(newline) : "");

			std::ofstream fileOut(mPath);
			fileOut << content;
		}

		mGame.SetState(GAMEOVER);
	}

	LevelInstance::LevelInstance(GameInstance& pGame, std::filesystem::path pPath) :
		mGame(pGame), mPath(pPath)
	{
		std::ifstream file(pPath);

		if (!file.is_open())
			throw std::runtime_error("Failed to open: " + pPath.string());

		std::string line;
		uint32_t width = 0, height = 0;

		// Read highscore from first line
		std::getline(file, line);
		mHighScore = std::stoi(line);

		while (std::getline(file, line))
		{
			if (line.size() > width) width = line.size();
			height++;
		}

		file.clear();
		file.seekg(0);
		std::getline(file, line); // Skip highscore line

		mTileMap = TileMap(width, height);
		int y = 0;
		while (std::getline(file, line))
		{
			for (int x = 0; x < line.size(); x++)
				if (line[x] != ' ')
					mTileMap.SetTile(&pGame.mWallTile, { x, y });
			y++;
		}
	}

	inline bool LevelInstance::IsPlayerActive(Player& pPlayer)
	{
		for (auto& snake : mActiveSnakes)
			if (snake.mPlayer == &pPlayer)
				return true;
		return false;
	}

	inline const PlayerSnake& LevelInstance::GetHighestScoreSnake()
	{
		PlayerSnake* result = &mActiveSnakes[0];

		for (size_t i = 1; i < mActiveSnakes.size(); i++)
			if (mActiveSnakes[i].Score() > result->Score())
				result = &mActiveSnakes[i];

		return *result;
	}

	inline const PlayerSnake& LevelInstance::GetLastSnake()
	{
		PlayerSnake* result = &mActiveSnakes[0];

		for (size_t i = 1; i < mActiveSnakes.size(); i++)
			if (mActiveSnakes[i].IsAlive())
				result = &mActiveSnakes[i];

		return *result;
	}


	void PlayerSnake::OnTick(GameTime& pTime, LevelInstance& pLevel)
	{
		if (mIsAlive && pLevel.mGame.GetState() == PLAYING)
			mPlayer->mInput.GetAxis(mDirection);
	}

	void PlayerSnake::OnFixedTick(GameTime& pTime, LevelInstance& pLevel)
	{
		if (mPositions.empty()) return;

		if (!mIsAlive)
		{
			pLevel.mTileMap.SetTile(nullptr, Head());
			mPositions.pop_front();

			return;
		}

		if (OnTickCollisions(pTime, pLevel)) return;
		if (pLevel.mGame.GetState() != PLAYING) return;

		pLevel.mTileMap.SetTile(nullptr, Tail());
		mPositions.pop_back();

		Vector2Int newHead = pLevel.mTileMap(Head() + mDirection);
		pLevel.mTileMap.SetTile(&mPlayer->mTile, newHead);
		mPositions.push_front(newHead);
	}

	bool PlayerSnake::OnTickCollisions(GameTime& pTime, LevelInstance& pLevel)
	{
		Tile* collidedTile = pLevel.mTileMap[pLevel.mTileMap(
			Head() + mDirection)];

		if (collidedTile)
			return collidedTile->mColEvent(pLevel, *this, collidedTile);

		return false;
	}

	void PlayerSnake::OnKilled(LevelInstance& pLevel)
	{
		mIsAlive = false;
	}

	void PlayerSnake::Add(LevelInstance& pLevel, int32_t pAmount)
	{
		mPoints += pAmount;

		while (pAmount--)
			mPositions.push_back(Tail());
	}

	const void PlayerInput::GetAxis(Vector2Int& pAxis) const
	{
		int inputX =
			Math::Clamp(Input::GetKey(mRight))
			- Math::Clamp(Input::GetKey(mLeft));
		int inputY =
			Math::Clamp(Input::GetKey(mDown))
			- Math::Clamp(Input::GetKey(mUp));

		if (inputX != 0 && pAxis.x == 0)
		{
			pAxis.y = 0;
			pAxis.x = inputX;
		}

		if (inputY != 0 && pAxis.y == 0)
		{
			pAxis.x = 0;
			pAxis.y = inputY;
		}
	}

	const bool PlayerInput::IsInputting() const
	{
		return Input::GetKey(mRight) + Input::GetKey(mLeft) +
			Input::GetKey(mDown) + Input::GetKey(mUp);
	}


	namespace CollisionEvents
	{
		bool FruitCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile)
		{
			pSnake.Add(pLevel);
			pLevel.SpawnRandomFruit();

			return false;
		}

		bool SuperFruitCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile)
		{
			pSnake.Add(pLevel, 2);
			pLevel.SpawnRandomFruit();

			return false;
		}

		bool MegaFruitCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile)
		{
			pSnake.Add(pLevel, 3);
			pLevel.SpawnRandomFruit();

			return false;
		}

		bool DeathCol(LevelInstance& pLevel, PlayerSnake& pSnake, Tile* pTile)
		{
			pLevel.KillSnake(pSnake);
			return true;
		}
	}
}