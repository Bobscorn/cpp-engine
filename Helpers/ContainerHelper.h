#pragma once

#include <cmath>

namespace Container
{
	// Similar to map, but instead of a series of pairs, its a pair of series
	// Does not guarantee unique keys
	template<class _1, class _2>
	struct VectorPair
	{
		inline void push_back(_1 no1, _2 no2)
		{
			Data.first.push_back(no1);
			Data.second.push_back(no2);
		}

		inline void pop_back()
		{
			Data.first.pop_back();
			Data.second.pop_back();
		}

		inline void insert(size_t index, _1 no1, _2 no2)
		{
			Data.first.insert(Data.first.begin() + index, no1);
			Data.second.insert(Data.second.begin() + index, no2);
		}

		inline void resize(size_t size)
		{
			Data.first.resize(size);
			Data.second.resize(size);
		}

		inline void reserve(size_t size)
		{
			Data.first.reserve(size);
			Data.second.reserve(size);
		}

		inline std::pair<_1, _2> at(size_t index)
		{
			if (index >= Data.first.size())
			{
				DERROR(L"out of range index");
				return std::pair<_1, _2>();
			}
			return std::make_pair(Data.first[index], Data.second[index]);
		}

		inline _1& first_at(size_t index)
		{
			if (index >= Data.first.size())
			{
				DERROR(L"out of range index");
				_DEBUG_ERROR("index out of range");
			}
			return Data.first[index];
		}

		inline _2& second_at(size_t index)
		{
			if (index >= Data.second.size())
			{
				DERROR(L"out of range index");
				_DEBUG_ERROR("index out of range");
			}
			return Data.second[index];
		}

		inline std::vector<_1> *First()
		{
			return &Data.first;
		}

		inline std::vector<_2> *Second()
		{
			return &Data.second;
		}

		inline size_t IndexOf(_1 thing)
		{
			for (size_t i = Data.first.size(); i-- > 0; )
				if (Data.first[i] == thing)
					return i;

			DERROR(L"could not find thing");
			return 0u;
		}

		inline bool Exists(_1 key)
		{
			bool found = false;
			for (auto& entry : Data.first)
			{
				if (entry == key)
				{
					found = true;
					break;
				}
			}
			return found;
		}

		inline _2& Find(_1 key)
		{
			for (UINT i = Data.first.size(); i-- > 0; )
			{
				if (Data.first[i] == key)
					return Data.second[i];
			}
			_DEBUG_ERROR("key not found");
			return _2();
		}

		inline _1& FindKey(_2 data) const
		{
			for (UINT i = Data.second.size(); i-- > 0; )
			{
				if (Data.second[i] == data)
					return Data.first[i];
			}
			_DEBUG_ERROR(L"data not found");
			return _1();
		}

		inline _2& operator[](_1 key)
		{
			for (size_t i = Data.first.size(); i-- > 0; )
				if (Data.first[i] == key)
					return Data.second[i];

			// Couldn't find it so create entry and return that
			push_back(key, _2());
			return Data.second.back();
		}

		inline bool operator==(const VectorPair<_1, _2> &other)
		{
			bool same = true;

			if (same = (Data.first.size() == other.Data.first.size()))
			{
				for (UINT i = Data.first.size(); i-- > 0 && same; )
				{
					same |= (Data.first[i] == other.Data.first[i] && Data.second[i] == other.Data.second[i]);
				}
			}

			return same;
		}

		inline bool empty() { return !(bool)Data.first.size(); }

		inline size_t size() { return Data.first.size(); }
	private:
		std::pair<std::vector<_1>, std::vector<_2>> Data;
	};

	template<class Data>
	using NamedPair = VectorPair<std::wstring, Data>;

	// ^
	// VectorPair Class
	// Modulus Class (Used by Range class)
	// v

	template<class T>
	struct Mod
	{
		T operator() (const T &left, const T &right) const
		{
			return left % right;
		}
	};

	template<>
	struct Mod<float>
	{
		float operator() (const float &left, const float &right) const
		{
			float ass = right;
			return modff(left, &ass);
		}
	};

	template<>
	struct Mod<double>
	{
		double operator() (const double &left, const double &right) const
		{
			double ass = right;
			return modf(left, &ass);
		}
	};

	template<>
	struct Mod<long double>
	{
		long double operator() (const long double &left, const long double &right) const
		{
			long double ass = right;
			return modfl(left, &ass);
		}
	};


	// ^
	// Modulus Class

	// Range Class
	// v

	enum OutOfRangeBehaviour
	{
		OUT_OF_RANGE_DEFAULT = 0,	// Output m_Data::Default (might have to be user defined)
		OUT_OF_RANGE_FIRST = 1,	// Output First Frame
		OUT_OF_RANGE_LAST = 2,	// Output Last Frame
		OUT_OF_RANGE_EXTRAPOLATE = 3,	// Extrapolate using the nearest 2 frames
		OUT_OF_RANGE_REPEAT = 4,	// Output Modulus Input
	};

	template<class Key, class Data, class Modulus = Mod<Key>, class Interpolator = ::Animation::Interpolation::LinearInterpolator>
	struct Range
	{
		Range(Key *keys, Data *data, UINT size, Data Default = Data(), OutOfRangeBehaviour before = OUT_OF_RANGE_DEFAULT, OutOfRangeBehaviour after = OUT_OF_RANGE_LAST) :
			Default(Default), BeforeRangeBehaviour(before), AfterRangeBehaviour(after)
		{
			Values.reserve(size);
			for (UINT i = count; i-- > 0; )
			{
				Insert(std::make_pair(keys[i], data[i]));
			}
		}

