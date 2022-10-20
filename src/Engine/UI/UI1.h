#pragma once


#include "Helpers/TransferHelper.h"
#include "Helpers/PointerHelper.h"

#include "Math/floaty.h"

#include "Helpers/DebugHelper.h"
#include "Systems/Events/Events.h"
#include "Systems/Execution/ResourceHolder.h"
#include "Systems/Input/InputAttachment.h"
#include "UIPositioner.h"

#include <set>
#include <algorithm>

namespace UI1
{
	struct UIElement;
	struct UIFloatElement;

	class RootElement;

	/// <summary> A UIElement represents an object that resides in a Tree structure User Interface </summary>
	/// <remarks>
	/// It contains the usual logic methods (BeforeDraw/Draw/AfterDraw)
	/// And it contains Positioning/Sizing Information for dynamic support for multiple window sizes
	/// Also contains a depth and a boolean indicating whether its enabled
	/// The <paramref="Depth"/> member variable is only used to allow any parent to decide which child is drawn first or receives input when overlaps are present
	struct UIElement : public virtual FullResourceHolder
	{
	protected:
		bool Enabled = true;

		UIElement *FindElement(floaty2 coords, bool trace = false);

		bool MouseMove(floaty2 coords);
		void MouseLeftClick(floaty2 coords);
		void MouseRightClick(floaty2 coords);
		void MouseWheelClick(floaty2 coords);
		bool MouseScroll(floaty2 coords, INT scrollage);
		bool MouseLeftDown(floaty2 coords);
		bool MouseRightDown(floaty2 coords);
		bool MouseWheelDown(floaty2 coords);
		bool MouseLeftUp(floaty2 coords);
		bool MouseRightUp(floaty2 coords);
		bool MouseWheelUp(floaty2 coords);
		bool MouseHover(floaty2 coords);

		// Used for Default Hover Behaviour
		size_t HoverFrameID = 0ull;

		inline virtual UIElement *FindOverride(floaty2 coords) { (void)coords; return nullptr; } // In case a specific element has special finding functionality, eg static objects, FindElement returns whatever this returns first

		inline unsigned int GetBottomDepth() 
		{ 
			unsigned int depth = 0u; 
			if (Children.size())
				if (Children.front()->Depth > 0)
					depth = Children.front()->Depth - 1u;
				else
					depth = 0u;
			return depth;
		}
		inline unsigned int GetTopDepth() 
		{ 
			unsigned int depth = 0u;
			if (Children.size())
				depth = Children.back()->Depth + 1;
			return depth;
		}
		
		inline bool OnTop(UIElement *child) { return Children.size() && child == Children.back(); }
		inline bool AtBottom(UIElement *child) { return Children.size() && child == Children.front(); }

	public:
		unsigned int Depth;

		RootElement *Root = nullptr;
		UIElement* Parent = nullptr;
		std::vector<UIElement*> Children;

		/// <summary> Constructor giving only the PositionInfo of this element </summary>
		/// <param name="pinfo">The Positioning/Sizing Information used for this element</param>
		UIElement() {};
		UIElement(unsigned int Depth) : Depth(Depth) {};
		UIElement(bool Enabled) : Enabled(Enabled) {};
		UIElement(unsigned int Depth, bool Enabled) : Depth(Depth), Enabled(Enabled) {};
		virtual ~UIElement() {};


		/// <summary> Virtual Initialization Method expected to be called the object that holds this actual object
		/// TO BE CALLED *AFTER* BEING ADDED TO PARENT, as this object receives its mResources pointer from its parent </summary>
		/// <remarks> There is no recursive/child initializing behaviour with this method unlike the <code>BeforeDraw</code> and other methods that go down the hierachy
		/// This virtual Initialize method does no such thing </remarks>
		virtual Debug::DebugReturn Initialize() = 0;

		/// <summary> Goes down UIElement the hierachy starting here, calling the IBeforeDraw/IDraw/IAfterDraw virtual method </summary>
		/// <remarks> This method is expected to be called once a frame under normal conditions and only by the root node in a UI tree structure </remarks>
		virtual void BeforeDraw();
		virtual void Draw();
		virtual void AfterDraw();

		virtual void IBeforeDraw() = 0; /// <summary> The virtual method assuming to be implemented with special logic to be called once every frame before drawing </summary>
		virtual void IDraw() = 0; /// <summary> The virtual drawing logic to be called when drawing is taking place </summary>
		virtual void IAfterDraw() = 0;

