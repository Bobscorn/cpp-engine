#pragma once

#include "UI/UI1.h"
#include "UI/UIPositioner.h"

#include "Config/Config.h"

#include "Drawing/TextDrawing.h"

#include "Systems/Input/InputAttachment.h"

namespace TextInput
{
	enum TextInputType
	{
		Anything = 0,
		NumbersOnly = 1,
		AZOnly = 2,
	};

	struct BaseInputThings
	{
		std::string FontName;
		std::string Content;
		TextInputType InputType;
		floaty4 Dimensions;
	};

	// Base class used to capture text input, cursor location, modify text input, render text input, etc.
	struct BaseInput : UI1::UIElement, InputAttach::IAttachable
	{
		BaseInput(BaseInputThings things);
		
		//virtual bool OnMouseLeftDown(floaty2 &coords) override;

		//virtual bool OnFocusLost(FocusLoss reason, UIElement *to) override;

	protected:

		void TakeFocus();
		void LoseFocus();
		void Paste(std::string::iterator before);
		void Copy(std::string::iterator start, std::string::iterator end);
		void Cut(std::string::iterator start, std::string::iterator end);
		std::string::iterator GetPrecedingIterator(floaty2 from);
		std::string::iterator GetFollowingIterator(floaty2 from);

		BaseInputThings m_Things;
		size_t m_CursorLocation;
	};
}

namespace Options
{
	enum MenuDisplayMode
	{
		Line = 0, // Display panes in a long line (1x? or ?x1 dimension, with scrolling as necessary)
		Tiled = 2, // Display panes in a tiled manner, generally I can't be screwed making this work too well (basically it would be a dynamic tiled menu attempting to use all screen space)
	};

	// Direction to put excess pane's in
	enum OverflowBehaviour
	{
		Vertical = 0,
		Horizontal = 1,
	};

	struct OptionThings
	{
		std::string Name;
		Config::Key Key;
	};

	// A modifiable Option interface
	// Generally expected to modify an option
	struct Option : virtual UI1::UIElement, virtual UIPositioner::IPositionInfo
	{
		Option(OptionThings tings) : m_Info(tings) {}
		virtual ~Option() {}

		virtual floaty2 GetMinDimensions() const = 0; // Get Minimum and Maximum Dimensions as { minwidth, minheight, maxwidth, maxheight }
		virtual floaty2 GetIdealDimensions() const = 0; // Ideal
		virtual void SetDimensions(floaty2 dim) = 0;
		virtual floaty2 GetDimensions() const = 0; // Get Current Dimensions
		
		inline std::string GetName() const { return m_Info.Name; }

	protected:
		OptionThings m_Info;
	};


	struct SliderThings
	{
		enum DisplayType
		{
			None = 0,
			Arrows = 1,
			Value = 2,
		};
		unsigned int Display; // Display Mask of DisplayTypes
		float MinVal = 0.f; // Slider Minimum Value
		float MaxVal = 1.f; // Slider Maximum Value
		float Increment = 0.1f; // Arrow Increment Amount
		bool RealtimeAdjustment = false;
	};

	// A numeric Option with a draggable slider
	struct SliderOption : Option, UIPositioner::RectangleInfo, UI1::BlankElement
	{
		SliderOption(CommonResources *resources, OptionThings ting1, SliderThings ting2);

		virtual void IDraw() override;

		virtual bool OnMouseLeftDown(floaty2 &coords) override;
		virtual bool OnMouseLeftUp(floaty2 &coords) override;

		virtual floaty2 GetMinDimensions() const override;
		virtual floaty2 GetIdealDimensions() const override;
		virtual void SetDimensions(floaty2 dim) override;
		virtual floaty2 GetDimensions() const override;

		virtual void IDebugDraw() override;

	protected:

		void UpdateValue();

		TextDrawing m_Name;
		TextDrawing m_Value;
		SliderThings m_Info;

		struct DisplayInfo
		{
			float Margin;
			float Border;
			float ArrowSize;
			float NameValueGap;
			float SliderDistance;
		};

		constexpr static float ValueReservedSpace = 50.f;
		constexpr static DisplayInfo Recommended = { 3.f, 2.f, 12.5f, 15.f, 50.f };
		constexpr static DisplayInfo Minimum = { 2.f, 1.f, 10.f, 0.f, 30.f };

