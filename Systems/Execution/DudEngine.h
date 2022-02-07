#pragma once

#include "Engine.h"

namespace Engine
{
	struct DudEngine : public Engine::IEngine
	{
		DudEngine() : IEngine() {}

		// Inherited via IEngine
		virtual void OnResize(int w, int h) override;
		virtual void OnMinimize() override;
		virtual void OnMaximize() override;
		virtual void OnDeMaximize() override;
		virtual void BeforeDraw() override;
		virtual void Draw() override;
		virtual void AfterDraw() override;

		virtual Stringy GetName() const override { return "A DudEngine instance"; }
	};
}