		inline void DebugDraw()
		{
			IDebugDraw();

			for (auto& child : Children)
				child->DebugDraw();
		}

		virtual void IDebugDraw();
		virtual Stringy GetName() const = 0;

		typedef bool DoConsume;

		// Mouse Functions
		// Returns whether to consider the event 'consumed'
		virtual DoConsume OnMouseMove(floaty2 &coords) = 0; // Generally avoided as mouse move events can have reasonable overhead, polling systems are preferred
		virtual DoConsume OnMouseLeftClick(floaty2 &coords) = 0;
		virtual DoConsume OnMouseRightClick(floaty2 &coords) = 0;
		virtual DoConsume OnWheelClick(floaty2 &coords) = 0;
		virtual DoConsume OnMouseScroll(floaty2 &coords, INT scrollage) = 0;

		// Returns whether to consider the event 'consumed'
		virtual DoConsume OnMouseLeftDown(floaty2 &coords) = 0;
		virtual DoConsume OnMouseRightDown(floaty2 &coords) = 0;
		virtual DoConsume OnMouseLeftUp(floaty2 &coords) = 0;
		virtual DoConsume OnMouseRightUp(floaty2 &coords) = 0;
		virtual DoConsume OnWheelDown(floaty2 &coords) = 0;
		virtual DoConsume OnWheelUp(floaty2 &coords) = 0;

		// Called when the mouse was pressed on this element, but was released elsewhere
		// Returns whether or not to send OnMouseLeft/RightUp as well
		virtual DoConsume OnMouseLeftLost(floaty2 &coords) { (void)coords; return true; }
		virtual DoConsume OnMouseRightLost(floaty2 &coords) { (void)coords; return true; }
		virtual DoConsume OnWheelLost(floaty2 &coords) { (void)coords; return true; }

		typedef bool SendUp;
		constexpr static SendUp DoSendUp = true;
		constexpr static SendUp DontSentUp = false;
		
		// Called when Mouse was released on this element, but was not pressed on this element
		virtual SendUp OnMouseLeftUnexpectedUp(floaty2 &coords) = 0;
		virtual SendUp OnMouseRightUnexpectedUp(floaty2 &coords) = 0;
		virtual SendUp OnWheelUnexpectedUp(floaty2 &coords) = 0;

		enum FocusLoss
		{
			FL_UNKNOWN = 0, // Default
			FL_ELEMENTCLICKED = 1, // Lost Focus as another Element was 'clicked' ('clicked' meaning either up or down mouse events)
			FL_FOCUSCLAIMED = 2, // Lost Focus because another Element claimed it
			FL_FOCUSRESET = 3, // Lost Focus because something called for a Focus Reset
			FL_WINDOWLOSS = 4, // Lost Focus because something happened to the window (probably minimized)
		};

		/// <summary>Called when this Element claimed Focus, and another element was clicked, or any other focus destroying events occured (ie window minimize)</summary>
		/// <remarks>The UIElement pointer is the address of the element that took the focus (received a focus event like mouse down, not by claiming focus)
		/// The pointer will be null for certain Focus losses (eg WindowFocus) </remarks>
		/// <returns> Returns true if it wants to keep focus, false otherwise </returns>
		virtual bool OnFocusLost(FocusLoss loss, UIElement *to) { (void)loss; (void)to; return false; }

		// Will always be received BeforeDraw, also the resources pointer contains the FrameID
		virtual bool OnHoveringThisFrame(size_t FrameID, floaty2 &coords) { HoverFrameID = FrameID; (void)coords; return true; }

		// Dpi changes are usually send via events, but UI has been designed so that only the root node receives events
		virtual void OnDpiChanged(float newdpi, float olddpi, float dpichangescale) { (void)newdpi; (void)olddpi; (void)dpichangescale; }

		virtual bool Within(floaty2 coords) = 0;
		virtual Matrixy2x3 GetParentToHereMatrix() = 0;
		virtual Matrixy2x3 GetFullMatrix() = 0; // Used by UIElement::IDebugDraw
		virtual Matrixy2x3 GetChildMatrix() = 0; // Not always used, just in case they're needed in some odd case
		virtual void PreComputePosition() = 0; // Used to tell elements they should modify their LocalBounds or similar as its about to be modified by parent
		virtual void ComputePosition(Matrixy2x3 accumulated) = 0; // Assumed that this method will properly update all matrices, and any children that need to be
		virtual void ChildAdded(UIElement *el) = 0;
		virtual void ChildrenAdded(std::vector<UIElement*> els) = 0;
		virtual void RootPosition() = 0; // Called by the Root whenever being Positioned by it
		virtual void ChildNeedsUpdate() { if (Parent) ComputePosition(Parent->GetChildMatrix()); else ComputePosition(Matrixy2x3::Identity()); }
		

