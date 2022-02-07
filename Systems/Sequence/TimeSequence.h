#pragma once

#include "ActionSequence.h"

#include <Systems/Timer/Timer.h>
#include <Helpers/DebugHelper.h>
#include <Systems/Thing.h>

#include <vector>
#include <memory>

namespace Sequence
{
	struct ITimedSequenceElement;
	

	struct TimeSequence
	{
		void Resume();
		void Reset();
		void Pause(); // Calling pause will cause Apply to always return false regardless of whether its at the end

		bool Apply();
		inline double GetTime() const { return LastTime; }

		template<class T, typename ... Args>
		std::enable_if_t<std::is_base_of_v<ITimedSequenceElement, T>, T* const> Add(Args&&... args)
		{
			auto ptr = new T(std::forward<Args>(args)...);
			Elements.emplace_back(ptr);
			return ptr;
		}

	protected:
		unsigned int CurrentIndex{ 0u };
		std::vector<std::unique_ptr<ITimedSequenceElement>> Elements;
		GameTimer SequenceTimer;
		double LastTime{ 0.0 };
		double Multiplier{ 1.0 };
	};

	struct ITimedSequenceElement
	{
		virtual ~ITimedSequenceElement() {}

		virtual double Apply(double localtime) = 0; // Local time meaning time since this element was given control, returns whethers its finished or not
	};
}