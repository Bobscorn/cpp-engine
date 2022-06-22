#pragma once

#include <vector>

namespace Drawing
{
	class BindingManager
	{
		static unsigned int _next;
	public:
		static unsigned int GetNext();
	};
}