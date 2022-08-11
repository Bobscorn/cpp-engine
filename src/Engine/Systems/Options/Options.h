#pragma once

#include <map>
#include <vector>
#include <initializer_list>

namespace Options
{
	typedef unsigned int UINT;

	struct OptionValue
	{
		bool HasValue;
		size_t Value;
	};

	struct OptionID
	{
		explicit OptionID(UINT val) : ID(val) {};
		UINT ID;
	};

	struct Optiony
	{
		UINT ID;
		double Value;
	};

	struct OptionIDLess
	{
		inline bool operator() (const OptionID& lhs, const OptionID& rhs) const
		{
			return lhs.ID < rhs.ID;
		}
	};

	struct Options
	{
		template<class T, size_t ID> friend struct Option;

		std::map<OptionID, double, OptionIDLess> mOptions;

	public:
		Options() {};

		Options(std::vector<Optiony> options)
		{
			for (auto& option : options)
			{
				mOptions[(OptionID)option.ID] = option.Value;
			}
		};

		Options(std::initializer_list<Optiony> options)
		{
			for (auto& option : options)
			{
				mOptions[(OptionID)option.ID] = option.Value;
			}
		}

		Options(std::vector<std::pair<OptionID, size_t>> options);
	};

	template<class T, size_t ID>
	struct Option
	{
		Option(const T& val) : Val(val) {};
		inline operator T() { return Val; }

		// Extract the value of this option from the given Options object,
		// Doesn't change the value if the Options object does not contain one
		template<typename U = T, typename std::enable_if<!std::is_enum<U>::value, std::size_t>::type = 0>
		inline const Options& operator<<(const Options& ops)
		{
			auto thing = ops.mOptions.find((OptionID)ID);
			if (thing != ops.mOptions.end())
				Val = (T)thing->second;

			return ops;
		}

		template<typename U = T, typename std::enable_if<std::is_enum<U>::value, std::size_t>::type = 0>
		inline const Options& operator<<(const Options& ops)
		{
			auto thing = ops.mOptions.find((OptionID)ID);
			if (thing != ops.mOptions.end())
				Val = (T)(size_t)thing->second;

			return ops;
		}

		T Val;
	};
}