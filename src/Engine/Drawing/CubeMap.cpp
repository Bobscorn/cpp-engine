#include "CubeMap.h"

namespace Drawing
{
	GLCubeMap::GLCubeMap(GLuint width, GLuint height, GLenum internalFormat)
		: GLImage(0, GL_TEXTURE_CUBE_MAP)
		, _width(width)
		, _height(height)
	{
		CHECK_GL_ERR("Before Creating Cube Map");
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_tex);
		CHECK_GL_ERR("Creating Cube Map");

		glTextureStorage2D(_tex, 1, internalFormat, _width, _height);
		CHECK_GL_ERR("Allocating storage for Cube Map");

		glTextureParameteri(_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(_tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(_tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
		CHECK_GL_ERR("Setting Cube Map parameters");
	}

	CubeMapTexture::CubeMapTexture()
		: GLImage()
		, _width(0)
		, _height(0)
	{
	}

	CubeMapTexture::CubeMapTexture(CubeMapTexture&& other)
		: GLImage(other.Release())
		, _cpuTexs(std::move(other._cpuTexs))
		, _width(other._width)
		, _height(other._height)
	{
		other._width = 0;
		other._height = 0;
	}

	CubeMapTexture::CubeMapTexture(const SimpleSurface& px, const SimpleSurface& nx, const SimpleSurface& py, const SimpleSurface& ny, const SimpleSurface& pz, const SimpleSurface& nz)
		: GLImage()
		, _cpuTexs()
		, _width(px ? px->w : 0)
		, _height(px ? px->h : 0)
	{
		if (!px || !nx || !py || !ny || !pz || !nz)
			throw std::invalid_argument("CubeMapTexture: One or more surfaces are null");

		auto indexToFace = [&px, &nx, &py, &ny, &pz, &nz](int index) { switch (index) { default: case 0: return px.Get(); case 1: return nx.Get(); case 2: return py.Get(); case 3: return ny.Get(); case 4: return pz.Get(); case 5: return nz.Get(); }; };
		
		SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
		for (int i = 0; i < 6; ++i)
		{
			_cpuTexs[i] = SDL_ConvertSurface(indexToFace(i), format, 0);
			SDL_SetSurfaceAlphaMod(_cpuTexs[i].Get(), 0);
			SDL_SetSurfaceBlendMode(_cpuTexs[i].Get(), SDL_BLENDMODE_NONE);
		}
		SDL_FreeFormat(format);
		format = nullptr;
		
		for (auto& t : _cpuTexs)
			if ((GLuint)t->w != _width || (GLuint)t->h != _height)
				throw std::runtime_error("CubeMapTexture: One or more surfaces have different dimensions");
	}
	
	CubeMapTexture::~CubeMapTexture()
	{
	}
	
	CubeMapTexture& CubeMapTexture::operator=(CubeMapTexture&& other)
	{
		GLImage::Reset(other.Release());
		_cpuTexs = std::move(other._cpuTexs);
		_width = other._width;
		_height = other._height;
		other._width = 0;
		other._height = 0;
		return *this;
	}

	void CubeMapTexture::Set(const SimpleSurface& px, const SimpleSurface& nx, const SimpleSurface& py, const SimpleSurface& ny, const SimpleSurface& pz, const SimpleSurface& nz)
	{

		auto indexToFace = [&px, &nx, &py, &ny, &pz, &nz](int index) 
		{ 
			switch (index)
			{
			default:
			case 0: return px.Get(); case 1: return nx.Get(); case 2: return py.Get(); case 3: return ny.Get(); case 4: return pz.Get(); case 5: return nz.Get();
			};
		};

		for (int i = 0; i < 6; ++i)
			if (!indexToFace(i))
				return;

		bool redo = (GLuint)px->w != _width || (GLuint)px->h != _height;

		GLuint new_width = px->w;
		GLuint new_height = px->h;
		for (int i = 0; i < 6; ++i)
			if ((GLuint)indexToFace(i)->w != new_width || (GLuint)indexToFace(i)->h != new_height)
			{
				DERROR("Given surfaces of different sizes!");
				return;
			}

		_width = new_width;
		_height = new_height;

		if (redo)
		{
			SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
			for (int i = 0; i < 6; ++i)
			{
				_cpuTexs[i] = SDL_ConvertSurface(indexToFace(i), format, 0);
				SDL_SetSurfaceAlphaMod(_cpuTexs[i].Get(), 0);
				SDL_SetSurfaceBlendMode(_cpuTexs[i].Get(), SDL_BLENDMODE_NONE);
			}
			SDL_FreeFormat(format);
		}

		if (!redo)
			for (int i = 0; i < 6; ++i)
			{
				SDL_BlitSurface(indexToFace(i), nullptr, _cpuTexs[i].Get(), nullptr);
			}
		
		if (redo && HasLoadedGL())
		{
			GLImage::Reset();
			LoadGL();
		}
		else if (HasLoadedGL())
		{
			glTextureSubImage3D(_tex, 0, 0, 0, 0, (GLsizei)_width, (GLsizei)_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)px->pixels);
			glTextureSubImage3D(_tex, 0, 0, 0, 1, (GLsizei)_width, (GLsizei)_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)nx->pixels);
			glTextureSubImage3D(_tex, 0, 0, 0, 2, (GLsizei)_width, (GLsizei)_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)py->pixels);
			glTextureSubImage3D(_tex, 0, 0, 0, 3, (GLsizei)_width, (GLsizei)_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)ny->pixels);
			glTextureSubImage3D(_tex, 0, 0, 0, 4, (GLsizei)_width, (GLsizei)_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pz->pixels);
			glTextureSubImage3D(_tex, 0, 0, 0, 5, (GLsizei)_width, (GLsizei)_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)nz->pixels);
		}
	}
	
	void CubeMapTexture::Destroy()
	{
		for (int i = 0; i < _cpuTexs.size(); ++i)
		{
			_cpuTexs[i].Delete();
		}
		_width = 0;
		_height = 0;
	}
	
	void CubeMapTexture::LoadGL()
	{
		if (HasLoadedGL())
			return;

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &_tex);
		
		glTextureParameteri(_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		
		glTextureStorage2D(_tex, 1, GL_RGBA8, _width, _height);

		glBindTexture(GL_TEXTURE_CUBE_MAP, _tex);
		/*glTextureSubImage3D(_tex, 0, 0, 0, 0, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTexs[0]->pixels);
		glTextureSubImage3D(_tex, 0, 0, 0, 1, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTexs[1]->pixels);
		glTextureSubImage3D(_tex, 0, 0, 0, 2, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTexs[2]->pixels);
		glTextureSubImage3D(_tex, 0, 0, 0, 3, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTexs[3]->pixels);
		glTextureSubImage3D(_tex, 0, 0, 0, 4, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTexs[4]->pixels);
		glTextureSubImage3D(_tex, 0, 0, 0, 5, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_cpuTexs[5]->pixels);*/
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _cpuTexs[0]->pixels);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _cpuTexs[2]->pixels);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _cpuTexs[4]->pixels);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _cpuTexs[1]->pixels);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _cpuTexs[3]->pixels);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _cpuTexs[5]->pixels);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void CubeMapTexture::UnloadGL()
	{
		if (!HasLoadedGL())
			return;
		GLImage::Reset();
	}
}