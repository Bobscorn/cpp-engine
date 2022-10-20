#include "SkyBoxShape.h"

#include "Drawing/GLRen2.h"

#include <filesystem>

std::vector<floaty3> SkyboxVertices =
{
	// NX Face
	{ -50.f, -50.f, -50.f },
	{ -50.f, +50.f, -50.f },
	{ -50.f, +50.f, +50.f },
	{ -50.f, -50.f, +50.f },
	
	// PX Face
	{ +50.f, -50.f, -50.f },
	{ +50.f, -50.f, +50.f },
	{ +50.f, +50.f, +50.f },
	{ +50.f, +50.f, -50.f },
	
	// NZ Face
	{ -50.f, -50.f, -50.f },
	{ +50.f, -50.f, -50.f },
	{ +50.f, +50.f, -50.f },
	{ -50.f, +50.f, -50.f },

	// PZ Face
	{ -50.f, -50.f, +50.f },
	{ -50.f, +50.f, +50.f },
	{ +50.f, +50.f, +50.f },
	{ +50.f, -50.f, +50.f },

	// NY Face
	{ -50.f, -50.f, -50.f },
	{ -50.f, -50.f, +50.f },
	{ +50.f, -50.f, +50.f },
	{ +50.f, -50.f, -50.f },
	
	// PY Face
	{ -50.f, +50.f, -50.f },
	{ +50.f, +50.f, -50.f },
	{ +50.f, +50.f, +50.f },
	{ -50.f, +50.f, +50.f }
};

std::vector<GLuint> SkyboxIndices =
{
	0, 1, 2,
	2, 3, 0,
	
	4, 5, 6,
	6, 7, 4,

	8, 9, 10,
	10, 11, 8,

	12, 13, 14,
	14, 15, 12,
	
	16, 17, 18,
	18, 19, 16,
	
	20, 21, 22,
	22, 23, 20
};

G1I::SkyBoxShape::SkyBoxShape(G1::IShapeThings tings, std::string skybox_prefix, std::string extension)
	: SkyBoxShape(std::move(tings), skybox_prefix + "_px" + extension, skybox_prefix + "_nx" + extension, skybox_prefix + "_py" + extension, skybox_prefix + "_ny" + extension, skybox_prefix + "_pz" + extension, skybox_prefix + "_nz" + extension)
{
}

G1I::SkyBoxShape::SkyBoxShape(G1::IShapeThings tings, std::string px_img, std::string nx_img, std::string py_img, std::string ny_img, std::string pz_img, std::string nz_img)
	: G1::IShape(tings)
	, FullResourceHolder(tings.Resources)
	, _mesh(std::make_shared<Drawing::Mesh>(Drawing::RawMesh{ Drawing::VertexData::FromGeneric(Drawing::PositionOnly3DDesc, SkyboxVertices.begin(), SkyboxVertices.end()), SkyboxIndices }, Drawing::MeshStorageType::DEDICATED_BUFFER))
	, _mat(std::make_shared<Drawing::Material>(Drawing::SerializableMaterial{ "skybox", "skybox-prog", {}, {} }.ToMaterial()))
	, _trans(std::make_shared<Matrixy4x4>(Matrixy4x4::Identity()))
	, _drawCall(mResources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2(_mesh, _mat, _trans, "Skybox shape", true)))
{
	auto p = std::filesystem::path("./Textures/");
	auto px = Drawing::SDLImage::LoadSurface((p / px_img).string());
	auto nx = Drawing::SDLImage::LoadSurface((p / nx_img).string());
	auto py = Drawing::SDLImage::LoadSurface((p / py_img).string());
	auto ny = Drawing::SDLImage::LoadSurface((p / ny_img).string());
	auto pz = Drawing::SDLImage::LoadSurface((p / pz_img).string());
	auto nz = Drawing::SDLImage::LoadSurface((p / nz_img).string());

	if (px && nx && py && ny && pz && nz)
	{
		_tex = std::make_shared<Drawing::CubeMapTexture>(px, nx, py, ny, pz, nz);
	}
	else
	{
		DERROR("One or more Skybox images were null!");
	}

	_mat->SetTexture("diffuse", Drawing::TextureReference{ _tex });
}

void G1I::SkyBoxShape::BeforeDraw()
{
	// Not sure if required
	*_trans = Matrixy4x4::Translate(Container->GetCamera()->GetPosition());
}
