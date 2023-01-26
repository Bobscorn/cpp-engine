#include "FrameBuffer.h"

Drawing::GLFrameBuffer::~GLFrameBuffer()
{
	Destroy();
}

void Drawing::GLFrameBuffer::Destroy()
{
	if (_fbo)
		glDeleteFramebuffers(1, &_fbo);
	_fbo = 0;
}

Drawing::GLFrameBuffer Drawing::GLFrameBuffer::CreateFBO()
{
	GLuint fbo = 0;
	CHECK_GL_ERR("Before Creating Frame Buffer");

	glCreateFramebuffers(1, &fbo);
	CHECK_GL_ERR("Creating Frame Buffer");

	return GLFrameBuffer(fbo);
}
