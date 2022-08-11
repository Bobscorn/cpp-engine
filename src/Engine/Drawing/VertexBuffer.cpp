#include "VertexBuffer.h"

namespace Drawing
{
    std::shared_ptr<VertexBuffer> VertexBuffer::_staticBuffer{ nullptr };

    GLenum VertexBuffer::GetGLUsage()
    {
        if (_dynamic)
            return GL_DYNAMIC_DRAW;
        else
            return GL_STATIC_DRAW;
    }

    /**
    * TODO: All of this crap
    */
    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
    {
        _VBO = std::move(other._VBO);
        _IBO = std::move(other._IBO);
        _indices = std::move(other._indices);
        _nextKey = other._nextKey;
        _dynamic = other._dynamic;
        _dirty = other._dirty;

        _vertices.Description = other._vertices.Description;
        _vertices.Vertices = std::move(other._vertices.Vertices);

        other._nextKey = 1;
        other._dynamic = false;
        other._dirty = false;

        return *this;
    }

    VertexBuffer::VertexBuffer()
        : _VBO()
        , _IBO()
        , _indices()
        , _dynamic(false)
        , _dirty(true)
        , _vertices()
        , _nextKey(1)
    {
    }

    VertexBuffer::VertexBuffer(const RawMesh& initialMesh)
        : VertexBuffer()
    {
        _vertices = initialMesh.VertexData;

        _dirty = true;
    }

    size_t VertexBuffer::AddMesh(const RawMesh& mesh)
    {
        if (_vertices.Vertices.empty())
        {
            _dirty = true;
            _vertices = mesh.VertexData;
            _indices = mesh.Indices;

            _meshOffsets.clear();
            _meshOffsets.emplace_back(InternalMeshData{ MeshOffsetData{ 0, 0, (GLsizei)mesh.Indices.size() }, (GLuint)mesh.VertexData.NumVertices(), 1 });
            if (_nextKey == 1)
                _nextKey = 2;
            return 1;
        }

        if (mesh.VertexData.Description != _vertices.Description && _vertices.Vertices.size())
            return 0; // Maybe throw?

        GLuint offset = (GLuint)_vertices.Vertices.size();
        _vertices.Vertices.insert(_vertices.Vertices.end(), mesh.VertexData.Vertices.begin(), mesh.VertexData.Vertices.end());

        GLuint start = (GLuint)_indices.size();
        _indices.insert(_indices.end(), mesh.Indices.begin(), mesh.Indices.end());

        InternalMeshData data;
        data.Data.IndexOffset = offset;
        data.Data.IndexStart = start;
        data.Data.IndicesCount = mesh.Indices.size();
        data.VertexCount = mesh.VertexData.NumVertices();
        data.Key = _nextKey++;

        _meshOffsets.emplace_back(std::move(data));

        _dirty = true;

        return data.Key;
    }

    void VertexBuffer::RemoveMesh(size_t key)
    {
        if (key == 0)
            return;
        size_t start = key;
        if (start >= _meshOffsets.size())
            start = _meshOffsets.size();

        size_t foundOffset = (size_t)-1;
        for (size_t i = start; i-- > 0; )
        {
            if (_meshOffsets[i].Key == key)
            {
                foundOffset = i;
                break;
            }
        }

        if (foundOffset == (size_t)-1)
        {
            DWARNING("Trying to remove a non-existant mesh from vertex buffer");
            return;
        }

        auto& desc = _meshOffsets[foundOffset];

        for (auto it = _meshOffsets.begin() + (foundOffset + 1); it != _meshOffsets.end(); ++it)
        {
            it->Data.IndexOffset -= desc.VertexCount;
            it->Data.IndexStart -= desc.Data.IndicesCount;
        }

        size_t size = _vertices.VertexByteSize();

        _vertices.Vertices.erase(_vertices.Vertices.begin() + desc.Data.IndexOffset * size, _vertices.Vertices.begin() + ((size_t)desc.Data.IndexOffset + desc.VertexCount) * size);
        _indices.erase(_indices.begin() + desc.Data.IndexStart, _indices.begin() + desc.Data.IndexStart + desc.Data.IndicesCount);

        _meshOffsets.erase(_meshOffsets.begin() + foundOffset);

        _dirty = true;
    }

    bool VertexBuffer::TryGetMeshOffset(size_t key, MeshOffsetData* outData)
    {
        if (key == 0)
            return false;
        size_t start = key;
        if (start >= _meshOffsets.size())
            start = _meshOffsets.size();

        size_t foundOffset = (size_t)-1;
        for (size_t i = start; i-- > 0; )
        {
            if (_meshOffsets[i].Key == key)
            {
                foundOffset = i;
                break;
            }
        }

        if (foundOffset == (size_t)-1)
        {
            return false;
        }

        *outData = _meshOffsets[foundOffset].Data;
        return true;
    }

    void VertexBuffer::UpdateIfDirty()
    {
        if (!_dirty)
            return;

        GLuint bufs[2] = { 0 };
        glGenBuffers(2, bufs);

        _VBO.Reset(bufs[0]);
        _IBO.Reset(bufs[1]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _IBO.Get());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _indices.size(), _indices.data(), GetGLUsage());

        glBindBuffer(GL_ARRAY_BUFFER, _VBO.Get());
        glBufferData(GL_ARRAY_BUFFER, _vertices.Vertices.size(), _vertices.Vertices.data(), GetGLUsage());

        _dirty = false;
    }

    std::shared_ptr<VertexBuffer> VertexBuffer::GetStaticBuffer()
    {
        return _staticBuffer;
    }

    void VertexBuffer::InitializeStaticBuffer()
    {
        if (_staticBuffer)
            return;

        _staticBuffer = std::make_shared<VertexBuffer>();
    }
}