#pragma once

#include "console_renderer.h"

typedef unsigned int UINT32;

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
}