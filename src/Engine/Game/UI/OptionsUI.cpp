#include "OptionsUI.h"

#include "Helpers/MathHelper.h"

#include "Drawing/Graphics2D.h"

#include <iomanip>
#include <sstream>

Options::SliderOption::SliderOption(CommonResources *resources, OptionThings ting1, SliderThings ting2) : FullResourceHolder(resources), Option(ting1), m_Info(ting2), m_Name(resources, ting1.Name, "Normal", "Text"), m_Value(resources, "0.00", "Normal", "Text")
{
}

void Options::SliderOption::IDraw()
{
	auto *Ren = mResources->Ren2;
	Ren->SetTransform(this->LocalToWorld);
	
	// Draw Name
	float x = LocalBounds.left;
	x += m_Active.Margin;
	//float y = LocalBounds.top - m_Active.Margin - m_Active.Border;
	float halfheight = 0.5f * (float)m_Name.GetHeight();
	Ren->DrawImage(&m_Name, { x, 0.8f * halfheight, x + (float)m_Name.GetWidth(), -1.2f * halfheight });
	x += (float)m_Name.GetWidth();
		
	x += m_Active.NameValueGap;

	// Draw Value
	if (m_Info.Display & SliderThings::Value)
	{
		halfheight = 0.5f * (float)m_Value.GetHeight();
		Ren->DrawImage(&m_Value, { x, 0.8f * halfheight, x + (float)m_Value.GetWidth(), - 1.2f * halfheight });
		x += ValueReservedSpace;
		x += m_Active.Margin;
	}

	// Draw Left Arrow
	if (m_Info.Display & SliderThings::Arrows)
	{
		Ren->FillTriangle({ x, 0.f }, { x + m_Active.ArrowSize, 0.5f * m_Active.ArrowSize }, { x + m_Active.ArrowSize, -0.5f * m_Active.ArrowSize }, floaty4{ 0.3f, 0.3f, 0.3f, 1.f });
		x += m_Active.ArrowSize + m_Active.Margin;
	}

	// Draw Slider
	constexpr float middle = 0.f;
	std::vector<floaty2> SliderVertices;
	SliderVertices.resize(12);
	SliderVertices[0] = { x, middle };
	x += 1.73205f;
	SliderVertices[1] = { x, middle + 2.f };
	SliderVertices[2] = { x, middle - 2.f };

	SliderVertices[3] = { x, middle - 2.f };
	SliderVertices[4] = { x, middle + 2.f };
	x += (m_Active.SliderDistance - 2.f * 1.73205f);
	SliderVertices[5] = { x, middle + 2.f };

	SliderVertices[6] = { x, middle + 2.f };
	SliderVertices[7] = { x, middle - 2.f };
	SliderVertices[8] = SliderVertices[3];

	SliderVertices[9] = { x, middle + 2.f };
	SliderVertices[10] = { x, middle - 2.f };
	x += 1.73205f;
	SliderVertices[11] = { x, middle };

	x += m_Active.Margin;

	Ren->FillVertices(SliderVertices, { 0.1f, 0.1f, 0.1f, 1.f });

	// Draw Right Arrow
	if (m_Info.Display & SliderThings::Arrows)
	{
		Ren->FillTriangle({ x, 0.5f * m_Active.ArrowSize }, { x, -0.5f * m_Active.ArrowSize }, { x + m_Active.ArrowSize, 0.f }, floaty4{ 0.3f, 0.3f, 0.3f, 1.f });
		x += m_Active.ArrowSize + m_Active.Margin;
	}

}

bool Options::SliderOption::OnMouseLeftDown(floaty2 &coords)
{
	(void)coords;
	return false;
}

bool Options::SliderOption::OnMouseLeftUp(floaty2 &coords)
{
	(void)coords;
	return false;
}

floaty2 Options::SliderOption::GetMinDimensions() const
{
	float x = 2.f * Minimum.Margin + 2.f * Minimum.Border;
	x += (float)m_Name.GetWidth();
	x += Minimum.SliderDistance;
	if (m_Info.Display & SliderThings::Arrows)
		x += Minimum.Margin * 2.f + 2.f * Minimum.ArrowSize;
	if (m_Info.Display & SliderThings::Value)
		x += Minimum.Margin + ValueReservedSpace;

	float y = 2.f * Minimum.Margin + 2.f * Minimum.Border;
	y += fmaxf((float)m_Name.GetHeight(), Minimum.ArrowSize);
	return { x, y };
}

