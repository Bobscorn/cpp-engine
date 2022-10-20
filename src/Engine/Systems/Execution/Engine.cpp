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
		// Try change
		auto old_scene = CurrentScene.release();
		if (old_scene)
			EventManager::Remove(old_scene);
		CurrentScene = std::move(toSwitch);
		auto success = CurrentScene->Initialize();
		ENGINE_PROFILE_POP();
		ENGINE_PROFILE_PUSH("SuccessPart");
		if (success)
		{
			Add(CurrentScene.get());
			toSwitch = nullptr;
			if (old_scene)
				delete old_scene;
		}
		else
		{
			toSwitch = nullptr;
			// Change failed
			if (old_scene && old_scene->ContinueIfFail())
			{
				int response{ -1 };
				if (success.HasErrors())
					response = Debug::ReportErrorCancelTryContinue("Error Occured", success.AsString().c_str());
				else
					response = Debug::ReportErrorCancelTryContinue("Error Occured", "Something failed to load properly, you can either close the application, try to load it again, or continue from where you were");

				if (response == Debug::ContinueID)
				{
					// Switch back to old scene
					CurrentScene.reset(old_scene);
					CurrentScene->WasContinued(&Resources);
					Add(CurrentScene.get());
				}
				else if (response == Debug::RetryID)
				{
					toSwitch = CurrentScene->Clone();
					CurrentScene = nullptr;
					ApplyScene();

				}
				else
				{
					// MessageBox failed or user clicked cancel
					delete old_scene;

					auto bad_scene = CurrentScene.release();

					try { delete bad_scene;	} catch (...) {}

					QuitDatAss();
					return;
				}
			}
			else
			{
				// Switch to error scene
				if (old_scene)
					delete old_scene;
				
				bool retry = false;
				if (success.HasErrors())
					retry = Debug::ReportRetry("Error Occured", success.AsString().c_str());
				else
					retry = Debug::ReportRetry("ErrorOccured", "Fatal Error occured when loading a new scene \n You can retry or quit");

				if (retry)
				{
					auto bad_scene = CurrentScene.release();
					toSwitch = bad_scene->Clone();
					ApplyScene();

					try	{ delete bad_scene;	} 
					catch (...)
					{
						DWARNING("a new scene failed to initialize, user asked for retry, old scene's deletion threw an exception");
					}
				}
				else
				{
					auto bad_scene = CurrentScene.release();

					try { delete bad_scene; }
					catch (...) {}

					QuitDatAss();
					return;
				}
			}
		}
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