		template<class T>
		Debug::DebugReturn AddChildren(unsigned int depth, std::vector<std::enable_if_t<std::is_base_of_v<UIElement, T>, T*>> els);

		/// <summary> Methods that Add Children to the UIElement </summary>
		Debug::DebugReturn AddChildBottom(UIElement *e);
		Debug::DebugReturn AddChildrenBottom(std::vector<UIElement*> els);
		Debug::DebugReturn AddChildrenTop(std::vector<UIElement*> els);
		Debug::DebugReturn AddChildTop(UIElement *e);
		Debug::DebugReturn AddChild(UINT depth, UIElement *e);

		inline bool IsUIEnabled() { return Enabled; }
		inline void EnableUI() { Enabled = true; }
		inline void DisableUI() { Enabled = false; }

		// Requires that the OnHoveringThisFrame virtual method sets HoverFrameID to the FrameID given every time
		inline bool BeingHovered(size_t currentFrameId) { return HoverFrameID == currentFrameId; }
		bool BeingClicked() const;
	};

	/// <summary> A UIFloatElement inherits from UIElement, with its major difference being drawn last (on top), and receiving input first </summary>
	/// <remarks>
	/// UIFloatElements are assumed to be stored separately from UIElements, whether they be stored by the Root of a User Interface tree, they also assume they will be 
	/// drawn after/on top of any UIElements, and that they have first pick at any input events
	/// An obvious use for such an element would be to represent something similar to a pop-up that should be on top of every other UI element
	///</remarks>
	struct UIFloatElement : public UIElement
	{
		UIFloatElement() : UIElement() {}
		UIFloatElement(bool Enabled) : UIElement(Enabled) {}
		virtual ~UIFloatElement();
		
		virtual void Added() { };
	private:
		using UIElement::Parent;
	};

	struct UIElementLess
	{
		inline bool operator() (const UIElement* e, const UIElement* e2) const { return e->Depth < e2->Depth; }
	};

	struct UIFloatElementLess
	{
		inline bool operator() (const UIFloatElement* e, const UIFloatElement* e2) const { return e->Depth < e2->Depth; }
	};

	class RootElement : public UIElement, public InputAttach::IAttachable, public Events::IEventListenerT<Events::ResizeWindowEvent, Events::WindowFocusEvent>
	{
		using UIElement::BeforeDraw;
		std::vector<UIFloatElement*> FloatElements;
	protected:
		UIElement * FindOverride(floaty2 coords) override;

		UIElement *LeftMouseDownEl{ nullptr };
		UIElement *RightMouseDownEl{ nullptr };
		UIElement *WheelDownEl{ nullptr };
		UIElement *Focus{ nullptr };

		bool DebugDraw = false;
		bool TraceClicks = false;
		bool DoHover = true;
		bool DoMouseMovement = true;
	public:
		RootElement(CommonResources *resources, bool attach = false) { this->Root = this; SetResources(resources); if (attach) resources->InputAttachment->Add(this); resources->Event->Add(this); }

		// Another reminder to call this and any UIElement Initialize methods *AFTER* you add *ALL* children
		Debug::DebugReturn Initialize() { ComputePositions(); return true; }

		void IBeforeDraw() override {}
		void IDraw() override {}
		void IAfterDraw() override {}

		void BeforeDraw();
		void Draw() override;

		// IEventListener methods (from IEventListenerT<>) (also IAttachable has these methods but thats to replicate IEventListener functionality)
		bool Receive(Events::IEvent *event) override;
		bool Receive(Event::WindowFocusEvent *winfocus) override;
		bool Receive(Event::ResizeEvent *resize) override;
		bool Receive(Event::MouseWheel *scrollevent) override;
		bool Receive(Event::MouseWheelButton *event) override;
		bool Receive(Event::MouseButton *mousebuttonevent) override;
		bool Receive(Event::MouseMove *mousemoveevent) override;
		bool Receive(Event::KeyInput *key) override; // Key Input has not been integrated into UI design just yet

		// IAttachable methods
		void Superseded() override;
		void Reinstated() override;

