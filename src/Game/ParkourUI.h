#pragma once

#include <Game/UI/GameUI.h>
#include <Helpers/TransferHelper.h>

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

	class ParkourInGameHUD
	{
		UI1I::ButtonyContainer m_Container;
		UI1I::SmallButtony m_FPSDisplay;
		UI1I::SmallButtony m_LightCountDisplay;
	public:
		ParkourInGameHUD(CommonResources* resources);


	};
}
