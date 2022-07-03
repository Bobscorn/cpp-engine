#pragma once

#include "Image.h"

#include <array>

namespace Drawing
{
	struct CubeMapTexture : GLImage
	{
	protected:
		std::array<SimpleSurface, 6> _cpuTexs; // Order: px, nx, py, ny, pz, nz
		
		GLuint _width, _height;
	public:
		CubeMapTexture();
		CubeMapTexture(CubeMapTexture&& other);
		CubeMapTexture(const SimpleSurface& px, const SimpleSurface& nx, const SimpleSurface& py, const SimpleSurface& ny, const SimpleSurface& pz, const SimpleSurface& nz);
		~CubeMapTexture();
		
		CubeMapTexture(const CubeMapTexture&) = delete;

		CubeMapTexture& operator=(const CubeMapTexture&) = delete;
		
		CubeMapTexture& operator=(CubeMapTexture&& other);
		
		void Set(const SimpleSurface& px, const SimpleSurface& nx, const SimpleSurface& py, const SimpleSurface& ny, const SimpleSurface& pz, const SimpleSurface& nz);
		
		virtual void Destroy() override;
		
		void LoadGL() override;
		void UnloadGL();

		inline bool HasLoadedGL() const { return _tex != 0; }

		inline virtual GLenum GetTarget() const override { return GL_TEXTURE_CUBE_MAP; }
	};
}