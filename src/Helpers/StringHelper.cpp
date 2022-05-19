//#include "Thing.h"
#include "StringHelper.h"

#include "../Systems/Animation/Animation.h"

#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <cstdlib>
#ifdef WIN32
#include <dwrite_3.h>
#endif

bool StringHelper::StartsWith(Stringy const & key, Stringy const & in)
{
	return in.find(key) == 0;
}

#ifdef WIDE_CHARS
Stringy StringHelper::CtoW(char const * c)
{
	const size_t cSize = strlen(c) + 1;
	Stringy wc = Stringy(cSize, L'$');
	mbstowcs_s(nullptr, &wc[0], cSize, c, cSize);

	// Trim trailing null character
	wc.pop_back();

	return wc;
}

std::string StringHelper::WtoC(Stringy const &in)
{
	std::string out = std::string(in.size(), '$');
	wcstombs_s(nullptr, &out[0], out.size() + 1, in.c_str(), _TRUNCATE);
	return out;
}
#endif

#ifdef USE_ASSIMP
Stringy StringHelper::AtoW(const aiString & in)
{
	return CtoW(in.C_Str());
}
#endif

void StringHelper::Tokenize(Stringy const & str, std::vector<Stringy>& tokens, Stringy const & delimiters, bool trimEmpty)
{
	Stringy::size_type pos, lastPos = 0, length = str.length();

	using value_type = std::vector<Stringy>::value_type;
	using _size_type = std::vector<Stringy>::size_type;

	while (lastPos < length + 1)
	{
		pos = str.find_first_of(delimiters, lastPos);
	if (pos == Stringy::npos)
		{
			pos = length;
		}

	if (pos != lastPos || !trimEmpty)
			tokens.push_back(value_type(str.data() + lastPos,
			(_size_type)pos - lastPos));

		lastPos = pos + 1;
	}
}

bool StringHelper::StoB(Stringy a)
{
	return a == "true" || a == "true" || std::stol(a) > 0;
}

bool StringHelper::IfFloat(Stringy const &s, float * out)
{
	auto c_str = s.c_str();
	char* p = 0;
	float val = std::strtof(c_str, &p);
	if (out)
		*out = val;
	return p != c_str;
}

bool StringHelper::IfDouble(Stringy const & s, double * out)
{
	auto c_str = s.c_str();
	char* p = 0;
	double val = std::strtod(c_str, &p);
	if (out && p != c_str)
		*out = val;
	return p != c_str;
}

bool StringHelper::IfUINT(Stringy const &s, UINT *out)
{
	auto c_str = s.c_str();
	char* p = 0;
	UINT val = (UINT)std::strtoul(c_str, &p, 10);
	if (out && p != c_str)
		*out = val;
	return p != c_str;
}

bool StringHelper::IfINT(Stringy const &s, INT * out)
{
	auto c_str = s.c_str();
	char* p = 0;
	int val = (int)std::strtol(c_str, &p, 10);
	if (out && p != c_str)
		*out = val;
	return p != c_str;
}

bool StringHelper::IfSizeT(Stringy const& s, size_t* out)
{
	auto c_str = s.c_str();
	char* p = 0;
	size_t val = (size_t)std::strtoull(c_str, &p, 10);
	if (out && p != c_str)
		*out = val;
	return p != c_str;
}

bool StringHelper::IfBool(Stringy const &s, bool * out)
{
	if (s == "0" || s == "false")
	{
		*out = false;
		return true;
	}
	else if (s == "1" || s == "true")
	{
		*out = true;
		return true;
	}
	return false;
}

#ifdef WIN32
bool StringHelper::IfFontWeight(Stringy const &s, DWRITE_FONT_WEIGHT &fweight)
{
	Stringy tmp(s.size(), L'$');
	if (s == "thin")
	{
		fweight = DWRITE_FONT_WEIGHT_THIN;
		return true;
	}
	else if (s == "extralight" || s == "ultralight")
	{
		fweight = DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
		return true;
	}
	else if (s == "light")
	{
		fweight = DWRITE_FONT_WEIGHT_LIGHT;
		return true;
	}
	else if (s == "semilight")
	{
		fweight = DWRITE_FONT_WEIGHT_SEMI_LIGHT;
		return true;
	}
	else if (s == "normal" || s == "regular")
	{
		fweight = DWRITE_FONT_WEIGHT_NORMAL;
		return true;
	}
	else if (s == "medium")
	{
		fweight = DWRITE_FONT_WEIGHT_MEDIUM;
		return true;
	}
	else if (s == "demibold" || s == "semibold")
	{
		fweight = DWRITE_FONT_WEIGHT_SEMI_BOLD;
		return true;
	}
	else if (s == "bold")
	{
		fweight = DWRITE_FONT_WEIGHT_BOLD;
		return true;
	}
	else if (s == "extrabold" || s == "ultrabold")
	{
		fweight = DWRITE_FONT_WEIGHT_EXTRA_BOLD;
		return true;
	}
	else if (s == "black" || s == "Heavy")
	{
		fweight = DWRITE_FONT_WEIGHT_BLACK;
		return true;
	}
	else if (s == "extrablack" || s == "extrabold")
	{
		fweight = DWRITE_FONT_WEIGHT_EXTRA_BLACK;
		return true;
	}
	return false;
}

