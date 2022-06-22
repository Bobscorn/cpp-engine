#include "Image2DArray.h"

namespace Drawing
{
    SDL_Surface* Image2DArray::CreateSurface()
    {
        auto out = SDL_CreateRGBSurfaceWithFormat(0, _width, _height, 32, SDL_PixelFormatEnum::SDL_PIXELFORMAT_ARGB8888);
        SDL_SetSurfaceAlphaMod(out, 0);
        SDL_SetSurfaceBlendMode(out, SDL_BLENDMODE_NONE);

        return out;
    }

    Image2DArray::Image2DArray()
        : GLImage()
        , _cpuSurfaces()
        , _width(0)
        , _height(0)
    {
    }

    Image2DArray::Image2DArray(size_t width, size_t height, size_t initialLayers)
        : GLImage()
        , _cpuSurfaces(initialLayers, nullptr, std::allocator<SDL_Surface*>())
        , _width(width)
        , _height(height)
    {
    }

    Image2DArray::Image2DArray(std::vector<SDL_Surface*>&& surfaces)
        : GLImage()
        , _cpuSurfaces(std::move(surfaces))
        , _width(surfaces.size() && surfaces[0] ? surfaces[0]->w : 0)
        , _height(surfaces.size() && surfaces[0] ? surfaces[0]->h : 0)
    {
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

        SDL_BlitSurface(surf, nullptr, _cpuSurfaces[targetLayer], nullptr);
    }

    void Image2DArray::SetArea(SDL_Surface* src, SDL_Rect dstRect, int targetLayer)
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

        SDL_BlitSurface(src, nullptr, _cpuSurfaces[targetLayer], &dstRect);

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

        glTextureStorage3D(_tex, 1, GL_RGBA8, _width, _height, _cpuSurfaces.size());

        for (int i = 0; i < _cpuSurfaces.size(); ++i)
            glTextureSubImage3D(_tex, 0, 0, 0, 0, _width, _height, 1, GL_RGBA, GL_UNSIGNED_BYTE, _cpuSurfaces[i] ? _cpuSurfaces[i]->pixels : nullptr);
    }

    void Image2DArray::UnLoadGL()
    {
        if (!HasLoadedGL())
            return;

        glDeleteTextures(1, &_tex);

        _tex = 0;
    }

    bool Image2DArray::HasLoadedGL()
    {
        return _tex;
    }
}