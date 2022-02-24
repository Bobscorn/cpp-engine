#pragma once

#include "Helpers/DebugHelper.h"

#include <string>


// Essentially encapsulation allowing for other 'modules' and classes to program without having to use operating system specific things
namespace Interaction
{
	struct IInteractor
	{
		virtual ~IInteractor() {}

		virtual std::wstring SaveDialogue() = 0;
		virtual std::wstring OpenDialogue() = 0;

		virtual std::wstring ReadFile(std::wstring filepath) = 0;
		virtual Debug::DebugReturn WriteFile(std::wstring filepath, std::wstring data) = 0;
	};
}