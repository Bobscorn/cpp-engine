#include "GameEngine.h"

#include "Scene/StartingScene.h"
#include "Scene/VoxelScene.h"

#include "Drawing/Program.h"
#include "Drawing/Material.h"

#include <iostream>

Engine::GameEngine::GameEngine() : IWindowEngine() 
{ 
	Drawing::VertexBuffer::InitializeStaticBuffer();
	Drawing::MaterialStore::InitializeStore("Materials", std::vector<Drawing::SerializableMaterial>{ {Drawing::SerializableMaterial{ "Default3D", "Default3D", {}, { Drawing::MaterialProperty::from("diffuse", floaty4{ 0.5f, 0.5f, 0.5f, 1.f }, Drawing::MaterialSize::FOUR), Drawing::MaterialProperty::from("specular", floaty4{ 0.8f, 0.8f, 0.8f, 3.f }, Drawing::MaterialSize::FOUR) } }} });
	Drawing::ProgramStore::InitializeStore("Programs");

	SwitchScene(new Voxel::VoxelScene(&Resources)); 
	Req.Add(this); 

	try
	{
		m_Audio = std::make_unique<Audio::Manager>();
		Audio::ALErrorGuy guy{ "Testing Audio" };

		Audio::ALSource source = Audio::CreateSauce();
		Audio::ALBufferI test = Audio::CreateBufferFile("test.wav");
		test.AttachTo(source);
		source.Play();
	}
	catch (const Audio::ALException & e)
	{
		DERROR("Audio failure: " + e.what());
		DINFO("Continuing without audio");
	}
}

void Engine::GameEngine::BeforeDraw()
{
	if (Window_Focused)
		SDL_GetMouseState(&MousePos.x, &MousePos.y);

	if (CurrentScene)
		CurrentScene->BeforeDraw();
}

void Engine::GameEngine::Draw()
{
	if (CurrentScene)
		CurrentScene->Draw();
}

void Engine::GameEngine::AfterDraw()
{
	if (CurrentScene)
		CurrentScene->AfterDraw();

	Ren.Present(Win.Get());
}

extern void QuitDatAss();

Debug::DebugReturn Engine::GameEngine::Request(Requests::Request & action)
{
	if (action.Name == "FullQuit")
	{
		std::cout << "You are trying to completely quit the app, this will *NOT* save any data, type IWANTTOQUIT to continue" << std::endl;
		Stringy c;
		std::cin >> c;
		if (c == "IWANTTOQUIT")
			QuitDatAss();
		else
			std::cout << "You failed to type IWANTTOQUIT, cancelling quit" << std::endl;
		
		return true;
	}
	else if (action.Name == "FullQuitNoWarningNoSave" || action.Name == "ExitGame")
	{
		QuitDatAss();
		return true;
	}
	return false;
}
