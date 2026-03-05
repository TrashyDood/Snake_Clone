#pragma once

#include <iostream>
#include <windows.h>

typedef unsigned int UINT32;
typedef unsigned short Color;
typedef int KeyID;

inline constexpr COORD defaultCoord = { 0, 0 };

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

	class RendBuffer // Doesn't handle memcopies. Im lazy...
	{
	public:
		const UINT32 mMaxSize;
		int mWidth = 0, mHeight = 0;

	private:
		int mActiveHandle = 0;
		char* mCharBuffer = nullptr;
		Color* mColorBuffer = nullptr;

	public:
		inline void FillBuffer(char pChar, Color pColor = white)
		{
			for (size_t i = 0; i < mMaxSize; i++)
			{
				mCharBuffer[i] = pChar;
				mColorBuffer[i] = pColor;
			}
		}

		inline void ClearBuffers() { FillBuffer(' '); }

		RendBuffer(UINT32 pMaxSize = 512) :
			mMaxSize(pMaxSize), mCharBuffer(new char[pMaxSize]()), mColorBuffer(new Color[pMaxSize]())
		{ 
			mActiveHandle = 1;
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

		inline void DrawChar(char pChar, unsigned int pIndex, unsigned short pColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
		{
			if (pIndex > mMaxSize - 1)
				return;

			mCharBuffer[pIndex] = pChar;
			mColorBuffer[pIndex] = pColor;
		}

		inline void DrawCharCoords(char pChar, unsigned int pX, unsigned int pY, unsigned short pColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
		{
			DrawChar(pChar, pX + pY * mWidth, pColor);
		}

		inline void DrawOutline(const char pChar, const UINT32 pWidth, const UINT32 pHeight, const UINT32 pX, const UINT32 pY)
		{
			for (size_t i = 0; i < pWidth; i++)
			{
				DrawCharCoords(pChar, i + pX, pY);
				DrawCharCoords(pChar, i + pX, pY + pHeight - 1);
			}
			for (size_t i = 1; i < pHeight - 1; i++)
			{
				DrawCharCoords(pChar, pX, i + pY);
				DrawCharCoords(pChar, pX + pWidth - 1, i + pY);
			}
		}

		inline void RenderBuffers(COORD pCoord = { 0, 0 })
		{

			GetWindowSize(mWidth, mHeight);

			DWORD written = 0;
			WriteConsoleOutputCharacterA(GetStdHandle(STD_OUTPUT_HANDLE), mCharBuffer, mMaxSize, pCoord, &written);
			WriteConsoleOutputAttribute(GetStdHandle(STD_OUTPUT_HANDLE), mColorBuffer, mMaxSize, pCoord, &written);
		}

		~RendBuffer()
		{
			mActiveHandle = 0;
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			std::cout << "\033[?1049l";
			std::cout << "\033[?25h";

			delete[](mCharBuffer);
			delete[](mColorBuffer);
		}
	};

	inline RendBuffer renderBuffer(3600);
}

namespace Input
{
	inline int GetKey(KeyID pKey) { return GetAsyncKeyState(pKey) & 0x8000; }
}