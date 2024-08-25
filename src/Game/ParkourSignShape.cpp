#include "ParkourSignShape.h"

#include <Drawing/GLRen2.h>
#include <Drawing/Texture.h>
#include <Drawing/TextDrawing.h>
#include <Drawing/GeometryStore.h>
#include <Game/VoxelStuff/VoxelChunk.h>
#include <Game/VoxelStuff/VoxelWorld.h>

const std::string Parkour::ParkourSignShape::SignMeshName = "parkour-sign-mesh";
const std::string Parkour::ParkourSignShape::SignTextureName = "parkour-sign-texture";
const std::string Parkour::ParkourSignShape::SignMaterialName = "parkour-sign-material";

Parkour::ParkourSignShape::ParkourSignShape(G1::IShapeThings things, Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos, std::shared_ptr<Drawing::Mesh> signMesh)
	: FullResourceHolder(things.Resources)
	, G1::IShape(things)
	, Voxel::ICube(world, chunk, pos)
	, m_SignTextMesh(signMesh)
{
	std::shared_ptr<Drawing::GLImage> drawImage;
	if (!Drawing::TextureStore::Instance().TryGetTexture(SignTextureName, drawImage))
	{
		// Generate image
		auto tmpImage = std::make_shared<TextDrawing>(mResources, "This Way", "Normal", "Text");

		Drawing::TextureStore::Instance().AddTexture(SignTextureName, tmpImage);
	}

	if (auto& mat = Drawing::MaterialStore::Instance().SetMaterial(SignMaterialName); !mat)
	{
		std::unordered_map<std::string, Drawing::TextureReference> textures{};
		textures["diffuse"] = SignTextureName;
		std::unordered_map<std::string, Drawing::MaterialProperty> props{};
		props["diffuse"] = Drawing::MaterialProperty::from("diffuse", floaty4{ 1.f, 1.f, 1.f, 1.f }, Drawing::IntToMatSize(4));
		props["tex-bitmap"] = Drawing::MaterialProperty::from(std::string("tex-bitmap"), BIT_DIFFUSE, Drawing::IntToMatSize(4), Drawing::PropertyType::INT);
		mat = std::make_shared<Drawing::Material>(Drawing::ProgramReference::ProgramReference("Default3D"), std::move(textures), std::move(props));
	}

	// Get the aspect ratio of the text image
	float ratio;
	{
		auto* stored_tex = Drawing::TextureReference(SignTextureName).GetTexture().get();
		if (!stored_tex)
		{
			DERROR("Did not find ParkourSignShape's sign texture in the texture store!");
			ratio = 1.f;
		}
		else
		{
			if (auto* sdl_img = dynamic_cast<Drawing::SDLImage*>(stored_tex); sdl_img)
			{
				float width = sdl_img->GetWidth(), height = sdl_img->GetHeight();
				if (width == 0 || height == 0)
				{
					DERROR("Could not grab texture ratio from the SDLImage! One of the dimensions was zero!");
					ratio = 1.f;
				}
				else
					ratio = (float)width / (float)height;
			}
			else
			{
				DWARNING("Parkour Sign texture was not an SDLImage, calculating ratio from the GL texture dimensions");
				GLuint gl_tex = stored_tex->Get();
				GLint width, height;
				glGetTextureParameteriv(gl_tex, GL_TEXTURE_WIDTH, &width);
				glGetTextureParameteriv(gl_tex, GL_TEXTURE_HEIGHT, &height);

				if (width == 0 || height == 0)
				{
					DERROR("Could not grab texture ratio from the GL texture! One of the dimensions was zero!");
					ratio = 1.f;
				}
				else
					ratio = (float)width / (float)height;
			}
		}
	}

	if (!m_SignTextMesh)
	{
		// Create the mesh to render the text (just a quad)

		// A hardcoded value determined by the width and height of the actual sign mesh
		const float sign_ratio = 1.5f;

		const float max_height = 0.4;
		const float max_width = 0.6;

		float width, height;
		if (ratio > sign_ratio)
		{
			width = max_width;
			height = max_width / ratio;
		}
		else
		{
			width = max_height * ratio;
			height = max_height;
		}
		float half_width = width * 0.5f;
		float half_height = height * 0.5f;
		float z_offset = 0.051f;

		const floaty3 top_left = floaty3(-half_width, half_height, z_offset);
		const floaty3 top_right = floaty3(half_width, half_height, z_offset);
		const floaty3 bottom_left = floaty3(-half_width, -half_height, z_offset);
		const floaty3 bottom_right = floaty3(half_width, -half_height, z_offset);

		const floaty3 normal = floaty3(0.f, 0.f, 1.f);
		const floaty3 binormal = floaty3(1.f, 0.f, 0.f);
		const floaty3 tangent = floaty3(0.f, 1.f, 0.f);

		const Drawing::Full3DVertex vertices[] = {
			Drawing::Full3DVertex {
				top_left,
				normal,
				binormal,
				tangent,
				floaty2(0.f, 0.f)
			},
			Drawing::Full3DVertex {
				top_right,
				normal,
				binormal,
				tangent,
				floaty2(1.f, 0.f)
			},
			Drawing::Full3DVertex {
				bottom_left,
				normal,
				binormal,
				tangent,
				floaty2(0.f, 1.f)
			},
			Drawing::Full3DVertex {
				bottom_right,
				normal,
				binormal,
				tangent,
				floaty2(1.f, 1.f)
			},
		};

		const unsigned int indices[] = { 0, 2, 1, 1, 2, 3 };

		DINFO("Created Parkour Sign Text Mesh");
		auto raw_mesh = Drawing::RawMesh{ Drawing::VertexData::FromFull3DVertices(vertices, vertices + 4), std::vector<unsigned int>(indices, indices + 6) };
		m_SignTextMesh = std::make_shared<Drawing::Mesh>(raw_mesh, Drawing::MeshStorageType::STATIC_BUFFER);
	}
}

void Parkour::ParkourSignShape::OnLoaded()
{
	floaty3 real_pos = m_World->GetPhysPosFromBlockCoord(Voxel::BlockCoord{ m_Chunk->GetCoord(), m_Pos });
	auto rot = m_Chunk->get_data(m_Pos).Data.Rotation;
	auto axis = rot.asBt().getAxis();
	auto angle = rot.asBt().getAngle();
	auto rot_matrix = Matrixy4x4::RotationAxisR((floaty3)axis, angle);
	auto translate_matrix = Matrixy4x4::Translate(real_pos);
	auto final_mat = Matrixy4x4::Multiply(translate_matrix, rot_matrix);
	std::shared_ptr<Matrixy4x4> mat = std::make_shared<Matrixy4x4>(final_mat);

	m_SignTextDrawCall = mResources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ m_SignTextMesh, Drawing::MaterialReference(SignMaterialName).GetMaterial(), std::move(mat), "Parkour Sign Text" });
}

void Parkour::ParkourSignShape::OnUnloaded()
{
	m_SignTextDrawCall.Reset();
}

std::unique_ptr<Voxel::ICube> Parkour::ParkourSignShape::Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const
{
	auto clone = std::make_unique<ParkourSignShape>(G1::IShapeThings{ Container, mResources, "Parkour Sign Clone" }, world, chunk, pos);

	return clone;
}
