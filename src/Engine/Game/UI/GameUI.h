#pragma once

#include <memory>

#include "UI/UIPresets/UITextButton.h"
#include "Systems/Requests/Requestable.h"

#include "Drawing/Image.h"

#pragma warning(disable:4250)
namespace UI1I
{
	class UIPosition
	{
		floaty2 m_Anchor = { 0.5f, 0.5f };
		floaty2 m_Pivot = { 0.f, 0.f };
		floaty2 m_HalfBounds = { 50.f, 50.f };
	public:
		UIPosition() = default;
		UIPosition(floaty2 anchor, floaty2 pivot, floaty2 halfBounds) : m_Anchor(anchor), m_Pivot(pivot), m_HalfBounds(halfBounds) {}

		void SetAnchor(floaty2 anchor);
		void SetPivot(floaty2 pivot);
		void SetHalfBounds(floaty2 halfBounds);

		/// <summary>
		/// Transforms a point based on stored Anchor, Pivot and Half Bound values.
		/// The anchor specifies which corner of the parent's space to base the origin off of. The Anchor is normalized -1..+1
		/// The Pivot specifies which corner of the Bounds is used. 
		/// A pivot of 0.f, 0.f does nothing, a pivot of -1, +1 makes the point (HalfWidth, HalfHeight) into (0, Height)
		/// </summary>
		/// <param name="in">The point to transform</param>
		/// <param name="ParentBounds">The pixel space bounds (xmin, ymin, xmax, ymax) of the parent to anchor to</param>
		/// <returns>The transformed point</returns>
		floaty2 Transform(floaty2 in, floaty4 ParentBounds) const;

		/// <summary>
		/// Identical to Transform with a floaty2, treating floaty4 as two floaty2s in the form (x1, y1, x2, y2)
		/// </summary>
		/// <param name="in">The point to transform</param>
		/// <param name="ParentBounds">The pixel space bounds (xmin, ymin, xmax, ymax) of the parent to anchor to</param>
		/// <returns>The 2 transformed points in the form (x1, y1, x2, y2)</returns>
		floaty4 Transform(floaty4 in, floaty4 ParentBounds) const;

		floaty4 GetTransformedBounds(floaty4 parentBounds) const;
	};

	class SizedUIElement : public virtual UI1::UIElement
	{
	protected:
		floaty4 m_ParentBounds = { 0.f, 0.f, 0.f, 0.f };
	public:
		virtual ~SizedUIElement() {}

		inline virtual void SetParentBounds(floaty4 parentBounds) { m_ParentBounds = parentBounds; }
	};

	class SizedUITextBox : public UI1I::UITextBox, public SizedUIElement
	{
		UIPosition m_Position;

	public:
		SizedUITextBox(CommonResources* resources, std::string name, std::string fontName, UIPosition position);

		virtual void IDraw() override;
	};

	struct Buttony : virtual UI1I::UITextBox
	{
		Buttony(Requests::Request action) : action(action) {}

		virtual void IBeforeDraw() override {}
		virtual void IAfterDraw() override {}
		virtual bool OnMouseMove(floaty2 & coords) override { (void)coords; return false; }
		bool OnMouseLeftClick(floaty2 & coords) override;
		virtual bool OnMouseRightClick(floaty2 & coords) override { (void)coords; return false; }
		virtual bool OnWheelClick(floaty2 & coords) override { (void)coords; return false; }
		virtual bool OnMouseScroll(floaty2 & coords, INT scrollage) override { (void)coords; (void)scrollage; return false; }
		virtual bool OnMouseLeftDown(floaty2 & coords) override { (void)coords; return true; }
		virtual bool OnMouseRightDown(floaty2 & coords) override { (void)coords; return true; }
		virtual bool OnWheelDown(floaty2 & coords) override { (void)coords; return false; }
		virtual bool OnMouseLeftUp(floaty2 & coords) override { (void)coords; return true; }
		virtual bool OnMouseRightUp(floaty2 & coords) override { (void)coords; return true; }
		virtual bool OnWheelUp(floaty2 & coords) override { (void)coords; return true; }
		virtual bool OnMouseLeftUnexpectedUp(floaty2 & coords) override { (void)coords; return false; }
		virtual bool OnMouseRightUnexpectedUp(floaty2 & coords) override { (void)coords; return false; }
		virtual bool OnWheelUnexpectedUp(floaty2 & coords) override { (void)coords; return false; }
		void PreComputePosition() override { }
		void ComputePosition(Matrixy2x3 Accumulated) override { (void)Accumulated; }
		void RootPosition() override {}
		void ChildAdded(UIElement *e) override { (void)e; }
		void ChildrenAdded(std::vector<UIElement*> e) override { (void)e; }
		inline bool Within(floaty2 coords) override { return UIPositioner::RectangleInfo::Within(coords); }
		inline Matrixy2x3 GetParentToHereMatrix() override { return UIPositioner::RectangleInfo::GetParentToHereMatrix(); }
		inline Matrixy2x3 GetFullMatrix() override { return UIPositioner::RectangleInfo::GetFullMatrix(); }
		inline Matrixy2x3 GetChildMatrix() override { return UIPositioner::RectangleInfo::GetChildMatrix(); }

		virtual inline Stringy GetName() const override { return Text.GetText() + " Button"; }

		virtual float GetHeight() const = 0;

	private:

		Requests::Request action;
	};

