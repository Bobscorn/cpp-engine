#pragma once

#include "Helpers/DebugHelper.h"
#include "Helpers/TransferHelper.h"
#ifdef EC_PROFILE
#include "Helpers/ProfileHelper.h"
#endif

#include "Config/Config.h"

#include "Systems/Requests/Requester.h"
#include "Systems/Input/InputAttachment.h"
#include "Systems/SDLWrapping/SDLWrappers.h"
#include "Systems/Input/Config1.h"
#include "Systems/Events/Events.h"
#include "Systems/Timer/Timer.h"

#include "Drawing/GLRen.h"

#include "Scene.h"

#include <random>

#ifdef EC_PROFILE
#	ifdef _DEBUG
#		define ENGINE_PROFILE_PUSH(x) Resources.Profile->Push(x, __FUNCTION_NAME__, __LINE__)
#		define ENGINE_PROFILE_POP() Resources.Profile->Pop(__FUNCTION_NAME__, __LINE__)
#	else
#		define ENGINE_PROFILE_PUSH(x) Resources.Profile->Push(x)
#		define ENGINE_PROFILE_POP() Resources.Profile->Pop()
#	endif
#else
#	define ENGINE_PROFILE_PUSH(x)
#	define ENGINE_PROFILE_POP() 
#endif

namespace Engine
{
	/// <summary> The only thing the main method sees </summary>
	/// Flow is like so:
	/// - Each Frame:
	///   - BeforeDraw
	///   - Draw - D3D First, D2D Last (not forced)
	///   - AfterDraw
	///   - Present (Not an actual method, but the Swapchain->Present(1, 0))
	struct IEngine : public Events::EventManager, public Scene::SceneManager
	{
		IEngine();
		virtual ~IEngine() noexcept {};
		
		inline void SwitchScene(Scene::IScene *scene) override { toSwitch = scene; } // Switch to this scene at the end of the current update

		// Updating and Drawing functions
		virtual void BeforeDraw()	= 0;
		virtual void Draw()			= 0;
		virtual void AfterDraw()	= 0;
		void ApplyScene();
		
		void Run()
		{
			++CurrentUpdateID;
			ENGINE_PROFILE_PUSH("PreUpdate");
			PreUpdate();
			ENGINE_PROFILE_POP();
			ENGINE_PROFILE_PUSH("BeforeDraw");
			BeforeDraw();
			ENGINE_PROFILE_POP();
			ENGINE_PROFILE_PUSH("Draw");
			Draw();
			ENGINE_PROFILE_POP();
			ENGINE_PROFILE_PUSH("AfterDraw");
			AfterDraw();
			ENGINE_PROFILE_POP();
			ENGINE_PROFILE_PUSH("ApplyScene");
			ApplyScene();
			ENGINE_PROFILE_POP();
			timmy.Tick();
			Delta = timmy.DeltaTime();
			//DINFO("Frame time: " + std::to_wstring(timmy.DeltaTime() * 1000.f) + L"ms");
		}

		virtual void OnResize(int width, int height) = 0;
		virtual void OnMinimize() = 0;
		virtual void OnMaximize() = 0;
		virtual void OnDeMaximize() = 0;
		virtual void OnDpiChanged();
		
		virtual Stringy GetName() const = 0;

		Requests::Requester Req;
		InputAttach::AttachmentManager InpAtt;
		Stringy WorkingDir;
		GameTimer timmy;
		double Delta;
		double TargetUpdateInterval;
		CommonResources Resources;
		std::random_device m_RandomDevice;
		std::mt19937_64 m_RandomGen;
	private:
		Scene::IScene *toSwitch = nullptr;
		bool NeedsQuit = false;
	protected:				
#ifdef EC_PROFILE
		ProfileMcGee m_Profiler;
#endif
		unsigned int CurrentUpdateID = 1;

		virtual void PreUpdate() {};
	};

	struct IWindowEngine : IEngine
	{
		IWindowEngine();
		virtual ~IWindowEngine() noexcept { CurrentScene = nullptr; }

		virtual void OnResize(int width, int height) override;
		virtual void OnMinimize() override;
		virtual void OnMaximize() override;
		virtual void OnDeMaximize() override;
				
	protected:
		void PreUpdate() override;

		SDLW::Window Win;
		GLRen Ren;
		std::unique_ptr<Config::IConfigThrone> Config;
		Config1::UIConfig UIC;
		float HalfWinWidth{ 0.f }, HalfWinHeight{ 0.f };
		double DpiScale = 1.0;
		double InvDpiScale = 1.0;
	};
}

