#pragma once

#include <random>

namespace Math
{
#define PI_CONST 3.1415926535897982384626433
	constexpr double PiD = 3.1415926535897982384626433;
	constexpr float PiF = 3.1415926535897982384626433f;
	constexpr double HalfPiD = (double)PI_CONST / 2.0;
	constexpr float HalfPiF = (float)PI_CONST / 2.f;
	constexpr double QuatPiD = (double)PI_CONST / 4.0;
	constexpr float QuatPiF = (float)PI_CONST / 4.f;
	constexpr double Pi2D = (double)PI_CONST * 2.0;
	constexpr float Pi2F = (float)PI_CONST * 2.f;
	constexpr double DegToRadD = (double)PI_CONST / 180.0;
	constexpr float DegToRadF = (float)PI_CONST / 180.f;
	constexpr double RadToDegD = 180.0 / (double)PI_CONST;
	constexpr float RadToDegF = 180.f / (float)PI_CONST;

	constexpr float NinetyDegreesRF = 90.f * DegToRadF;
	constexpr double NinetyDegreesRD = 90.0 * DegToRadD;
	constexpr float NinetyDegreesDF = 90.f;
	constexpr double NinetyDegreesDD = 90.0;

	constexpr float HalfNinetySine = 0.70710678118654752440084436210485f;
	constexpr double HalfNinetySineD = 0.70710678118654752440084436210485;

	inline unsigned int NextPow2(unsigned int x)
	{
		x--;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		x++;

		return x;
	}

	template<class T>
	class max
	{
	public:
		inline constexpr T operator()(T a, T b) const noexcept
		{
			return (a > b ? a : b);
		}
	};

	template<class T>
	class min
	{
	public:
		inline constexpr T operator()(T a, T b) const noexcept
		{
			return (a < b ? a : b);
		}
	};

	using maxu = max<unsigned int>;
	using maxi = max<int>;
	using minu = min<unsigned int>;
	using mini = min<int>;

	inline float RandF()
	{
		return ((float)std::rand() / (float)RAND_MAX);
	}

	constexpr inline float Map(float val, float minin, float maxin, float minout, float maxout)
	{
		// Convert value of minin - maxin to proportional value between minout - maxout
		float inrange = maxin - minin;
		float outrange = maxout - minout;
		if (inrange == 0.f || outrange == 0.f)
			return 0.f; // Avoid division by zero

		val = val - minin;
		val = val / inrange;
		val = minout + (val * outrange);
		return val;
	}

	// Map, but output is clamped to the minout-maxout range
	constexpr inline float MapClampped(float val, float minin, float maxin, float minout, float maxout)
	{
		return min<float>()(maxout, max<float>()(minout, Math::Map(val, minin, maxin, minout, maxout)));
	}

	constexpr inline float MinZero(float a, float b)
	{
		return (fabsf(a) < fabsf(b) ? a : b);
	}

	inline float DifAngles(float a, float b)
	{
		float aa = (a - b) + PiF;
		return ((aa - floorf(aa / Pi2F) * Pi2F) - PiF);
	}
}