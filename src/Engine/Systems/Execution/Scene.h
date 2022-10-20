#pragma once

#include "Helpers/DebugHelper.h"
#include "Systems/Events/EventsBase.h"

#include <memory>

class Camera;
struct CommonResources;

namespace Scene
{
	struct IScene;

	//struct SceneManager
	//{
	//	SizeU WindowSize;
	//	PointU Center;
	//	DOUBLE2 InverseSize;
	//	double DpiScale;
	//	double InverseDpiScale;
	//	double AspectRatio;
	//	PointU MousePos;
	//	bool MouseChanged{ false };
	//	//bool DrawOutline;  Not implemented wtf was I doing with this?

	//	virtual void SwitchScene(Scene::IScene * scene) = 0;

	//protected:
	//	std::unique_ptr<Scene::IScene>	CurrentScene;
	//};


	struct IScene : public Listener::IGEventListener
	{
		virtual Debug::DebugReturn Initialize() = 0;

		virtual bool Receive(Events::IEvent *event) override = 0;

		virtual void BeforeDraw() = 0;
		virtual void Draw() = 0;
		virtual void AfterDraw() = 0;

		// Whether or not to continue running this scene if a scene change is attempted, but fails (if this scene was running before the scene change attempt)
		inline virtual bool ContinueIfFail() { return false; }

		// Called when this scene was continued after failing to change to a different one (nothing should have changed, but maybe a notification would be nice)
		virtual void WasContinued(CommonResources *resources) { (void)resources; }

		// Returns loose pointer
		virtual std::unique_ptr<IScene> Clone() = 0;

		virtual ~IScene() {};
	};
}