#include "BindingManager.h"

#include <stdexcept>

#include "Helpers/GLHelper.h"

namespace Drawing
{
	unsigned int BindingManager::_next = 0;

	unsigned int BindingManager::GetNext()
	{
		GLint maxUniforms = 0;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniforms);
		if (_next >= (GLuint)maxUniforms)
		{
			DERROR("Exceeded maximum number of uniform buffer bindings!");
			throw std::runtime_error("Ran out of OpenGL Uniform Buffer Bindings!");
		}
		return ++_next;
	}
}