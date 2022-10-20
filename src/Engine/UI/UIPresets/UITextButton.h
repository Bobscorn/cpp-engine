#pragma once

#include "UI/UI1.h"
#include "UI/UIPositioner.h"
#include "Systems/Events/EventsBase.h"
#include "Systems/Input/Config1.h"
#include "Drawing/TextDrawing.h"

namespace UI1I
{
	/// <summary> Generic Text Button containing all necessary drawing code, and Within override </summary>
	/// <remarks>
	/// Only does drawing, Hit detection <see cref='Within'/> and reacts to font/margin changes
	/// Only uses UIConfig's small margin
	/// Only supports single line text (although it doesn't remove endline \\n characters, their behaviour is undefined.
	/// </remarks>
	struct UITextButton : virtual UI1::UIElement, UIPositioner::RectangleInfo, Events::IEventListenerT<Events::FontChangeEvent, Events::MarginChangeEvent>
	{
		UITextButton(Stringy name, Stringy fontname, ConfigColorTemp textcolor = "Text") : Text(mResources, name, fontname, textcolor) {}

		virtual Debug::DebugReturn Initialize() override;

		virtual inline bool Receive(Events::IEvent *event) override { (void)event; return false; }
		virtual bool Receive(Event::FontSizeEvent *e) override;
		virtual bool Receive(Event::MarginChangeEvent *e) override;

		virtual void IDraw() override;
		virtual void IDebugDraw() override { UIElement::IDebugDraw(); RectangleInfo::DebugDraw(mResources); }
		virtual void PreComputePosition() override { }
		virtual void ComputePosition(Matrixy2x3 Accumulated) override { (void)Accumulated; }
		virtual void RootPosition() override {}
		inline bool Within(floaty2 coords) override { return UIPositioner::RectangleInfo::Within(coords); }
		inline Matrixy2x3 GetParentToHereMatrix() override { return UIPositioner::RectangleInfo::GetParentToHereMatrix(); }
		inline Matrixy2x3 GetFullMatrix() override { return UIPositioner::RectangleInfo::GetFullMatrix(); }
		inline Matrixy2x3 GetChildMatrix() override { return UIPositioner::RectangleInfo::GetChildMatrix(); }

		virtual Stringy GetName() const override { return Text.GetText() + " Generic TextButton"; }
		inline float GetWidth() const { return float(Text.GetWidth()); } // Does not include padding/margin
		inline float GetFullWidth() const { return float(GetWidth() + GetMargin() * 2.f); }

		virtual float GetMargin() const = 0;
		virtual float GetFontSize() const = 0;
		void SetTextFormat(Stringy Name);

		inline void SetTextColor(SDL_Color color) { Text.SetColor(color); }
		inline void SetTextColor(Stringy colorname) { Text.SetColor(colorname); }

	protected:
		TextDrawing Text;
	};

	struct UITextButtonE : virtual UITextButton
	{
		UITextButtonE(Stringy name, Stringy fontname, ConfigColorTemp hovercolor = "HoverText", ConfigColorTemp mousedowncolor = "MouseDownText") : HoveredText(mResources, name, fontname, hovercolor), MouseDownText(mResources, name, fontname, mousedowncolor) {}
		virtual void IDraw() override;

		inline void SetHoverColor(SDL_Color color) { HoveredText.SetColor(color); }
		inline void SetHoverColor(Stringy colorname) { HoveredText.SetColor(colorname); }
		inline void SetMouseDownColor(ConfigColor color) { MouseDownText.SetColor(color); }

		virtual float GetMargin() const = 0;
		virtual float GetFontSize() const = 0;

	protected:
		TextDrawing HoveredText;
		TextDrawing MouseDownText;
	};

	struct NormalButton : virtual UITextButton
	{
		NormalButton() {}
		float GetMargin() const override { return mResources->UIConfig->GetNormalMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetNormalFontSize(); }
	};

	struct SmallButton : virtual UITextButton
	{
		SmallButton() {}
		float GetMargin() const override { return mResources->UIConfig->GetSmallMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetSmallFontSize(); }
	};

	struct BigButton : virtual UITextButton
	{
		BigButton() {}
		float GetMargin() const override { return mResources->UIConfig->GetBigMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetBigFontSize(); }
	};

	struct NormalButtonE : virtual UITextButtonE
	{
		NormalButtonE() {}
		float GetMargin() const override { return mResources->UIConfig->GetNormalMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetNormalFontSize(); }
	};

	struct SmallButtonE : virtual UITextButtonE
	{
		SmallButtonE() {}
		float GetMargin() const override { return mResources->UIConfig->GetSmallMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetSmallFontSize(); }
	};

	struct BigButtonE : virtual UITextButtonE
	{
		BigButtonE() {}
		float GetMargin() const override { return mResources->UIConfig->GetBigMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetBigFontSize(); }
	};
}