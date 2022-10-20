#include "ParkourUI.h"

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