		inline void EnableDebugDraw() { DebugDraw = true; }
		inline void DisableDebugDraw() { DebugDraw = false; }
		inline void ToggleDebugDraw() { DebugDraw = !DebugDraw; }
		inline void DisableHover() { DoHover = false; }
		inline void EnableHover() { DoHover = true; }
		inline void ToggleHover() { DoHover = !DoHover; }
		inline void DisableMouseMovement() { DoMouseMovement = false; }
		inline void EnableMouseMovement() { DoMouseMovement = true; }
		inline void ToggleMouseMovement() { DoMouseMovement = !DoMouseMovement; }
		inline Stringy GetName() const override { return "UI RootElement"; }
		inline bool DoTrace() const { return TraceClicks; }

		inline void ComputePositions() { for (auto& child : Children) { child->RootPosition(); child->PreComputePosition(); child->ComputePosition(Matrixy2x3::Identity()); } for (auto& fchild : FloatElements) { fchild->RootPosition(); fchild->PreComputePosition(); fchild->ComputePosition(Matrixy2x3::Identity()); } }

		// Methods for UIElements
		inline UIElement *GetCurrentFocus() const { return Focus; }
		inline void ClaimFocus(UIElement *e) { if (Focus) Focus->OnFocusLost(UIElement::FocusLoss::FL_FOCUSCLAIMED, e); Focus = e; }
		inline void ResetFocus(UIElement::FocusLoss loss = UIElement::FocusLoss::FL_FOCUSRESET) { if (Focus) Focus->OnFocusLost(loss, nullptr); Focus = nullptr; }
		inline UIElement *GetLeftMouseDown() const { return LeftMouseDownEl; }
		inline UIElement *GetRightMouseDown() const { return RightMouseDownEl; }
		inline UIElement *GetWheelDown() const { return WheelDownEl; }
		inline void ResetLeftMouseDown(UIElement *e = nullptr) { LeftMouseDownEl = e; }
		inline void ResetRightMouseDown(UIElement *e = nullptr) { RightMouseDownEl = e; }
		inline void ResetWheelDown(UIElement *e = nullptr) { WheelDownEl = e; }

		inline void CFFC(UIElement *el) 
		{ 
			if (el != Focus && Focus)
			{
				if (DebugDraw)
					DINFO("UI Debug: Element Name '" + el->GetName() + "' ");
				if (!Focus->OnFocusLost(UIElement::FocusLoss::FL_ELEMENTCLICKED, el))
					Focus = nullptr;
			}
		}
		inline bool LMBD(UIElement *el, floaty2 coords) { auto r = el->OnMouseLeftDown(coords); if (r) { LeftMouseDownEl = el; CFFC(el); } return r; } // LMBD = Left Mouse Button Down, wrapper of el->OnLeftMouseDown();
		inline bool RMBD(UIElement *el, floaty2 coords) { auto r = el->OnMouseRightDown(coords); if (r) { RightMouseDownEl = el; CFFC(el); }  return r; }
		inline bool WMBD(UIElement *el, floaty2 coords) { auto r = el->OnWheelDown(coords); if (r) { WheelDownEl = el; CFFC(el); } return r; }
		inline bool LMBU(UIElement *el, floaty2 coords) 
		{ 
			auto r = el->OnMouseLeftUp(coords);
			if (LeftMouseDownEl == el)
				el->OnMouseLeftClick(coords);
			else if (LeftMouseDownEl) 
			{
				el->OnMouseLeftUnexpectedUp(coords); 
				if (LeftMouseDownEl->OnMouseLeftLost(coords))
					LeftMouseDownEl->OnMouseLeftUp(coords);
			}
			if (r)
			{
				ResetLeftMouseDown();
				CFFC(el);
			}
			return r;
		}
		inline bool RMBU(UIElement *el, floaty2 coords)
		{
			auto r = el->OnMouseRightUp(coords);
			if (RightMouseDownEl == el)
				el->OnMouseRightClick(coords);
			else if (RightMouseDownEl)
			{
				el->OnMouseRightUnexpectedUp(coords);
				if (RightMouseDownEl->OnMouseRightLost(coords))
					RightMouseDownEl->OnMouseRightUp(coords);
			}
			if (r)
			{
				ResetRightMouseDown();
				CFFC(el);
			}
			return r;
		}
		inline bool WMBU(UIElement *el, floaty2 coords)
		{
			auto r = el->OnWheelUp(coords);
			if (WheelDownEl == el)
				el->OnWheelClick(coords);
			else if (WheelDownEl)
			{
				el->OnWheelUnexpectedUp(coords);
				if (WheelDownEl->OnWheelLost(coords))
					WheelDownEl->OnWheelUp(coords);
			}
			if (r)
			{
				ResetWheelDown();
				CFFC(el);
			}
			return r;
		}
		/*inline bool Scroll(UIElement *el, floaty2 coords, INT Scrollage)
		{
			// TODO: Sort out other mouse methods
		}*/