	struct SmallButtony : Buttony, UI1I::SmallButtonE, virtual UI1I::UITextButtonE, virtual UI1I::UITextBox
	{
		SmallButtony(CommonResources *resources, Stringy name, Requests::Request action, Stringy fontname = "Small") : UITextBox(name, fontname), UITextButtonE(name, fontname), FullResourceHolder(resources), Buttony(action) {}
		float GetHeight() const { return mResources->UIConfig->GetSmallFontSize() + mResources->UIConfig->GetSmallMarginSize(); }
		virtual inline Stringy GetName() const override { return UITextBox::Text.GetText() + " Small Button"; }
	};

	struct NormalButtony : Buttony, UI1I::NormalButtonE, virtual UI1I::UITextButtonE, virtual UI1I::UITextBox
	{
		NormalButtony(CommonResources *resources, Stringy name, Requests::Request action, Stringy fontname = "Normal") : UITextBox(name, fontname), UITextButtonE(name, fontname), FullResourceHolder(resources), Buttony(action) {}
		float GetHeight() const override { return mResources->UIConfig->GetNormalFontSize() + mResources->UIConfig->GetNormalMarginSize(); }
		virtual inline Stringy GetName() const override { return UITextBox::Text.GetText() + " Normal Button"; }
	};

	struct BigButtony : Buttony, UI1I::BigButtonE, virtual UI1I::UITextButtonE, virtual UI1I::UITextBox
	{
		BigButtony(CommonResources *resources, Stringy name, Requests::Request action, Stringy fontname = "Big") : UITextBox(name, fontname), UITextButtonE(name, fontname), FullResourceHolder(resources), Buttony(action) {}
		float GetHeight() const override { return mResources->UIConfig->GetBigFontSize() + mResources->UIConfig->GetBigMarginSize(); }
		virtual inline Stringy GetName() const override { return UITextBox::Text.GetText() + " Big Button"; }
	};

	// Simple Buttony Container
	// Displays/Stores them in a vertical line
	// No Scrolling
	struct ButtonyContainer : UI1::BlankElement
	{
		enum Alignment
		{
			LEFT_ALIGN = 0,
			TOP_ALIGN = 0,
			CENTRE_ALIGN = 1,
			RIGHT_ALIGN = 2,
			BOTTOM_ALIGN = 2,
		};

		ButtonyContainer(CommonResources *resources, const std::string& background_img, Alignment x_align = LEFT_ALIGN, Alignment y_align = CENTRE_ALIGN) : FullResourceHolder(resources), Image(resources, background_img), X_Align(x_align), Y_Align(y_align) {}

		Debug::DebugReturn Initialize() override;

		void IBeforeDraw() override;
		void IDraw() override;

		void ComputePosition(Matrixy2x3 accumulated) override;
		virtual inline Stringy GetName() const override { return "InvisibleButtonyContainer Instance"; }

		inline void AddButton(Buttony *button)
		{
			Buttons.push_back(button);
			AddChildTop(button);
			UpdateThatShit();
		}
		inline void AddButtons(std::vector<Buttony*> buttons)
		{
			Buttons.insert(Buttons.end(), buttons.begin(), buttons.end());
			AddChildren<Buttony>(0u, buttons);
			UpdateThatShit();
		}

		inline void UpdateThatShit()
		{
			if (Parent && Parent != Root)
				this->ComputePosition(Parent->GetChildMatrix());
			else
				this->ComputePosition(Matrixy2x3::Identity());
		}

		void UpdateImageScale();
		void UpdateImageTrans();

	protected:

		floaty2 GetAlignmentPosition();

		std::vector<Buttony*> Buttons;

		Drawing::SDLFileImage Image;
		Alignment X_Align;
		Alignment Y_Align;
		Matrixy2x3 ImageTrans;
		float Scale{ 1.f };
		float TransX{ 0.f }, TransY{ 0.f };
		float Marginny{ 30.f };
	};

	struct TitleText : UI1I::UITextBox, UI1::BlankElement
	{
		TitleText(CommonResources *resources, Stringy name) : FullResourceHolder(resources), UITextBox(name, "Title") {}
		Debug::DebugReturn Initialize() override;

		inline void IDraw() override { UI1I::UITextBox::IDraw(); }
		virtual void IDebugDraw() override { UIElement::IDebugDraw(); RectangleInfo::DebugDraw(mResources); }
		virtual void PreComputePosition() override { }
		virtual void ComputePosition(Matrixy2x3 Accumulated) override { this->ComputeWorldMatrix(Accumulated); }
		virtual void RootPosition() override {
			float &HalfWidth = LocalBounds.right;
			this->RecommendBoundingBox({ -HalfWidth, *mResources->HalfWindowHeight - mResources->UIConfig->GetBigMarginSize(), HalfWidth, *mResources->HalfWindowHeight - (mResources->UIConfig->GetBigMarginSize() + 2.f * LocalBounds.top) });
			this->ComputeWorldMatrix(Matrixy2x3::Identity());
		}
		inline bool Within(floaty2 coords) override { return UI1I::UITextBox::Within(coords); }
		inline Matrixy2x3 GetParentToHereMatrix() override { return UIPositioner::RectangleInfo::GetParentToHereMatrix(); }
		inline Matrixy2x3 GetFullMatrix() override { return UIPositioner::RectangleInfo::GetFullMatrix(); }
		inline Matrixy2x3 GetChildMatrix() override { return UIPositioner::RectangleInfo::GetChildMatrix(); }


		float GetMargin() const override { return mResources->UIConfig->GetBigMarginSize(); }
		float GetFontSize() const override { return mResources->UIConfig->GetBigFontSize(); }
		float GetHeight() const { return mResources->UIConfig->GetBigFontSize() + 2.f * mResources->UIConfig->GetBigMarginSize(); }
		
		virtual inline Stringy GetName() const override { return Text.GetText() + " Title Text"; }
		
	protected:
	};
}
#pragma warning(default:4250)
