#pragma once

#include "Helpers/BulletHelper.h"

#include <LinearMath/btVector3.h>

namespace Voxel
{
	struct IInteractor
	{

	};

	struct IInteractable : BulletHelp::INothingInterface
	{
		virtual void OnInteract1(btVector3 hitNormal, btVector3 ray, IInteractor *interactor) = 0;
		virtual void OnInteract2(btVector3 hitNormal, btVector3 ray, IInteractor *interactor) = 0;
	};
}