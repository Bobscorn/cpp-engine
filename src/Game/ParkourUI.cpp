#include "ParkourUI.h"

#include <Structure/BasicShapes.h>

Parkour::ParkourInGameMenu::ParkourInGameMenu(CommonResources* resources)
	: m_Container(resources, "background.jpeg", UI1I::ButtonyContainer::CENTRE_ALIGN, UI1I::ButtonyContainer::CENTRE_ALIGN)
	, m_ResumeButton(resources, "Resume", Requests::Request{ "Resume" }, "Normal")
	, m_RestartButton(resources, "Restart", Requests::Request{ "RestartRun" }, "Normal")
	, m_QuitMenuButton(resources, "Back To Menu", Requests::Request{ "ReturnToMenu" }, "Normal")
{
	m_Container.AddButton(&m_ResumeButton);
	m_Container.AddButton(&m_RestartButton);
	m_Container.AddButton(&m_QuitMenuButton);
}

void Parkour::ParkourInGameMenu::AddTo(UI1::RootElement& root)
{
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
#ifdef _DEBUG
	m_Container.AddButton(&m_FPSDisplay);
	m_Container.AddButton(&m_LightCountDisplay);
#endif
}

void Parkour::ParkourInGameHUD::AddTo(UI1::RootElement& root)
{
	root.AddChildTop(&m_Container);
}

void Parkour::ParkourInGameHUD::Update()
{
	m_FPSDisplay.SetText("Frame Time: " + std::to_string((float)(int)(*mResources->DeltaTime * 10000.f) / 10.f) + "ms");
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

Parkour::ParkourFinishMenu::ParkourFinishMenu(CommonResources* resources)
	: FullResourceHolder(resources)
	, m_Container(resources, "menu_backdrop.jpeg", UI1I::ButtonyContainer::CENTRE_ALIGN, UI1I::ButtonyContainer::CENTRE_ALIGN)
	, m_AgainButton(resources, "Begin Another Run", Requests::Request{ "RestartRun" })
	, m_BackButton(resources, "Return to Menu", Requests::Request{ "ReturnToMenu" })
	, m_QuitDesktopButton(resources, "Quit to Desktop", Requests::Request{ "ExitGame" })
	, m_Fader(resources, 1.f)
	, m_Enabled(false)
{
	m_Container.AddButtons({ &m_AgainButton, &m_BackButton, &m_QuitDesktopButton });
}

void Parkour::ParkourFinishMenu::AddTo(UI1::RootElement& root)
{
	root.AddChildTop(&m_Container);
}

void Parkour::ParkourFinishMenu::Update()
{
	if (m_Enabled)
	{
		if (m_Fader.Fade() && !m_Container.IsUIEnabled())
		{
			m_Container.EnableUI();
		}
	}
}

void Parkour::ParkourFinishMenu::Enable()
{
	m_Fader.Reset();
	m_Fader.Start();
	m_Enabled = true;
}

void Parkour::ParkourFinishMenu::Disable()
{
	m_Fader.Stop();
	m_Container.DisableUI();
	m_Enabled = false;
}
