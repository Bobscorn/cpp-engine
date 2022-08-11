#pragma once

#include "Drawing/Graphics1.h"
#include "Drawing/DrawCallReference.h"
#include "Drawing/CubeMap.h"

#include <string>

namespace Drawing
{
	class Mesh;
	struct Material;
}

struct SkyBoxShape : G1::IShape
{
	SkyBoxShape(G1::IShapeThings tings, std::string skybox_prefix, std::string extension); // Looks for textures [skybox_prefix]_px[extension] [skybox_prefix]_nx[extension] [skybox_prefix]_py[extension] [skybox_prefix]_ny[extension] [skybox_prefix]_pz[extension] [skybox_prefix]_nz[extension]
	SkyBoxShape(G1::IShapeThings tings, std::string px_img, std::string py_img, std::string pz_img, std::string nx_img, std::string ny_img, std::string nz_img);
	~SkyBoxShape() = default;
	
	virtual void BeforeDraw() override;
	inline virtual void AfterDraw() override {};

protected:

	std::shared_ptr<Drawing::CubeMapTexture> _tex;
	std::shared_ptr<Drawing::Mesh> _mesh;
	std::shared_ptr<Drawing::Material> _mat;
	std::shared_ptr<Matrixy4x4> _trans;
	Drawing::DrawCallReference _drawCall;
};