bool StringHelper::IfFontStretch(Stringy const &s, DWRITE_FONT_STRETCH & fstretch)
{
	Stringy tmp(s.size(), L'$');
	if (s == "undefined")
	{
		fstretch = DWRITE_FONT_STRETCH_UNDEFINED;
		return true;
	}
	else if (s == "ultracondensed")
	{
		fstretch = DWRITE_FONT_STRETCH_ULTRA_CONDENSED;
		return true;
	}
	else if (s == "extracondensed")
	{
		fstretch = DWRITE_FONT_STRETCH_EXTRA_CONDENSED;
		return true;
	}
	else if (s == "condensed")
	{
		fstretch = DWRITE_FONT_STRETCH_CONDENSED;
		return true;
	}
	else if (s == "semicondensed")
	{
		fstretch = DWRITE_FONT_STRETCH_SEMI_CONDENSED;
		return true;
	}
	else if (s == "normal")
	{
		fstretch = DWRITE_FONT_STRETCH_NORMAL;
		return true;
	}
	else if (s == "medium")
	{
		fstretch = DWRITE_FONT_STRETCH_MEDIUM;
		return true;
	}
	else if (s == "semiexpanded")
	{
		fstretch = DWRITE_FONT_STRETCH_SEMI_EXPANDED;
		return true;
	}
	else if (s == "expanded")
	{
		fstretch = DWRITE_FONT_STRETCH_EXPANDED;
		return true;
	}
	else if (s == "extraexpanded")
	{
		fstretch = DWRITE_FONT_STRETCH_EXTRA_EXPANDED;
		return true;
	}
	else if (s == "ultraexpanded")
	{
		fstretch = DWRITE_FONT_STRETCH_ULTRA_EXPANDED;
		return true;
	}
	return false;
}

bool StringHelper::IfFontStyle(Stringy const &s, DWRITE_FONT_STYLE & fstyle)
{
	Stringy tmp(s.size(), L'$');
	if (s == "normal" || s == "regular")
	{
		fstyle = DWRITE_FONT_STYLE_NORMAL;
		return true;
	}
	else if (s == "oblique")
	{
		fstyle = DWRITE_FONT_STYLE_OBLIQUE;
		return true;
	}
	else if (s == "italic")
	{
		fstyle = DWRITE_FONT_STYLE_ITALIC;
		return true;
	}
	return false;
}

bool StringHelper::IfParagraphAlignment(Stringy const &s, DWRITE_PARAGRAPH_ALIGNMENT & alignment)
{
	Stringy tmp(s.size(), L'$');
	if (s == "near")
	{
		alignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
		return true;
	}
	else if (s == "far")
	{
		alignment = DWRITE_PARAGRAPH_ALIGNMENT_FAR;
		return true;
	}
	else if (s == "center")
	{
		alignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
		return true;
	}
	return false;
}

bool StringHelper::IfTextAlignment(Stringy const &s, DWRITE_TEXT_ALIGNMENT & alignment)
{
	Stringy tmp(s.size(), L'$');
	if (s == "leading")
	{
		alignment = DWRITE_TEXT_ALIGNMENT_LEADING;
		return true;
	}
	else if (s == "trailing")
	{
		alignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
		return true;
	}
	else if (s == "center" || s == "centre")
	{
		alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
		return true;
	}
	else if (s == "justified")
	{
		alignment = DWRITE_TEXT_ALIGNMENT_JUSTIFIED;
		return true;
	}
	return false;
}
std::string StringHelper::NumToPlace(unsigned int e)
{
	std::string num = std::to_string(e);

	switch (e % 10)
	{
	case 1:
		if ((e % 100) == 11)
			return num + "th"; // ends with 11, 11th, not 11st
		return num + "st"; // 1st, 21st, 101st
	case 2:
		if ((e % 100) == 12)
			return num + "th"; // ends with 12, 12th not 12nd
		return num + "nd"; // 2nd, 42nd, 2012
	case 3:
		if ((e % 100) == 13)
			return num + "rd"; // ends with 13, 13th not 13rd
		return num + "rd";
	default:
		return num + "th";
	}
}

std::string StringHelper::ToCPPString(float val)
{
	std::stringstream ass;
	ass << val;
	std::string ass2 = ass.str();
	if (std::any_of(ass2.begin(), ass2.end(), [](char i) -> bool { return i == '.'; }))
		ass << "f";
	else
		ass << ".f";
	return ass.str();
}
#endif

bool StringHelper::IfInterpolation(Stringy const & s, Animation::Interpolation::INTERPOLATE & inter)
{
	Stringy tmp(s.size(), L'$');
	std::transform(s.begin(), s.end(), tmp.begin(), ::tolower);
	if (s == "linear")
	{
		inter = Animation::Interpolation::INTERPOLATE_LINEAR;
		return true;
	}
	else if (s == "smoothinout")
	{
		inter = Animation::Interpolation::INTERPOLATE_SMOOTH_IN_OUT;
		return true;
	}
	else if (s == "fastout" || s == "smoothin" || s == "smoothinfastout")
	{
		inter = Animation::Interpolation::INTERPOLATE_SMOOTH_IN_FAST_OUT;
		return true;
	}
	else if (s == "fastin" || s == "smoothout" || s == "smoothoutfastin")
	{
		inter = Animation::Interpolation::INTERPOLATE_FAST_IN_SMOOTH_OUT;
		return true;
	}
	return false;
}