floaty2 Options::SliderOption::GetIdealDimensions() const
{
	float x = 2.f * Recommended.Margin + 2.f * Recommended.Border;
	x += (float)m_Name.GetWidth();
	x += Recommended.SliderDistance;
	x += Recommended.NameValueGap;
	if (m_Info.Display & SliderThings::Arrows)
		x += Recommended.Margin * 2.f + 2.f * Recommended.ArrowSize;
	if (m_Info.Display & SliderThings::Value)
		x += Recommended.Margin + ValueReservedSpace;

	float y = 2.f * Recommended.Margin + 2.f * Recommended.Border;
	y += fmaxf((float)m_Name.GetHeight(), Recommended.ArrowSize);
	return { x, y };
}

void Options::SliderOption::SetDimensions(floaty2 dim)
{
	LocalBounds.right = dim.x * 0.5f;
	LocalBounds.left = -LocalBounds.right;
	LocalBounds.top = dim.y * 0.5f;
	LocalBounds.bottom = -LocalBounds.top;
	
	float nongap = 2.f * m_Active.Margin + (float)m_Name.GetWidth();
	nongap += m_Active.SliderDistance;
	if (m_Info.Display & SliderThings::Arrows)
		nongap += m_Active.Margin * 2.f + 2.f * m_Active.ArrowSize;
	if (m_Info.Display & SliderThings::Value)
		nongap += m_Active.Margin + ValueReservedSpace;

	float remaining = dim.x - nongap;
	if (remaining < 0.f)
	{
		if (remaining + (m_Active.SliderDistance - Minimum.SliderDistance) > 0.f)
		{
			m_Active.SliderDistance += remaining;
			remaining = 0.f;
		}
		else
		{
			DWARNING("Minimum Width was not respected");
			m_Active.SliderDistance = Minimum.SliderDistance;
			remaining = 0.f;
		}
	}

	m_Active.NameValueGap = remaining;
}

floaty2 Options::SliderOption::GetDimensions() const
{
	float y = 2.f * m_Active.Margin + fmaxf((float)m_Name.GetHeight(), m_Active.ArrowSize);
	float x = 2.f * m_Active.Margin + (float)m_Name.GetWidth();
	if (m_Info.Display & SliderThings::Arrows)
		x += m_Active.Margin * 2.f + 2.f * m_Active.ArrowSize;
	if (m_Info.Display & SliderThings::Value)
		x += m_Active.Margin + ValueReservedSpace;
	x += m_Active.NameValueGap;
	return { x, y };
}

void Options::SliderOption::IDebugDraw()
{
	UIElement::IDebugDraw(); 
	RectangleInfo::DebugDraw(mResources);

	auto *Ren = mResources->Ren2;
	Ren->SetTransform(this->LocalToWorld);
	float x = LocalBounds.left;
	x += m_Active.Margin + m_Name.GetWidth() + m_Active.Margin;

	Ren->DrawLine({ LocalBounds.left, 0.f }, { LocalBounds.right, 0.f }, { 0.6f, 0.6f, 0.2f, 1.f });
	Ren->DrawLine({ x, LocalBounds.top }, { x, LocalBounds.bottom }, { 0.6f, 0.6f, 0.2f, 1.f });

	x += m_Active.NameValueGap - m_Active.Margin;

	Ren->DrawLine({ x, LocalBounds.top }, { x, LocalBounds.bottom }, { 0.6f, 0.6f, 0.2f, 1.f });

	x += m_Active.Margin;

	if (m_Info.Display & SliderThings::Value)
	{
		x += ValueReservedSpace + m_Active.Margin;
		Ren->DrawLine({ x, LocalBounds.top }, { x, LocalBounds.bottom }, { 0.6f, 0.6f, 0.2f, 1.f });
	}

	if (m_Info.Display & SliderThings::Arrows)
	{
		x += m_Active.ArrowSize;
		Ren->DrawLine({ x, LocalBounds.top }, { x, LocalBounds.bottom }, { 0.6f, 0.6f, 0.2f, 1.f });
		x += m_Active.Margin;
	}

	x += m_Active.SliderDistance;
	Ren->DrawLine({ x, LocalBounds.top }, { x, LocalBounds.bottom }, { 0.6f, 0.6f, 0.2f, 1.f });
	x += m_Active.Margin;
	
	if (m_Info.Display & SliderThings::Arrows)
	{
		x += m_Active.ArrowSize;
		Ren->DrawLine({ x, LocalBounds.top }, { x, LocalBounds.bottom }, { 0.6f, 0.6f, 0.2f, 1.f });
		x += m_Active.Margin;
	}
}