		/// UIElement methods
		//
		bool Within(floaty2 point) override { (void)point; return true; }
		bool OnMouseMove(floaty2 &coords) override { (void)coords; return true; }
		bool OnMouseLeftClick(floaty2 &coords) override { (void)coords; return true; }
		bool OnMouseRightClick(floaty2 &coords) override { (void)coords; return true; }
		bool OnWheelClick(floaty2 &coords) override { (void)coords; return true; }
		bool OnMouseScroll(floaty2 &coords, INT scrollage) override { (void)coords; (void)scrollage; return true; }

		bool OnMouseLeftDown(floaty2 &coords) override { (void)coords; return true; }
		bool OnMouseRightDown(floaty2 &coords) override { (void)coords; return true; }
		bool OnWheelDown(floaty2 &coords) override { (void)coords; return true; }
		bool OnMouseLeftUp(floaty2 &coords) override { (void)coords; return true; }
		bool OnMouseRightUp(floaty2 &coords) override { (void)coords; return true; }
		bool OnWheelUp(floaty2 &coords) override { (void)coords; return true; }

		bool OnMouseLeftUnexpectedUp(floaty2 &coords) override { (void)coords; return false; }
		bool OnMouseRightUnexpectedUp(floaty2 &coords) override { (void)coords; return false; }
		bool OnWheelUnexpectedUp(floaty2 &coords) override { (void)coords; return false; }

		inline Matrixy2x3 GetParentToHereMatrix() override { return Matrixy2x3::Identity(); }
		inline Matrixy2x3 GetFullMatrix() override { return Matrixy2x3::Identity(); }
		inline Matrixy2x3 GetChildMatrix() override { return Matrixy2x3::Identity(); }
		inline void PreComputePosition() override {}
		inline void ComputePosition(Matrixy2x3 accumulated) override { (void)accumulated; }
		void ChildAdded(UIElement * el) override;
		void ChildrenAdded(std::vector<UIElement*> els) override;
		inline void RootPosition() override {}
		//

		inline void AddFloatChildPosition(UINT depth, UIFloatElement *e) 
		{
			e->Depth = depth;
			RootElement::AddFloatChildPosition(e);
		}

		inline void AddFloatChildPositionTop(UIFloatElement *e) 
		{
			RootElement::AddFloatChildPosition((FloatElements.size() ? FloatElements.back()->Depth + 1 : 0u), e);
		}

		inline void AddFloatChildPositionBottom(UIFloatElement *e) 
		{
			RootElement::AddFloatChildPosition((FloatElements.size() ? (FloatElements.front()->Depth ? FloatElements.front()->Depth - 1 : 0u) : 0u), e);
		}

		inline void AddFloatChildPosition(UIFloatElement *e) 
		{
			RootElement::AddFloatChild(e);
			e->RootPosition();
			e->ComputePosition(Matrixy2x3::Identity());
		}

		inline void AddFloatChild(UINT depth, UIFloatElement *e) 
		{
			e->Depth = depth;
			RootElement::AddFloatChild(e);
		}

		inline void AddFloatChildTop(UIFloatElement *e) 
		{
			RootElement::AddFloatChild((FloatElements.size() ? FloatElements.back()->Depth + 1 : 0u), e);
		}

		inline void AddFloatChildBottom(UIFloatElement *e) 
		{
			RootElement::AddFloatChild((FloatElements.size() ? (FloatElements.front()->Depth ? FloatElements.front()->Depth - 1 : 0u) : 0u), e);
		}

		inline void AddFloatChild(UIFloatElement *e) 
		{
			if (!e->GetResources())
				e->SetResources(mResources);
			e->Root = this;
			FloatElements.push_back(e);
			std::stable_sort(FloatElements.begin(), FloatElements.end(), UIFloatElementLess());
		}

