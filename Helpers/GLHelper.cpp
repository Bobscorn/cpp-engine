#include "GLHelper.h"

#include "Helpers/ExceptionHelper.h"

GLProgram GLProgram::CreateProgram(std::vector<GLShaderPair> shaderfiles)
{
	if (shaderfiles.empty())
		throw Debug::GLExc("Cannot create program with no shaders");

	GLProgram out = glCreateProgram();

	if (!out)
	{
		DERROR("Failed to create Program, OpenGL Error: " + GET_GL_ERR);
		throw GL_EXCEPT("glCreateProgram");
	}

	std::vector<GLShader> CompiledShaders;
	CompiledShaders.reserve(shaderfiles.size());
	for (auto& shader : shaderfiles)
	{
		auto &ShaderFile = shader.path;
		auto &ShaderType = shader.Type;
		GLShader &CompiledShader = CompiledShaders.emplace_back(LoadShader(ShaderFile, ShaderType));
		
		if (!CompiledShader)
		{
			DERROR("Failed to create and compile shader, aborting program creation");
			throw Debug::GLExc("Failed to compile shader");
		}

		glAttachShader(out.Get(), CompiledShader.Get());
		CHECK_GL_ERR("glAttachShader");
	}

	CHECK_GL_ERR("thing before glLinkProgram");
	GLuint p = out.Get();
	DINFO("Program ID: " + std::to_string(p));
	glLinkProgram(p);

	CHECK_GL_ERR("glLinkProgram");

	GLint success = GL_TRUE;
	glGetProgramiv(p, GL_LINK_STATUS, &success);
	if (success != GL_TRUE)
	{
		DERROR("Failed to link program");
		OutputProgramLog(p);
		
		throw GL_EXCEPT("glLinkProgram");
	}

	return out;
}
