#pragma once

namespace Math
{
	template<typename T>
	constexpr T Clamp(const T pValue, const T pMin = T(0), const T pMax = T(1))
	{
		return pValue < pMin ? pMin : (pValue > pMax ? pMax : pValue);
		//return (pValue * (pValue <= pMax && pValue >= pMin)) +
		//	(pMin * (pValue < pMin)) + (pMax * (pValue > pMax));
	}
}