		inline void RemoveFloatChild(UIFloatElement *e) 
		{
			for (unsigned int i = (unsigned int)FloatElements.size(); i-- > 0; )
				if (FloatElements[i] == e)
				{
					std::swap(FloatElements[i], FloatElements.back());
					FloatElements.pop_back();
					std::stable_sort(FloatElements.begin(), FloatElements.end(), UIFloatElementLess());
					e->Root = nullptr;
					return;
				}

			DWARNING("requested element was not found, and as such no removal was made, nor was the element's FloatController pointer reset");
		}
	};

	template<class T>
	inline Debug::DebugReturn UI1::UIElement::AddChildren(unsigned int depth, std::vector<std::enable_if_t<std::is_base_of_v<UIElement, T>, T*>> els)
	{
		std::vector<UIElement*> children(els.size(), nullptr);
		for (UINT i = 0; i < els.size(); ++i)
		{
			if (!els[i]->GetResources())
				els[i]->SetResources(mResources);

			els[i]->Depth = depth;
			els[i]->Parent = this;
			els[i]->Root = this->Root;
			children[i] = els[i];
		}

		Children.insert(Children.end(), children.begin(), children.end());
		std::stable_sort(Children.begin(), Children.end(), UIElementLess());

		Stringy Errors = "Errors initializing UIElements are: ";

		for (auto& child : children)
		{
			auto d = child->Initialize();
			if (!d)
				if (d.HasErrors())
					Errors += "Shape: '" + child->GetName() + d.AsString();
				else
					Errors += "Shape: '" + child->GetName() + "Unknown Error";
		}

		ChildrenAdded(children);

		if (Errors.size() > 37u)
			return Errors;
		return true;
	}

	template<>
	inline Debug::DebugReturn UIElement::AddChildren<UIElement>(unsigned int depth, std::vector<UIElement*> els)
	{
		for (UINT i = 0; i < els.size(); ++i)
		{
			if (!els[i]->GetResources())
				els[i]->SetResources(mResources);

			els[i]->Depth = depth;
			els[i]->Parent = this;
			els[i]->Root = this->Root;
		}

		Children.insert(Children.end(), els.begin(), els.end());
		std::stable_sort(Children.begin(), Children.end(), UIElementLess());

		Stringy Errors = "Errors initializing UIElements are: ";

		for (auto& el : els)
		{
			auto d = el->Initialize();
			if (!d)
				if (d.HasErrors())
					Errors += "Shape: '" + el->GetName() + d.AsString();
				else
					Errors += "Shape: '" + el->GetName() + "Unknown Error";
		}

		this->ChildrenAdded(els);

		if (Errors.size() > 37u)
			return Errors;
		return true;
	}

	struct BlankElement : virtual UIElement
	{
		// Inherited via UIElement
		inline virtual Debug::DebugReturn Initialize() override { return true; }
		inline virtual void IBeforeDraw() override {}
		inline virtual void IDraw() override {}
		inline virtual void IAfterDraw() override {}
		inline virtual bool OnMouseMove(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseLeftClick(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseRightClick(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnWheelClick(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseScroll(floaty2 & coords, INT scrollage) override { (void)coords; (void)scrollage; return true; }
		inline virtual bool OnMouseLeftDown(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseRightDown(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseLeftUp(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseRightUp(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnWheelDown(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnWheelUp(floaty2 & coords) override { (void)coords; return true; }
		inline virtual bool OnMouseLeftUnexpectedUp(floaty2 & coords) override { (void)coords; return false; }
		inline virtual bool OnMouseRightUnexpectedUp(floaty2 & coords) override { (void)coords; return false; }
		inline virtual bool OnWheelUnexpectedUp(floaty2 & coords) override { (void)coords; return false; }
		inline virtual bool Within(floaty2 coords) override { (void)coords; return true; }
		inline Matrixy2x3 GetParentToHereMatrix() override { return Matrixy2x3::Identity(); }
		inline Matrixy2x3 GetFullMatrix() override { return Matrixy2x3::Identity(); }
		inline Matrixy2x3 GetChildMatrix() override { return Matrixy2x3::Identity(); }
		inline virtual void PreComputePosition() override {}
		inline virtual void ComputePosition(Matrixy2x3 accumulated) override { (void)accumulated; }
		inline virtual void ChildAdded(UIElement * el) override { (void)el; }
		inline virtual void ChildrenAdded(std::vector<UIElement*> els) override { (void)els; }
		inline virtual void RootPosition() override {}
	};
}