#include "ParkourUI.h"

#include <Structure/BasicShapes.h>

Parkour::ParkourInGameMenu::ParkourInGameMenu(CommonResources* resources)
	: m_Container(resources, "background.jpeg", UI1I::ButtonyContainer::CENTRE_ALIGN, UI1I::ButtonyContainer::CENTRE_ALIGN)
	, m_ResumeButton(resources, "Resume", Requests::Request{ "Resume" }, "Normal")
	, m_QuitButton(resources, "Quit", Requests::Request{ "Quit" }, "Normal")
{
}

void Parkour::ParkourInGameMenu::AddTo(UI1::RootElement& root)
{
	m_Container.AddButton(&m_ResumeButton);
	m_Container.AddButton(&m_QuitButton);
	root.AddChildTop(&m_Container);
}

void Parkour::ParkourInGameMenu::Enable()
{
	m_Container.EnableUI();
}

void Parkour::ParkourInGameMenu::Disable()
{
	m_Container.DisableUI();
}

Parkour::ParkourInGameHUD::ParkourInGameHUD(CommonResources* resources)
	: FullResourceHolder(resources)
	, m_Container(resources, "", UI1I::ButtonyContainer::LEFT_ALIGN, UI1I::ButtonyContainer::TOP_ALIGN)
	, m_FPSDisplay(resources, "Frame Time: ", Requests::Request{ "DoNothing" })
	, m_LightCountDisplay(resources, "Lights: ", Requests::Request{ "DoNothing" })
{
	m_Container.AddButton(&m_FPSDisplay);
	m_Container.AddButton(&m_LightCountDisplay);
}

void Parkour::ParkourInGameHUD::AddTo(UI1::RootElement& root)
{
	root.AddChildTop(&m_Container);
}

void Parkour::ParkourInGameHUD::Update()
{
	m_FPSDisplay.SetText("Frame Time: " + std::to_string(*mResources->DeltaTime));
	m_LightCountDisplay.SetText("Lights: " + std::to_string(G1I::LightShape::GetNumUsedLights()));
}

void Parkour::ParkourInGameHUD::Enable()
{
	m_Container.EnableUI();
}

void Parkour::ParkourInGameHUD::Disable()
{
	m_Container.DisableUI();
}
