#pragma once

#include "Helpers/DebugHelper.h"

#include "../Events/Events.h"
#include "Importer.h"
#include "Systems/Execution/ResourceHolder.h"
#include "Systems/Requests/Requestable.h"
#include "Systems/SDLWrapping/SDLWrappers.h"

#ifdef __linux__
#include <SDL2/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

namespace Config1
{
	struct UIConfig;

	const Stringy ConfigPrefix = "Config/";
}


namespace Event
{
	struct FontSizeEvent : Events::IEvent
	{
		constexpr static Events::Event MyType = Events::FontChangeEvent;
		constexpr FontSizeEvent(float smallfont, float normal, float big, unsigned int frameid = 0) : IEvent(MyType, frameid), SmallFontSize(smallfont), NormalFontSize(normal), BigFontSize(big) {}
		float SmallFontSize, NormalFontSize, BigFontSize;
		virtual bool operator==(Events::IEvent *event) override;
	};

	struct MarginChangeEvent : Events::IEvent
	{
		constexpr static Events::Event MyType = Events::MarginChangeEvent;
		constexpr MarginChangeEvent(float smallmargin, float normal, float big, unsigned int frameid = 0) : IEvent(MyType, frameid), SmallMargin(smallmargin), NormalMargin(normal), BigMargin(big) {}
		float SmallMargin, NormalMargin, BigMargin;
		virtual bool operator==(Events::IEvent * event) override;
	};
}

namespace Config1
{

	struct UIConfig : Events::IEventListenerT<Events::ResizePreEvent, Events::ReleaseGraphicsEvent, Events::CreateGraphicsEvent>, FullResourceHolder
	{
	private:
	public:
		const static Stringy DefaultUIConfigFile;

		UIConfig(Events::EventManager *e, float dpi = 1.f, Stringy Filename = DefaultUIConfigFile);

		void Reset(Stringy Filename);
		void Reset(std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>> &xmlbits);

		bool Receive(Events::IEvent *event) override;
		
		// Adds DPI Itself, do not apply api before setting font size
		inline void SetScale(float newscale) 
		{
			float ScaleChange = 1.f;
			if (newscale != 0.f)
				ScaleChange = newscale / FontScale;
			FontScale = newscale; 
			DoFontSizes(); 
			ResizeFonts(ScaleChange);
			NotifyFontSizeChange(); 
		}
		inline void ScaleFontsBy(float scale) { FontScale *= scale; DoFontSizes(); ResizeFonts(scale); NotifyFontSizeChange(); }
		

		inline float GetSmallFontSize() const { return FinalSmallSize; }
		inline float GetNormalFontSize() const { return FinalNormalSize; }
		inline float GetBigFontSize() const { return FinalBigSize; }
		inline float GetTooltipFontSize() const { return FinalTooltipSize; }
		inline float GetDipsPerScroll() const { return DipsPerScroll; }
		inline float GetSmallMarginSize() const { return SmallMargin * MarginScale; }
		inline float GetNormalMarginSize() const { return NormalMargin * MarginScale; }
		inline float GetBigMarginSize() const { return BigMargin * MarginScale; }
		
		SDLW::Font *RegisterNewTextFormat(const Stringy& Name, SDLW::FontDesc fd);
		SDLW::Font *GetTextFormat(const Stringy& Name);

		SDL_Color RegisterNewBrush(const Stringy& Name, SDL_Color color);
		SDL_Color GetBrush(const Stringy& Name);

	private:
		void ResizeFonts(float scalechange = 0.f);

		inline void DoFontSizes()
		{
			float DpiScale = 1.f;
			if (mResources)
				DpiScale = (float)(*mResources->DpiScale);

			FinalSmallSize		= FontScale * SmallFontSize		* DpiScale;
			FinalNormalSize		= FontScale * NormalFontSize	* DpiScale;
			FinalBigSize		= FontScale * BigFontSize		* DpiScale;
			FinalTooltipSize	= FontScale * TooltipFontSize	* DpiScale;
		}

		inline void NotifyFontSizeChange()
		{
			if (mResources)
			{
				::Event::FontSizeEvent ass(FinalSmallSize, FinalNormalSize, FinalBigSize, *mResources->UpdateID);
				mResources->Event->Send(&ass);
			}
		}

		float FontScale = 1.f;
		float BigFontSize = 34.f;
		float NormalFontSize = 26.f;
		float SmallFontSize = 12.f;
		float FinalBigSize = 0.f;
		float FinalNormalSize = 0.f;
		float FinalSmallSize = 0.f;
		float TooltipFontSize = 12.f;
		float FinalTooltipSize = 0.f;

		float DipsPerScroll = 24.f;

		float SmallMargin = 3.f;
		float NormalMargin = 7.f;
		float BigMargin = 12.f;
		float MarginScale = 1.f;

		std::unordered_map<Stringy, SDLW::Font> RegisteredTextFormats;
		std::unordered_map<Stringy, SDL_Color> RegisteredBrushs;
	};
}