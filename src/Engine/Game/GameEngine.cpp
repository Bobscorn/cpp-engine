#include "GameEngine.h"

#ifndef CPP_ENGINE_TESTS
#include "Scene/StartingScene.h"
#include "Scene/VoxelScene.h"

#include "Drawing/Program.h"
#include "Drawing/Material.h"
#include "Drawing/VoxelStore.h"
#include "Drawing/Texture.h"
#include "Drawing/GeometryStore.h"

#include "Drawing/GeometryGeneration.h"

#include <iostream>

Engine::GameEngine::GameEngine() : IWindowEngine() 
{ 
	Drawing::VertexBuffer::InitializeStaticBuffer();
	Drawing::MaterialStore::InitializeStore("Materials");
	Drawing::ProgramStore::InitializeStore("Programs");
	Drawing::TextureStore::InitializeStore("Textures");
	Voxel::VoxelStore::InitializeVoxelStore("PreStitched not implemented", "Blocks", "Textures", "Meshes");

	// Generate a UV Sphere
	auto sphereMesh = GeoGen::GeometryGenerator::CreateSphere(0.5f, 16, 6);
	auto convertedMesh = Drawing::RawMesh{ Drawing::VertexData::ConvertGeometry(Drawing::VertexData::FromGeneric(Drawing::LegacyVertexDesc, sphereMesh.Vertices.begin(), sphereMesh.Vertices.end()), Drawing::Full3DVertexDesc), std::move(sphereMesh.Indices) };

	Drawing::GeometryStore::InitializeStore("Geometry Loading not implemented yet", { Drawing::RawMeshData{ "default-cube", Drawing::MeshStorageType::STATIC_BUFFER, Drawing::CreateCubeMesh()}, Drawing::RawMeshData{ "default-sphere", Drawing::MeshStorageType::STATIC_BUFFER, std::move(convertedMesh) } });

	//SwitchScene(new Voxel::VoxelScene(&Resources)); 
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
	else if (action.Name == "ReloadMaterials")
	{
		Drawing::MaterialStore::Instance().Reload("Materials");
		return true;
	}
	else if (action.Name == "ReloadMeshes")
	{
		DINFO("Reloading voxel meshes...");
		Voxel::VoxelStore::ReloadMeshes("PreStitched not implemented", "Blocks", "Textures", "Meshes");
		return true;
	}
	return false;
}
#endif

#ifdef CPP_ENGINE_TESTS
#include <iostream>

int GameEngine(int argc, char* argv[])
{
	std::cout << "GameEngine Test!" << std::endl;

	return 0;
}
#endif