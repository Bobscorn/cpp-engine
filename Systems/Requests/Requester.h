#pragma once

#include <Helpers/PointerHelper.h>

#include "Requestable.h"

namespace Requests
{
	class Requester
	{
		std::vector<IRequestable*> Requestables;

	public:
		inline void Add(IRequestable *a) { Requestables.push_back(a); a->master = this; }
		inline void Remove(IRequestable *a) 
		{
			for (auto i = Requestables.size(); i-- > 0; ) 
				if (Requestables[i] == a) { std::swap(Requestables[i], Requestables.back()); Requestables.pop_back(); }
		}

		Debug::DebugReturn Request(Requests::Request req);
		Debug::DebugReturn Request(const MultiRequest &&req);
		Debug::DebugReturn Request(MultiRequest req);
	};
}