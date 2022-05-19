#pragma once

#include "Helpers/GLHelper.h"

#include "Geometry.h"

#include <unordered_map>
#include <memory>

namespace Drawing
{
	struct MeshOffsetData
	{
		GLuint IndexOffset;
		GLuint IndexStart;
		GLsizei IndicesCount;
	};

	class VertexBuffer
	{
		struct InternalMeshData
		{
			MeshOffsetData Data;
			GLuint VertexCount;
			size_t Key;
		};

		GLBuffer _VBO;
		GLBuffer _IBO;

		VertexData _vertices;
		std::vector<GLuint> _indices;
		std::vector<InternalMeshData> _meshOffsets;

		size_t _nextKey = 1;

		bool _dynamic = false;
		bool _dirty = true;

		GLenum GetGLUsage();


		VertexBuffer& operator=(VertexBuffer&& other);

		static std::shared_ptr<VertexBuffer> _staticBuffer;

	public:
		VertexBuffer();
		VertexBuffer(const RawMesh& initialMesh);


		size_t AddMesh(const RawMesh& mesh);

		void RemoveMesh(size_t key);

		bool TryGetMeshOffset(size_t key, MeshOffsetData* outData);

		void UpdateIfDirty();

		inline const GLBuffer& GetVBO() const { return _VBO; }
		inline const GLBuffer& GetIBO() const { return _IBO; }
		inline const GeometryDescription& GetDescription() const { return _vertices.Description; }

		static std::shared_ptr<VertexBuffer> GetStaticBuffer();
		static void InitializeStaticBuffer();
	};
}