void Options::SliderOption::UpdateValue()
{
	try
	{
		float value;
		value = std::any_cast<float>(mResources->Config->Get(Option::m_Info.Key));
		std::stringstream ss;
		ss << std::fixed << std::setprecision(2) << value;
		std::string newvalue = ss.str();
		m_Value.SetText(newvalue);

		m_SliderPosition = Math::Map(value, m_Info.MinVal, m_Info.MaxVal, 0.f, 1.f);
	}
	catch (const Config::CouldNotFindException &e)
	{
		(void)e;
		DWARNING("No value found in config to Update Slider with");
	}
	catch (const std::bad_any_cast &e)
	{
		(void)e;
		DWARNING("Value found in config updating slider is non-floating point");
	}
}

Options::OptionPane::OptionPane(OptionPaneThings ting, CommonResources *resources) : FullResourceHolder(resources), UIElement(true), m_Info(ting)
{
	UpdateShit();
}

Options::OptionPane::OptionPane(OptionPaneThings ting, CommonResources *resources, std::vector<std::unique_ptr<Option>> options) : FullResourceHolder(resources), UIElement(true), m_Info(ting), m_Options(std::move(options))
{
	UpdateShit();
}

void Options::OptionPane::AddOption(std::unique_ptr<Option> option)
{
	auto &ptr = m_Options.emplace_back(std::move(option));
	(void)this->UIElement::AddChildTop(ptr.get());
	UpdateShit();
}

floaty2 Options::OptionPane::GetDimensions() const
{
	return m_InteriorDimensions + (m_Info.BorderThickness * 2.f);
}

void Options::OptionPane::UpdateShit()
{
	// Eventually different spacing behaviours would use different updating logic here
	m_InteriorDimensions = { 0.f, 0.f };
	for (auto &option : m_Options)
	{
		// Only considering the Ideal dimensions, can not be screwed actually implementing any more complex behaviour
		auto dim = option->GetIdealDimensions();
		m_IdealOptionWidth = m_InteriorDimensions.x = fmaxf(m_InteriorDimensions.x, dim.x);
		m_InteriorDimensions.y += dim.y;
	}

	float x = -(m_InteriorDimensions.x * 0.5f);
	float y = -(m_InteriorDimensions.y * 0.5f);
	for (auto &option : m_Options)
	{
		float py = option->GetDimensions().y;
		option->SetDimensions({ m_IdealOptionWidth, py });
		option->PreComputePosition();
		float tmpy = y - py;
		option->RecommendBoundingBox({ x, y, -x, tmpy });
		y = tmpy;
		option->ComputeWorldMatrix(m_WorldToHere);
	}
}

void Options::OptionPane::SetMatrix(Matrixy2x3 worldtohere)
{
	m_WorldToHere = worldtohere;
}

std::string Options::OptionPane::GetName() const
{
	return "Some Random Ass Option Pane";
}

Options::OptionMenu::OptionMenu(OptionMenuThings ting, CommonResources *resources) : FullResourceHolder(resources), UIElement(true), m_Info(ting)
{
}

Options::OptionMenu::OptionMenu(OptionMenuThings tings, CommonResources *resources, std::vector<std::unique_ptr<OptionPane>> panes) : FullResourceHolder(resources), m_Info(tings), m_Panes(std::move(panes))
{
	for (auto &child : m_Panes)
	{
		(void)AddChildTop(child.get());
	}

	UpdateShit();
}

void Options::OptionMenu::AddOptionPane(OptionPane *pane)
{
	m_Panes.emplace_back(pane);
	(void)AddChildTop(pane);
	UpdateShit();
}

