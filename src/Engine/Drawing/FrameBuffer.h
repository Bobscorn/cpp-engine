#pragma once

#include "Helpers/GLHelper.h"

namespace Drawing
{
	class GLFrameBuffer
	{
		GLuint _fbo = 0;

	public:
		GLFrameBuffer() {}
		GLFrameBuffer(GLuint fbo) : _fbo(fbo) {}
		GLFrameBuffer(GLFrameBuffer&& other) noexcept : _fbo(other.Release()) {}
		GLFrameBuffer(const GLFrameBuffer& other) = delete;
		~GLFrameBuffer();

		inline GLFrameBuffer& operator=(GLFrameBuffer&& other) noexcept { Reset(other.Release()); return *this; }
		GLFrameBuffer& operator=(const GLFrameBuffer& other) = delete;

		inline GLuint Get() const noexcept { return _fbo; }

		inline GLuint Release() noexcept
		{
			auto tmp = _fbo;
			_fbo = 0;
			return tmp;
		}

		inline void Reset(GLuint fbo = 0)
		{
			Destroy();
			_fbo = fbo;
		}
		
		void Destroy();

		static GLFrameBuffer CreateFBO();
	};
}