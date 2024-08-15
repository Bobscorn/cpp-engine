#include "ParkourSignShape.h"

#include <Drawing/GLRen2.h>
#include <Drawing/Texture.h>
#include <Drawing/TextDrawing.h>
#include <Drawing/GeometryStore.h>
#include <Game/VoxelStuff/VoxelChunk.h>

const std::string Parkour::ParkourSignShape::SignMeshName = "parkour-sign-mesh";
const std::string Parkour::ParkourSignShape::SignTextureName = "parkour-sign-texture";
const std::string Parkour::ParkourSignShape::SignMaterialName = "parkour-sign-material";

Parkour::ParkourSignShape::ParkourSignShape(G1::IShapeThings things, Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos)
	: FullResourceHolder(things.Resources)
	, G1::IShape(things)
	, Voxel::ICube(world, chunk, pos)
{
	std::shared_ptr<Drawing::GLImage> drawImage;
	if (!Drawing::TextureStore::Instance().TryGetTexture(SignTextureName, drawImage))
	{
		constexpr unsigned imgWidth = 250;
		constexpr unsigned imgHeight = 100;
		// Generate image
		SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, imgWidth, imgHeight, 32, SDL_PIXELFORMAT_RGBA32);
		auto tmpImage = std::make_shared<TextDrawing>(mResources, "This Way", "Normal", "Text");
		//auto tmpImage = std::make_shared<Drawing::SDLImage>();

		Drawing::TextureStore::Instance().AddTexture(SignTextureName, tmpImage);
	}

	if (auto mat = Drawing::MaterialStore::Instance().SetMaterial(SignMaterialName); !mat)
	{
		std::unordered_map<std::string, Drawing::TextureReference> textures{};
		textures["diffuse"] = SignTextureName;
		std::unordered_map<std::string, Drawing::MaterialProperty> props{};
		props["diffuse"] = Drawing::MaterialProperty::from("diffuse", floaty4{ 1.f, 1.f, 1.f, 1.f }, Drawing::IntToMatSize(4));
		mat = std::make_shared<Drawing::Material>(Drawing::ProgramReference::ProgramReference("Default3D"), std::move(textures), std::move(props));
	}

	if (auto mesh = Drawing::GeometryStore::Instance().GetMesh(SignMeshName); !mesh)
	{
		DERROR("Parkour sign shape has no mesh loaded!");
	}
}

void Parkour::ParkourSignShape::OnLoaded()
{
	// TODO: figure out what this should do
}

void Parkour::ParkourSignShape::OnUnloaded()
{
	// TODO: figure out what this should do
}

std::unique_ptr<Voxel::ICube> Parkour::ParkourSignShape::Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const
{
	auto clone = std::make_unique<ParkourSignShape>(G1::IShapeThings{ Container, mResources, "Parkour Sign Clone" }, world, chunk, pos);
	
	auto rot = chunk->get_data(pos).Data.Rotation;
	auto axis = rot.asBt().getAxis();
	auto angle = rot.asBt().getAngle();
	std::shared_ptr<Matrixy4x4> mat = std::make_shared<Matrixy4x4>(Matrixy4x4::RotationAxisR((floaty3)axis, angle));
	clone->m_SignTextDrawCall = mResources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ SignMeshName, SignMaterialName, mat, "Parkour Sign Clone"});

	return clone;
}