Options::OptionPane *Options::OptionMenu::EmplaceOptionPane(OptionPaneThings things)
{
	auto *out = m_Panes.emplace_back(std::make_unique<OptionPane>(things, mResources)).get();
	(void)AddChildTop(out);
	UpdateShit();
	return out;
}

void Options::OptionMenu::ComputePosition(Matrixy2x3 Accumulated)
{
	this->m_WorldToHere = Accumulated;

	Matrixy2x3 ToChild;
	floaty2 runnningpoint = { 0.f, 0.f };
	for (auto &pane : m_Panes)
	{
		ToChild = Matrixy2x3::Translation(runnningpoint.x, runnningpoint.y);
		pane->SetMatrix(m_WorldToHere * ToChild);
		runnningpoint.y += pane->GetDimensions().y;
	}
}

void Options::OptionMenu::RootPosition()
{
	// Find a centering matrix
	// By default RootPosition means the object will by default be in the top left corner

	floaty2 halfwindims = { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight };

	ComputePosition(Matrixy2x3::Identity());

	floaty2 halfmindims = m_MinimumDimensions * 0.5f;
	Recty rec;
	rec.left = -halfmindims.x;
	rec.right = halfmindims.x;
	rec.bottom = -halfmindims.y;
	rec.top = halfmindims.y;
	RecommendBoundingBox(rec);
}

bool Options::OptionMenu::Within(floaty2 point)
{
	floaty2 herepoint = m_WorldToHere.TransformPoint(point);
	return (herepoint.x > m_BoundingBox.left && herepoint.x < m_BoundingBox.right &&
		herepoint.y < m_BoundingBox.top && herepoint.y > m_BoundingBox.bottom);
}

void Options::OptionMenu::MatrixChanged()
{
}

Recty Options::OptionMenu::GetBoundingBox()
{
	return m_BoundingBox;
}

void Options::OptionMenu::RecommendBoundingBox(Recty rect)
{
	// Rescale bounding box if its too small, but keep the given center
	floaty2 center = { rect.left + 0.5f * rect.right, rect.bottom + 0.5f * rect.top };
	floaty2 dims = { rect.right - rect.left, rect.top - rect.bottom };
	dims.x = fmaxf(m_MinimumDimensions.x, dims.x);
	dims.y = fmaxf(m_MinimumDimensions.y, dims.y);
	floaty2 halfdims = { dims.x * 0.5f, dims.y * 0.5f };

	m_BoundingBox.left = center.x - halfdims.x;
	m_BoundingBox.right = center.x + halfdims.x;
	m_BoundingBox.bottom = center.y - halfdims.y;
	m_BoundingBox.top = center.y + halfdims.y;
}

void Options::OptionMenu::UpdateShit()
{
	// Update Minimum Bounding Box
	m_MinimumDimensions = { 0.f, 0.f };
	for (auto &pane : m_Panes)
	{
		floaty2 dims = pane->GetDimensions();
		m_MinimumDimensions.x = fmaxf(m_MinimumDimensions.x, dims.x);
		m_MinimumDimensions.y = fmaxf(m_MinimumDimensions.y, dims.y);
	}
	RecommendBoundingBox(m_BoundingBox);
}

Options::DefaultOptionMenu::DefaultOptionMenu(CommonResources *resources) : FullResourceHolder(resources), OptionMenu({ "Big Boi Menu", MenuDisplayMode::Line, OverflowBehaviour::Vertical }, resources)
{
	auto *pane = EmplaceOptionPane({ {1.f, 1.f} });
	pane->AddOption(std::make_unique<SliderOption>(resources, OptionThings{ "Field Of View", Config::KeyC{"FOV"} }, SliderThings{ SliderThings::Arrows | SliderThings::Value, 0.f, 1.f, 0.1f, false }));
	pane->AddOption(std::make_unique<SliderOption>(resources, OptionThings{ "Mouse Sensitivity", Config::KeyC{"MouseSensitivity"} }, SliderThings{ SliderThings::Arrows | SliderThings::Value, 150.f, 350.f, 5.f, false }));
}

TextInput::BaseInput::BaseInput(BaseInputThings things) : m_Things(things), m_CursorLocation((size_t) - 1)
{
}
