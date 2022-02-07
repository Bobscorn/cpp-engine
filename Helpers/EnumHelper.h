#pragma once

#include <vector>

namespace Enum
{
	template<class Enum>
	Enum FirstFindEnum(std::wstring& me, const std::vector<std::pair<std::wstring, Enum>>& in, size_t *pos = nullptr)
	{
		Enum return_value = (Enum)0;
		for (auto& entry : in)
		{
			std::wstring entree = entry.first;
			if (StringHelper::StartsWith(entree, me))
			{
				return_value = entry.second;
				if (pos)
					*pos = entry.first.size();
				break;
			}
		}
		return return_value;
	}

	// Uses a vector of pairs to get a key's match
	template<class Key, class Enum>
	Enum FindEnum(Key& me, const std::vector<std::pair<Key, Enum>>& in)
	{
		Enum return_value = 0;
		for (auto& entry : in)
		{
			if (me == entry.first)
			{
				return_value = entry.second;
				break;
			}
		}
		return return_value;
	}

	// Takes a vector of pairs that match 2 enumerations that are used for the same thing, 
	// and converts 1 enum to the other, in case it tickles your OCD to not just use 1 enum
	template<class Enum1, class Enum2>
	Enum2 MatchEnums(std::vector<std::pair<Enum1, Enum2>> matches, Enum1 matchy)
	{
		for (auto& pair : matches)
		{
			if (matchy == pair.first)
				return pair.second;
		}

		// Couldn't find it...... well shit
		return (Enum2)0;
	}
}