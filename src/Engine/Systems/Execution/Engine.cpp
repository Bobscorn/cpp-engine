#include "Engine.h"

extern void QuitDatAss();

Engine::IEngine::IEngine() : Delta(0.0), TargetUpdateInterval(1.0 / 60.0), m_RandomGen{ std::uniform_int_distribution<size_t>()(m_RandomDevice) }
#ifdef EC_PROFILE
, m_Profiler({})
#endif
{
#ifdef EC_PROFILE
	Resources.Profile = &m_Profiler;
#endif
	Resources.RandomDevice = &m_RandomDevice;
	Resources.RandomGen = &m_RandomGen;
	Resources.Request = &Req; 
	Resources.InputAttachment = &InpAtt; 
	Resources.UpdateID = &CurrentUpdateID;
	Resources.WorkingDirectory = "";
	Resources.Engine = this;
	Resources.Event = this;
	Resources.DeltaTime = &Delta;
	Resources.Time = &Time;
	Resources.TargetUpdateInterval = &TargetUpdateInterval;
	timmy.Reset(); 
	timmy.Start(); 
	Add(&InpAtt);
}

void Engine::IEngine::ApplyScene()
{
	if (toSwitch)
	{
		ENGINE_PROFILE_PUSH("TrySceneChange");
		CurrentScene = nullptr;
		CurrentScene = std::move(toSwitch);
		CurrentScene->Initialize();
		ENGINE_PROFILE_POP();
	}
}

void Engine::IEngine::OnDpiChanged()
{
	// Not done yet (check 'removed unused code' commit for old content)
}

Engine::IWindowEngine::IWindowEngine() : Win("", "Config/windowconfig.xml", SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE), UIC(this), Ren(&Resources), Config(Config::GetDefaultConfigThrone())
{
	Resources.Ren2 = &Ren;
	Resources.Ren3 = &Ren;
	Resources.Ren3v2 = Ren.Getv2();
	Resources.Window = Win.Get();
	Resources.UIConfig = &UIC;
	Resources.Config = Config.get();
	Resources.WindowWidth = Win.GetPWinWidth();
	Resources.WindowHeight = Win.GetPWinHeight();
	Resources.HalfWindowWidth = &HalfWinWidth;
	Resources.HalfWindowHeight = &HalfWinHeight;
	Resources.CenterX = Win.GetPCenterX();
	Resources.CenterY = Win.GetPCenterY();
	Resources.AspectRatio = Win.GetPAspectRatio();
	Resources.InverseWindowWidth = Win.GetPInvWidth();
	Resources.InverseWindowHeight = Win.GetPInvHeight();
	Resources.DpiScale = &DpiScale;
	Resources.InverseDpiScale = &InvDpiScale;
	Resources.MousePosition = &MousePos;
	Resources.MouseChangedPosition = &MouseMoved;
	Resources.WindowFocused = &Window_Focused;

	HalfWinWidth = ((float)Win.Width) * 0.5f;
	HalfWinHeight = ((float)Win.Height) * 0.5f;

	Ren.Resize(Win.Width, Win.Height);
}

void Engine::IWindowEngine::OnResize(int width, int height)
{
	Win.SetNewDims(width, height);

	HalfWinWidth = ((float)Win.Width) * 0.5f;
	HalfWinHeight = ((float)Win.Height) * 0.5f;

	Ren.Resize((unsigned int)width, (unsigned int)height);

	auto winSize = Vector::inty2{ Win.GetWidth(), Win.GetHeight() };
	Event::ResizePreEvent asss(winSize, CurrentUpdateID);
	Send(&asss);

	Event::ResizeEvent ass = Event::ResizeEvent(winSize, CurrentUpdateID);
	Send(&ass);
}

void Engine::IWindowEngine::OnMinimize()
{
	Win.WriteToFile("");
}

void Engine::IWindowEngine::OnMaximize()
{
	Win.Maximized = true;
}

void Engine::IWindowEngine::OnDeMaximize()
{
	Win.Maximized = false;
}

void Engine::IWindowEngine::PreUpdate()
{
	if (Window_Focused)
	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		MouseMoved = (MousePos.x != x) || (MousePos.y != y);
		MousePos.x = x;
		MousePos.y = y;
	}
}
