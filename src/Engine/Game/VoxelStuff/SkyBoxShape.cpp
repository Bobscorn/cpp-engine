#include "SkyBoxShape.h"

#include "Drawing/GLRen2.h"

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

SkyBoxShape::SkyBoxShape(G1::IShapeThings tings, std::string skybox_prefix, std::string extension)
	: SkyBoxShape(std::move(tings), skybox_prefix + "_px" + extension, skybox_prefix + "_nx" + extension, skybox_prefix + "_py" + extension, skybox_prefix + "_ny" + extension, skybox_prefix + "_pz" + extension, skybox_prefix + "_nz" + extension)
{
}

SkyBoxShape::SkyBoxShape(G1::IShapeThings tings, std::string px_img, std::string nx_img, std::string py_img, std::string ny_img, std::string pz_img, std::string nz_img)
	: G1::IShape(tings)
	, FullResourceHolder(tings.Resources)
	, _mesh(std::make_shared<Drawing::Mesh>(Drawing::RawMesh{ Drawing::VertexData::FromGeneric(Drawing::PositionOnly3DDesc, SkyboxVertices.begin(), SkyboxVertices.end()), SkyboxIndices }, Drawing::MeshStorageType::DEDICATED_BUFFER))
	, _mat(std::make_shared<Drawing::Material>(Drawing::SerializableMaterial{ "skybox", "skybox-prog", {}, {} }.ToMaterial()))
	, _trans(std::make_shared<Matrixy4x4>(Matrixy4x4::Identity()))
	, _drawCall(mResources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2(_mesh, _mat, _trans, "Skybox shape", true)))
{
	auto px = Drawing::SDLImage::LoadSurface(px_img);
	auto nx = Drawing::SDLImage::LoadSurface(nx_img);
	auto py = Drawing::SDLImage::LoadSurface(py_img);
	auto ny = Drawing::SDLImage::LoadSurface(ny_img);
	auto pz = Drawing::SDLImage::LoadSurface(pz_img);
	auto nz = Drawing::SDLImage::LoadSurface(nz_img);

	if (px && nx && py && ny && pz && nz)
	{
		_tex = std::make_shared<Drawing::CubeMapTexture>(px, nx, py, ny, pz, nz);
	}

	_mat->Textures["diffuse"] = Drawing::TextureReference{ _tex };
}

void SkyBoxShape::BeforeDraw()
{
	*_trans = Matrixy4x4::Translate(Container->GetCamera()->GetPosition());
}
