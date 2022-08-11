#pragma once

namespace Vector
{
	struct inty4
	{
		union
		{
			struct
			{
				int x, y, z, w;
			};
			struct
			{
				int r, g, b, a;
			};
			int m[4];
		};

		inty4() = default;
		constexpr inty4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
	};

	struct inty3
	{
		union
		{
			struct
			{
				int x, y, z;
			};
			struct
			{
				int r, g, b;
			};
			int m[3];
		};

		inty3() = default;
		constexpr inty3(int x, int y, int z) : x(x), y(y), z(z) {}
	};

	struct inty2
	{
		union
		{
			struct
			{
				int x, y;
			};
			int m[2];
		};

		inty2() = default;
		constexpr inty2(int x, int y) : x(x), y(y) {}
	};
}