		DisplayInfo m_Active = Recommended;
		float m_SliderPosition = 0.f; // Normalized [0,...,1] position between Minimum and Maximum Slider values
	};
	
	struct ToggleThings
	{
		enum DisplayType
		{
			HighlightSingle = 0, // An option that only displays the first value, and highlights if its on
			HighlightDouble = 1, // Displays both values, highlighting whichever represents the current value (eg values of "Multisampling On/Multisampling Off" true highlights On, false highlights Off)
			SingleSlider = 2, // Displays first value, and a slider next to it
		};
	};

	// True/False option
	struct ToggleOption : Option
	{

	};

	// Option with preset possible values to pick from (with drop down list)
	struct ListOption : Option
	{

	};

	// Option with left and right arrows with keyboard input
	struct ValueOption : Option
	{

	};

	struct OptionPaneThings
	{
		floaty2 BorderThickness;
	};

	// A visible container that directly holds Options
	// What you could consider an Option Group
	// Something like the Video or Audio tab of most option menus
	struct OptionPane : UI1::BlankElement
	{
		OptionPane(OptionPaneThings ting, CommonResources *resources);
		OptionPane(OptionPaneThings ting, CommonResources *resources, std::vector<std::unique_ptr<Option>> options);

		template<class T, class ... Args>
		inline T *AddOption(Args ... args)
		{
			std::unique_ptr<T> ptr = std::make_unique<T>(std::forward<Args>(args)...);
			auto *out = ptr.get();
			AddOption(std::move(ptr));
			return out;
		}

		void AddOption(std::unique_ptr<Option> option);

		floaty2 GetDimensions() const;

		void UpdateShit(); // Updates Dimensions

		void SetMatrix(Matrixy2x3 worldtohere);

		virtual std::string GetName() const override;

		inline virtual Matrixy2x3 GetFullMatrix() override { Matrixy2x3 temp = m_WorldToHere; if (!temp.Invert()) temp = Matrixy2x3::Identity(); return temp; }

	protected:
		OptionPaneThings m_Info;
		Matrixy2x3 m_WorldToHere; 
		floaty2 m_InteriorDimensions; // Technically in parent space
		float m_IdealOptionWidth = 0.f;

		std::vector<std::unique_ptr<Option>> m_Options;
	};

	struct OptionMenuThings
	{
		std::string Name;
		MenuDisplayMode Display;
		OverflowBehaviour Overflow;
	};

	// A (usually) invisible container that holds OptionPanes (which hold Options)
	// Generally this is given a preset to organise, hide, scroll etc. the panes with
	// OptionMenus expect the parent to align this object as intended, for Root as parent, center alignment is used
	// OptionMenus must have a bounding box larger or identical to m_MinimumBoundingBox to fit the internal OptionPanes
	struct OptionMenu : UI1::BlankElement, UIPositioner::IPositionInfo
	{
		OptionMenu(OptionMenuThings ting, CommonResources *resources);
		OptionMenu(OptionMenuThings tings, CommonResources *resources, std::vector<std::unique_ptr<OptionPane>> panes);


		void AddOptionPane(OptionPane *pane);
		OptionPane *EmplaceOptionPane(OptionPaneThings things);

		virtual void ComputePosition(Matrixy2x3 Accumulated) override;
		virtual void RootPosition() override;

		virtual bool Within(floaty2 point) override;
		virtual void MatrixChanged() override;
		virtual Recty GetBoundingBox() override;
		virtual void RecommendBoundingBox(Recty rect) override;

		inline virtual std::string GetName() const override { return m_Info.Name; }

		inline virtual Matrixy2x3 GetFullMatrix() override { Matrixy2x3 temp = m_WorldToHere; if (!temp.Invert()) temp = Matrixy2x3::Identity(); return temp; }

	protected:

		void UpdateShit();

		std::vector<std::unique_ptr<OptionPane>> m_Panes;
		OptionMenuThings m_Info;
		Recty m_BoundingBox; // In Parent Space
		floaty2 m_MinimumDimensions;
		Matrixy2x3 m_WorldToHere;
	};

	struct DefaultOptionMenu : OptionMenu
	{
		DefaultOptionMenu(CommonResources *resources);

	};
}