#include "TextDrawing.h"
#include "Systems/Input/Config1.h"

void TextDrawing::Update() noexcept
{
	SDLW::Font *font = mResources->UIConfig->GetTextFormat(FontName);	
	if (font && font->GetFont())
	{
		Destroy();

		SimpleSurface text = TTF_RenderUTF8_Blended_Wrapped(font->GetFont(), Text.c_str(), (SDL_Color)Color, (Uint32)*mResources->WindowWidth);

		if (!text)
		{
			DERROR("Creating new Text Texture failed with: " + TTF_GetError());
			return;
		}

		Reset(text.Release());
	}
}

floaty2 CharacterPosition(const SDLW::Font& font, std::string text, size_t index, float linespacing, float wraplength)
{
	if (!font)
		return { 0.f, 0.f };

	if (index >= text.size())
		return { 0.f, 0.f };
	
	size_t newlines = 0;
	size_t lastnewlineindex = (size_t)-1;
	for (size_t i = 0; i < index; ++i)
	{
		if (text[i] == '\n')
		{
			++newlines;
			lastnewlineindex = i;
		}
	}


	std::string lastline;
	if (lastnewlineindex == (size_t)-1)
		lastline = text.substr(0, index + 1);
	else
		lastline = text.substr(lastnewlineindex, index - lastnewlineindex);

	int w, h;
	TTF_SizeUTF8(font.GetFont(), lastline.c_str(), &w, &h);
	
	float yoffset = newlines * ((float)h + linespacing);
	float xoffset = (float)w;

	xoffset = (float)w;

	return { xoffset, yoffset };
}