		Range(std::pair<Key, Data> *vals, UINT size, Data Default = Data(), OutOfRangeBehaviour before = OUT_OF_RANGE_DEFAULT, OutOfRangeBehaviour after = OUT_OF_RANGE_LAST) :
			Default(Default), BeforeRangeBehaviour(before), AfterRangeBehaviour(after)
		{
			Values.reserve(size);
			for (UINT i = size; i-- > 0; )
			{
				Insert(vals[i]);
			}
		}

		Range(std::vector<std::pair<Key, Data>> vals, Data Default = Data(), OutOfRangeBehaviour before = OUT_OF_RANGE_DEFAULT, OutOfRangeBehaviour after = OUT_OF_RANGE_LAST) : Range(vals.data(), (UINT)vals.size(), Default, before, after) {};

	protected:
		std::vector<std::pair<Key, Data>> Values; // Unknown length array
		OutOfRangeBehaviour BeforeRangeBehaviour;
		OutOfRangeBehaviour AfterRangeBehaviour;

		Data Default;
		Modulus m_Mod;
		Interpolator m_Inter;
	public:
		inline bool Empty()
		{
			return Values.empty();
		}

		inline size_t Size()
		{
			return Values.size();
		}

		inline void Insert(std::pair<Key, Data> value)
		{
			if (Values.size())
			{
				if (Values.back().first < value.first)
				{
					Values.push_back(value);
					return;
				}
				for (UINT i = 0; i < Values.size(); ++i)
				{
					if (Values[i].first > value.first)
					{
						Values.insert(Values.begin() + i, value);
						return;
					}
				}
			}
			else
			{
				Values.push_back(value);
			}
		}

		inline Key Lower() const
		{
			return Values.front().first;
		}

		inline Key Upper() const
		{
			return Values.back().first;
		}

		inline Data Get(Key at)
		{
			// Modulus-ify the value with the highest value
			if (AfterRangeBehaviour == OUT_OF_RANGE_REPEAT)
			{
				Key RangeLow = Values.front().first;
				Key RangeHigh = Values.back().first;
				at = RangeLow + m_Mod((at - RangeLow), (RangeHigh - RangeLow));
			}

			// Find the closest smaller entry
			// Use 1 higher than the size as a index that will never be found
			size_t didnotfind = Values.size() + 1;

			size_t index = didnotfind;

			for (size_t i = Values.size(); i-- > 0; )
			{
				if (Values[i].first < at)
				{
					index = i;
					break;
				}
			}

			// Means the key given is smaller than all of Values
			// Meaning given key is 'BeforeRange'
			if (index == didnotfind)
			{
				switch (BeforeRangeBehaviour)
				{
				case OUT_OF_RANGE_EXTRAPOLATE:
				{
					Data &Val1 = Values[0].second;
					Data &Val2 = Values[1].second;

					Key &Key1 = Values[0].first;
					Key &Key2 = Values[1].first;

					double extrapolate = 1.0 + (double)(Key1 - at) / (double)(Key2 - Key1);
					return m_Inter(Val1, Val2, extrapolate); // Multiple 1D vector pointing from Val2 to Val1 by the extrapolation (which *will* be above 1)
				}
				case OUT_OF_RANGE_LAST:
					return Values.back().second;

				case OUT_OF_RANGE_FIRST:
					return Values.front().second;

				case OUT_OF_RANGE_REPEAT: // This is here because I'm not sure how exactly you can repeat 'at' when it is smaller than all entries
				case OUT_OF_RANGE_DEFAULT:
				default:
				{
					return Default; // Value given in template, or just defaulted to m_Data()
				}
				}
			}


			// Means the key given was larger than the largest entered key,
			// Meaning given key is 'AfterRange'
			if (index == Values.size() - 1)
			{
				switch (AfterRangeBehaviour)
				{
				case OUT_OF_RANGE_EXTRAPOLATE:
				{
					Data &Val1 = Values[Values.size() - 2].second; // 2nd to Last Element
					Data &Val2 = Values[Values.size() - 1].second; // Last Element

					Key &Key1 = Values[Values.size() - 2].first;
					Key &Key2 = Values[Values.size() - 1].first;

					double extrapolate = (double)(at - Key1) / (double)(Key2 - Key1); // 1.0 + Difference of at and Key1 divided by difference of Key2 and Key1
					return m_Inter(Val1, Val2, extrapolate);
				}
				case OUT_OF_RANGE_LAST:
					return Values.back().second;

				case OUT_OF_RANGE_FIRST:
					return Values.front().second;

				case OUT_OF_RANGE_REPEAT: // This is here because I'm not sure how exactly you can repeat 'at' when it is smaller than all entries
				case OUT_OF_RANGE_DEFAULT:
				default:
				{
					return Default; // Value given in template, or just defaulted to m_Data()
				}
				}
			}


			// Key is within range

			std::pair<Key, Data> &Pair1 = Values[index];
			std::pair<Key, Data> &Pair2 = Values[index + 1];

			//DINFO(L"Using frames: " + std::to_wstring(index) + L" and " + std::to_wstring(index + 1));
			//DINFO(L"Frame 1: " + Debug::VectorString(Pair1.second) + L" and Frame 2: " + Debug::VectorString(Pair2.second));

			Key key = at - Pair1.first;
			double factor = key / (Pair2.first - Pair1.first);
			/*double interpolated = m_Inter(factor);
			m_Data diff = Pair2.second - Pair1.second;
			return Pair1.second + interpolated * diff;*/
			Data Interpolation = m_Inter(Pair1.second, Pair2.second, factor);
			//DINFO(L"Thing " + Random::CtoW(typeid(m_Data).name()));
			return Interpolation;
		}
	};

	/*template<class Key, class m_Data>
	class IOPFAC
	{
		std::vector<Key> InsertionOrder;
		std::unordered_map<Key, m_Data> FastAccess;

	public:

	};*/
}