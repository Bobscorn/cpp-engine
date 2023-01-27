#pragma once

#include "Helpers/DebugHelper.h"

#include <utility>
#include <string>
#include <vector>
#include <list>

#ifdef USE_ASSIMP
#include <assimp/types.h>
#endif

#ifdef WIN32
enum DWRITE_FONT_WEIGHT;
enum DWRITE_FONT_STRETCH;
enum DWRITE_FONT_STYLE;
enum DWRITE_PARAGRAPH_ALIGNMENT;
enum DWRITE_TEXT_ALIGNMENT;
#endif

namespace Animation
{
	namespace Interpolation
	{
		enum INTERPOLATE : char;
	}
}

namespace StringHelper
{
	typedef unsigned int UINT;
	typedef int INT;

	bool StartsWith(Stringy const& key, Stringy const& in);

#ifdef WIDE_CHAR
	Stringy CtoW(char const* c);
	std::string WtoC(Stringy const &in);
#endif

#ifdef USE_ASSIMP
	Stringy AtoW(const aiString &in);
#endif
	
	inline bool IsWhiteSpace(wchar_t& c) { return (c == L' ' || c == L'\t' || c == L'\r' || c == L'\n' || c == L'\0'); }
	void Tokenize(Stringy const& data, std::vector<Stringy> &tokens, Stringy const& delimiters, bool trimEmpty);

	bool StoB(Stringy a);

	bool IfFloat(Stringy const &s, float *out);
	bool IfDouble(Stringy const &s, double *out);
	bool IfUINT(Stringy const &s, UINT *out);
	bool IfSizeT(const Stringy& s, size_t* out);
	bool IfINT(Stringy const &s, INT *out);
	bool IfBool(Stringy const &s, bool *out);
#ifdef WIN32
	bool IfFontWeight(Stringy const &s, DWRITE_FONT_WEIGHT &fweight);
	bool IfFontStretch(Stringy const &s, DWRITE_FONT_STRETCH &fstretch);
	bool IfFontStyle(Stringy const &s, DWRITE_FONT_STYLE &fstyle);
	bool IfParagraphAlignment(Stringy const &s, DWRITE_PARAGRAPH_ALIGNMENT &alignment);
	bool IfTextAlignment(Stringy const &s, DWRITE_TEXT_ALIGNMENT &alignment);
#endif
	bool IfInterpolation(Stringy const &s, Animation::Interpolation::INTERPOLATE &inter);

	std::string NumToPlace(unsigned int e);

	std::string ToCPPString(float val);
}

struct StringyList
{
	StringyList() {}
	StringyList(std::string string) : Items(1ul, string) {}
	StringyList(std::list<std::string>&& ass) : Items(ass) {}
	StringyList(const std::list<std::string> &ass) : Items(ass) {}
	StringyList(StringyList&& ass) noexcept : Items(ass.Items) {}
	StringyList(const StringyList &ass) : Items(ass.Items) {}

	std::list<std::string> Items;

	inline StringyList operator+(std::string string)
	{
		std::list<std::string> ass = Items;
		ass.push_back(string);
		return std::move(ass);
	}

	inline StringyList operator+(const StringyList &list)
	{
		std::list<std::string> ass(Items.size() + list.Items.size(), "");
		auto it = ass.begin();
		for (auto i = Items.begin(); i != Items.end(); ++i)
		{
			(*it) = (*i);
			++i;
		}
		for (auto i = list.Items.begin(); i != list.Items.end(); ++i)
		{
			(*it) = (*i);
			++i;
		}
		return ass;
	}
};

struct ListString
{
	ListString(std::string ending = "") : End(ending) {}

	std::list<std::string> Items;
	std::string Separator = ", ";
	std::string EndingSeparator = " and ";
	std::string End = "";

	inline std::string Finalize()
	{
		if (Items.empty())
			return "";

		std::string out;
		if (Items.size() == 1)
			return Items.front() + End;

		auto it = Items.begin();
		auto end = ----Items.end();
		for (; it != end; ++it)
		{
			out += *it + Separator;
		}

		out += (*(----Items.end())) + EndingSeparator + Items.back();

		return out + End;
	}

	inline operator std::string() { return Finalize(); }

	inline ListString& operator+=(std::string stringy)
	{
		Items.push_back(stringy);
		return *this;
	}

	inline ListString& operator=(const StringyList &l)
	{
		Items = l.Items;
		return *this;
	}

	inline ListString& operator=(StringyList &&l)
	{
		Items = std::move(l.Items);
		return *this;
	}

	inline operator bool() const noexcept { return Items.size(); }
};