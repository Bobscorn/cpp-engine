#pragma once

#include <Game/UI/GameUI.h>
#include <Helpers/TransferHelper.h>
#include <Drawing/Effects/Fade.h>

namespace Parkour
{
	class ParkourInGameMenu
	{
		UI1I::ButtonyContainer m_Container;
		UI1I::NormalButtony m_ResumeButton;
		UI1I::NormalButtony m_QuitButton;

	public:
		ParkourInGameMenu(CommonResources *resources);

		void AddTo(UI1::RootElement& root);

		void Enable();
		void Disable();
	};

	class ParkourInGameHUD : public FullResourceHolder
	{
		UI1I::ButtonyContainer m_Container;
		UI1I::BigButtony m_FPSDisplay;
		UI1I::BigButtony m_LightCountDisplay;
	public:
		ParkourInGameHUD(CommonResources* resources);

		void AddTo(UI1::RootElement& root);

		void Update();

		void Enable();
		void Disable();
	};

	class ParkourFinishMenu : public FullResourceHolder
	{
		SolidFader m_Fader;
		UI1I::ButtonyContainer m_Container;
		UI1I::BigButtony m_AgainButton;
		UI1I::BigButtony m_BackButton;
		UI1I::BigButtony m_QuitDesktopButton;
		bool m_Enabled = false;
	public:
		ParkourFinishMenu(CommonResources* resources);

		void AddTo(UI1::RootElement& root);

		void Update();

		void Enable();
		void Disable();
		inline bool IsEnabled() const { return m_Enabled; }
	};
}
