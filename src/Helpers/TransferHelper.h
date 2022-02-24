#pragma once

#include "VectorHelper.h"
#include "DebugHelper.h"

#include <string>
#include <random>

struct SDL_Window;

#ifdef EC_PROFILE
struct ProfileMcGee;
#endif EC_PROFILE
struct SDLRen2D;
struct IRen2D;
struct IRen3D;

namespace Particles
{
	struct IParticleDrawer;
}

namespace Events
{
	class EventManager;
}
namespace Scene
{
	struct SceneManager;
}
namespace Config1
{
	struct UIConfig;
}

namespace Requests
{
	class Requester;
}

namespace Interaction
{
	struct IInteractor;
}

namespace InputAttach
{
	class AttachmentManager;
}

namespace Config
{
	class IConfigThrone;
}

struct CommonResources
{
#ifdef EC_PROFILE
	ProfileMcGee						*Profile;
#endif
	IRen2D								*Ren2;
	IRen3D								*Ren3;
	Particles::IParticleDrawer			*Particles;
	SDL_Window							*Window;
	Events::EventManager				*Event;
	Scene::SceneManager					*Scene;
	Requests::Requester					*Request;
	InputAttach::AttachmentManager		*InputAttachment;
	Config1::UIConfig					*UIConfig;
	Config::IConfigThrone				*Config;
	Stringy								WorkingDirectory;
	std::random_device					*RandomDevice;
	std::mt19937_64						*RandomGen;
	const int							*WindowWidth;
	const int							*WindowHeight;
	const float							*HalfWindowWidth;
	const float							*HalfWindowHeight;
	const float							*InverseWindowWidth;
	const float							*InverseWindowHeight;
	const int							*CenterX;
	const int							*CenterY;
	const double						*DpiScale;
	const double						*InverseDpiScale;
	const double						*DeltaTime;
	const double						*TargetUpdateInterval; // Used by physics
	const float							*AspectRatio;
	const unsigned int					*UpdateID;
	const PointU						*MousePosition; // Polled every frame
	const bool							*WindowFocused;
	const bool							*MouseChangedPosition;
};


struct FullResourceHolder
{
protected:
	CommonResources *mResources;
public:
	FullResourceHolder(CommonResources *resources = nullptr) : mResources(resources) {}
	virtual ~FullResourceHolder() {}

	inline void SetResources(CommonResources *resources) { mResources = resources; }
	inline CommonResources *GetResources() const { return mResources; }
};