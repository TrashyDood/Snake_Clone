#pragma once

#include <iostream>
#include <windows.h>
#include <string>

typedef unsigned short Color;
typedef int KeyID;

constexpr COORD defaultCoord = { 0, 0 };

const Color white = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

namespace Math
{
	template<typename T>
	inline constexpr T Clamp(const T pValue, const T pMin = T(0), const T pMax = T(1))
	{
		return pValue < pMin ? pMin : (pValue > pMax ? pMax : pValue);
		//return (pValue * (pValue <= pMax && pValue >= pMin)) +
		//	(pMin * (pValue < pMin)) + (pMax * (pValue > pMax));
	}
}

namespace Rendering
{
	inline void GetWindowSize(int& pWidth, int& pHeight)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		pWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		pHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}

	class RendBuffer // Doesn't handle memcopies... Im lazy...
	{
	public:
		const UINT32 mMaxSize;
	private:
		int mWidth = 0, mHeight = 0;
		char* mCharBuffer = nullptr;
		Color* mColorBuffer = nullptr;

	public:
		inline int GetWidth() const { return mWidth; }
		inline int GetHeight() const { return mHeight; }

		inline void FillBuffer(const char pChar, const Color pColor = white)
		{
			for (size_t i = 0; i < mMaxSize; i++)
			{
				mCharBuffer[i] = pChar;
				mColorBuffer[i] = pColor;
			}
		}

		inline void ClearBuffers()
		{
			GetWindowSize(mWidth, mHeight);
			FillBuffer(' '); 
		}

		RendBuffer(const UINT32 pMaxSize = 512) :
			mMaxSize(pMaxSize), mCharBuffer(new char[pMaxSize]()), mColorBuffer(new Color[pMaxSize]())
		{ 
			HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

			DWORD dwMode = 0;
			GetConsoleMode(h, &dwMode);
			dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			//dwMode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
			SetConsoleMode(h, dwMode);

			std::cout << "\033[?25l"; // secret command sent to windows... sets position?
			std::cout << "\033[?1049h"; // even more secret... hmmmmmm.....

			mCharBuffer = new char[mMaxSize];
			mColorBuffer = new Color[mMaxSize];

			ClearBuffers();
		}

		inline void DrawChar(const char pChar, const UINT32 pIndex, const Color pColor = white)
		{
			if (pIndex > mMaxSize - 1)
				return;

			mCharBuffer[pIndex] = pChar;
			mColorBuffer[pIndex] = pColor;
		}

		inline void DrawCharCoords(const char pChar, const UINT32 pX, const UINT32 pY, const Color pColor = white)
		{
			if (pX > mWidth - 1 || pX < 0 || pY > mHeight - 1 || pY < 0)
				return;

			DrawChar(pChar, pX + pY * mWidth, pColor);
		}

		inline void DrawString(const std::string pString, UINT32 pX, UINT32 pY, Color pColor = white)
		{
			for (size_t i = 0; i < pString.size(); i++)
				DrawCharCoords(pString[i], pX + i, pY, pColor);
		}

		inline void DrawOutline(const char pChar, const UINT32 pWidth, const UINT32 pHeight, const UINT32 pX, const UINT32 pY, Color pColor = white)
		{
			for (size_t i = 0; i < pWidth; i++)
			{
				DrawCharCoords(pChar, i + pX, pY, pColor);
				DrawCharCoords(pChar, i + pX, pY + pHeight - 1, pColor);
			}
			for (size_t i = 1; i < pHeight - 1; i++)
			{
				DrawCharCoords(pChar, pX, i + pY, pColor);
				DrawCharCoords(pChar, pX + pWidth - 1, i + pY, pColor);
			}
		}

		inline void RenderBuffers(const COORD pCoord = { 0, 0 })
		{
			DWORD written = 0;
			WriteConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), mCharBuffer, mMaxSize, pCoord, &written);
			WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), mColorBuffer, mMaxSize, pCoord, &written);
		}

		~RendBuffer()
		{
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			std::cout << "\033[?1049l";
			std::cout << "\033[?25h";

			delete[](mCharBuffer);
			delete[](mColorBuffer);
		}
	};
}

namespace Collections
{
	template<typename T>
	class DLinkedList
	{
		class Node
		{
			T mItem;
			Node *mNext = nullptr,
				*mPrev = nullptr;
		};

		Node *mFirst = nullptr,
			*mLast = nullptr;

	public:
		Node* PushForward(T pItem)
		{
			Node* newNode = new Node{ pItem };

			newNode->mNext = mFirst;

			if (mFirst) mFirst->mPrev = newNode;
			else mLast = newNode;

			mFirst = newNode;

			return newNode;
		}

		Node* PushBack(T pItem)
		{
			Node* newNode = new Node{ pItem };

			newNode->mPrev = mLast;

			if (mLast) mLast->mNext = newNode;
			else mFirst = newNode;

			mLast = newNode;

			return newNode;
		}
	};
}

namespace Input
{
	inline int GetKey(KeyID pKey) { return GetAsyncKeyState(pKey) & 0x8000; }
}