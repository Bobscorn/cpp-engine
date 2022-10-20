#include "TestHelper.h"

#include <cmath>

#include <stdexcept>

namespace TmpName
{
	namespace Testing
	{
		bool ApproximatelyEquals(float a, float b, float epsilon)
		{
			return fabs(a - b) < epsilon;
		}

		bool ApproximatelyEquals(double a, double b, double epsilon)
		{
			return fabs(a - b) < epsilon;
		}

		void Assert(bool condition, const char* message)
		{
			if (!condition)
			{
				throw std::runtime_error(message);
			}
		}
	}
}