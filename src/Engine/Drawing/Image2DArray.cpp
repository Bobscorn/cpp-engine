#include "Image2DArray.h"

namespace Drawing
{
    SDL_Surface* Image2DArray::CreateSurface()
    {
        auto out = SDL_CreateRGBSurfaceWithFormat(0, _width, _height, 32, SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGBA32);
        SDL_SetSurfaceAlphaMod(out, 0);
        SDL_SetSurfaceBlendMode(out, SDL_BLENDMODE_NONE);

        return out;
    }

    void Image2DArray::VerifySurfaceFormats()
    {
        for (auto& surf : _cpuSurfaces)
        {
            VerifySurfaceIsFormat(&surf, SDL_PIXELFORMAT_RGBA32);
        }
    }

    Image2DArray::Image2DArray()
        : GLImage()
        , _cpuSurfaces()
        , _width(0)
        , _height(0)
    {
    }

    Image2DArray::Image2DArray(GLsizei width, GLsizei height, size_t initialLayers)
        : GLImage()
        , _cpuSurfaces(initialLayers, nullptr, std::allocator<SDL_Surface*>())
        , _width(width)
        , _height(height)
    {
    }

    Image2DArray::Image2DArray(std::vector<SDL_Surface*>&& surfaces)
        : GLImage()
        , _cpuSurfaces(std::move(surfaces))
        , _width(_cpuSurfaces.size() && _cpuSurfaces[0] ? _cpuSurfaces[0]->w : 0)
        , _height(_cpuSurfaces.size() && _cpuSurfaces[0] ? _cpuSurfaces[0]->h : 0)
    {
        VerifySurfaceFormats();
    }

    Image2DArray::Image2DArray(Image2DArray&& other)
        : GLImage(std::forward<GLImage>(other))
        , _cpuSurfaces(std::move(other._cpuSurfaces))
        , _width(other._width)
        , _height(other._height)
    {
        other._tex = 0;
        other._width = 0;
        other._height = 0;
    }

    Image2DArray::~Image2DArray()
    {
        Reset();
    }

    void Image2DArray::Destroy()
    {
        GLImage::Destroy();

        for (int i = 0; i < _cpuSurfaces.size(); ++i)
        {
            SDL_FreeSurface(_cpuSurfaces[i]);
        }

        _cpuSurfaces.clear();
        _width = 0;
        _height = 0;
    }

    Image2DArray& Image2DArray::operator=(Image2DArray&& other)
    {
        Image2DArray::Destroy();

        _cpuSurfaces = std::move(other._cpuSurfaces);

        _tex = other._tex;
        _width = other._width;
        _height = other._height;

        other._tex = 0;
        other._width = 0;
        other._height = 0;

        return *this;
    }

    void Image2DArray::SetSurface(SDL_Surface* surf, int targetLayer)
    {
        if (!surf)
            return;
        if (targetLayer < 0 || targetLayer >= _cpuSurfaces.size())
            return;

        if (surf->w != _width ||
            surf->h != _height)
            return;

        if (!_cpuSurfaces[targetLayer])
        {
            _cpuSurfaces[targetLayer] = CreateSurface();
        }

		SimpleSurface tmp = SDL_ConvertSurface(surf, _cpuSurfaces[targetLayer]->format, 0);
        SDL_BlitSurface(tmp.Get(), nullptr, _cpuSurfaces[targetLayer], nullptr);
    }

    void Image2DArray::SetArea(SDL_Surface* src, SDL_Rect dstRect, int targetLayer)
    {
        if (!src)
            return;
        SetArea(src, SDL_Rect{ 0, 0, src->w, src->h }, dstRect, targetLayer);
    }

    void Image2DArray::SetArea(SDL_Surface* src, SDL_Rect srcRect, SDL_Rect dstRect, int targetLayer)
    {
        if (!src)
            return;
        if (targetLayer < 0 || targetLayer >= _cpuSurfaces.size())
            return;

        if (src->w > _width || src->h > _height)
            return;

        if (!_cpuSurfaces[targetLayer])
        {
            _cpuSurfaces[targetLayer] = CreateSurface();
        }
		
        SimpleSurface tmp = SDL_ConvertSurface(src, _cpuSurfaces[targetLayer]->format, 0);
        SDL_BlitSurface(tmp.Get(), &srcRect, _cpuSurfaces[targetLayer], &dstRect);

        if (HasLoadedGL())
        {
            glTextureSubImage3D(_tex, 0, 0, 0, targetLayer, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, _cpuSurfaces[targetLayer]->pixels);
        }
    }

    void Image2DArray::AddLayer()
    {
        bool wasLoaded = HasLoadedGL();
        if (wasLoaded)
            UnLoadGL();

        _cpuSurfaces.emplace_back(CreateSurface());

        if (wasLoaded)
        {
            LoadGL();
        }
    }

    void Image2DArray::EnsureCPU()
    {
        // TODO: this
    }

    void Image2DArray::LoadGL()
    {
        if (HasLoadedGL())
            return;

        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &_tex);

        glTextureStorage3D(_tex, 2, GL_RGBA8, _width, _height, (GLsizei)_cpuSurfaces.size());

        glTextureParameteri(_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureParameteri(_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTextureParameteri(_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        CHECK_GL_ERR("Setting Texture2DArray parameters");

        for (int i = 0; i < _cpuSurfaces.size(); ++i)
            glTextureSubImage3D(_tex, 0, 0, 0, i, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, _cpuSurfaces[i] ? _cpuSurfaces[i]->pixels : nullptr);
    }

    void Image2DArray::UnLoadGL()
    {
        if (!HasLoadedGL())
            return;

        glDeleteTextures(1, &_tex);

        _tex = 0;
    }

    void Image2DArray::GenerateMipmaps()
    {
        LoadGL();
        if (HasLoadedGL())
            glGenerateTextureMipmap(_tex);
    }

    bool Image2DArray::HasLoadedGL()
    {
        return _tex;
    }

    void VerifySurfaceIsFormat(SDL_Surface** surf, Uint32 format)
    {
        if (!surf || !*surf)
            return;

        auto* real_surf = *surf;

        auto surf_format = SDL_MasksToPixelFormatEnum(real_surf->format->BitsPerPixel, real_surf->format->Rmask, real_surf->format->Gmask, real_surf->format->Bmask, real_surf->format->Amask);
        if (surf_format != format)
        {
            auto* format_data = SDL_AllocFormat(format);
            auto* new_surf = SDL_ConvertSurface(real_surf, format_data, 0);
            SDL_FreeSurface(real_surf);
            *surf = new_surf;
        }
    }
}