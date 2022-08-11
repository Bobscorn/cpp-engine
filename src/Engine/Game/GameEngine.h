#include "Systems/Execution/Engine.h"

#include "Systems/Requests/Requestable.h"
#include "Systems/Audio/Audio.h"

#include "Game/Scene/StartingScene.h"

namespace Engine
{
	struct GameEngine : public IWindowEngine, public Requests::IRequestable
	{
		GameEngine();
		virtual ~GameEngine() {};
		
		virtual void BeforeDraw() override;
		virtual void Draw() override;
		virtual void AfterDraw() override;
		
		Debug::DebugReturn Request(Requests::Request &action) override;
		inline Stringy GetName() const override { return "GameEngine instance"; }
	protected:

		std::unique_ptr<Audio::Manager> m_Audio;
	};
}