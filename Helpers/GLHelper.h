#pragma once

#include "Helpers/DebugHelper.h"
#include "Helpers/ExceptionHelper.h"

#include <GL/glew.h>
#ifdef __linux__
#include <SDL2/SDL_opengl.h>
#else
#include <SDL_opengl.h>
#endif

#include <string>
#include <fstream>

#if defined(_DEBUG) | defined(DEBUG)
#include <iostream>

inline void GLAPIENTRY glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
{
	(void)length;
	(void)userParam;
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif



typedef void(GLAPIENTRY*funcy)(GLuint);
typedef void(GLAPIENTRY*funcy2)(GLsizei, const GLuint*);

template<void (GLAPIENTRY*func)(GLuint)>
struct Wrapper
{
	Wrapper() : InternalLoc(0) {}
	Wrapper(GLuint in) : InternalLoc(in) {}
	Wrapper(Wrapper<func> &&other) : InternalLoc(other.InternalLoc) { other.InternalLoc = 0u; }
	~Wrapper() { Reset(); }


	inline Wrapper<func> &operator=(Wrapper<func> &&other)
	{
		Reset(other.InternalLoc);
		other.InternalLoc = 0u;
		return *this;
	}
	inline Wrapper<func> &operator=(GLuint newname)
	{
		Reset(newname);
		return *this;
	}

	inline void Reset(GLuint newname = 0u)
	{
		if (InternalLoc)
			(*func)(InternalLoc);
		InternalLoc = newname;
	}

	inline GLuint Release()
	{
		GLuint tmp = InternalLoc;
		InternalLoc = 0u;
		return tmp;
	}

	inline GLuint Get() const noexcept { return InternalLoc; }

	inline explicit operator GLuint() const noexcept { return InternalLoc; }
	inline explicit operator bool() const noexcept { return InternalLoc; }

protected:
	GLuint InternalLoc{ 0u };
};

template<funcy2 *func>
struct WrapperWrapper
{
	static inline void GLAPIENTRY Funcalicious(GLuint name) { (*func)(1, &name); }
};

template<funcy2 *func>
inline void GLAPIENTRY Funcylicious(GLuint name) { (*func)(1, &name); }

template<funcy2 *func>
struct Wrapper2 : Wrapper<Funcylicious<func>>
{
	Wrapper2() : SuperType() {}
	Wrapper2(GLuint in) : SuperType(in) {};
	Wrapper2(Wrapper2<func> &&other) : SuperType(other.InternalLoc) { other.InternalLoc = 0u; }

	inline Wrapper2 &operator=(GLuint newname) { SuperType::Reset(newname); return *this; }
	inline Wrapper2 &operator=(Wrapper2<func> &&other) noexcept { SuperType::Reset(other.InternalLoc); other.InternalLoc = 0u; return *this; }

	using SuperType = Wrapper<Funcylicious<func>>;
};

using GLBuffer = Wrapper2<&glDeleteBuffers>;
using GLVertexArray = Wrapper2<&glDeleteVertexArrays>;

GLuint LoadShader(std::string file, GLenum type);
void OutputShaderLog(GLuint shader);

struct GLShader
{
	GLShader(std::string file, GLenum type) : ShaderLoc(LoadShader(file, type)) {}
	GLShader(GLuint shader) : ShaderLoc(shader) {}
	GLShader(GLShader &&other) noexcept : ShaderLoc(other.ShaderLoc) { other.ShaderLoc = 0; }
	~GLShader() { glDeleteShader(ShaderLoc); }

	inline GLuint Get() const noexcept { return ShaderLoc; }

	inline explicit operator GLuint() const noexcept { return ShaderLoc; }
	inline explicit operator bool() const noexcept { return ShaderLoc; }

protected:
	GLuint ShaderLoc{ 0u };
};

struct GLShaderPair
{
	std::string path;
	GLenum Type = 0;
};

struct GLProgram
{
	//GLProgram(std::vector<GLuint> shaders = {}) : ProgramLoc(CreateProgram(shaders)) {}
	GLProgram(GLuint program) : ProgramLoc(program) {}
	GLProgram(GLProgram &&other) noexcept : ProgramLoc(other.ProgramLoc) { other.ProgramLoc = 0u; }
	GLProgram(std::vector<GLShaderPair> shaderfiles = {}) : GLProgram((shaderfiles.size() ? CreateProgram(shaderfiles) : 0)) {}
	~GLProgram() { Reset(); }

	inline GLProgram &operator=(GLuint newprogram)
	{
		Reset(newprogram);
		return *this;
	}
	inline GLProgram &operator=(GLProgram &&other) noexcept
	{
		Reset(other.ProgramLoc);
		other.ProgramLoc = 0u;
		return *this;
	}

	inline void Deactivate() { DeactivateAll(); }
	inline static void DeactivateAll() { glUseProgram(0); }
	static GLProgram CreateProgram(std::vector<GLShaderPair> shaderfiles);

	inline void Reset(GLuint newprogram = 0u)
	{
		if (ProgramLoc)
			glDeleteProgram(ProgramLoc);
		ProgramLoc = newprogram;
	}

	inline GLuint Get() const noexcept { return ProgramLoc; }
	inline explicit operator bool() const noexcept { return ProgramLoc; }
protected:

	GLuint ProgramLoc;
};

inline GLuint LoadShader(std::string file, GLenum shadertype)
{
	if (shadertype == 0)
	{
		// Deduce shader type from extension
		if (file.size() < 2)
			DERROR("Shader file is not even 2 characters long, cannot deduce shader type from that");
		std::string extension = file.substr(file.size() - 2, 2);
		if (extension == "fs")
			shadertype = GL_FRAGMENT_SHADER;
		else if (extension == "vs")
			shadertype = GL_VERTEX_SHADER;
		else if (extension == "cs")
			shadertype = GL_COMPUTE_SHADER;
		else if (extension == "tcs")
			shadertype = GL_TESS_CONTROL_SHADER;
		else if (extension == "tes")
			shadertype = GL_TESS_EVALUATION_SHADER;
		else if (extension == "gs")
			shadertype = GL_GEOMETRY_SHADER;
		else
		{
			throw GL_EXCEPT("Cannot deduce shader type from file: " + file);
		}
	}

	//Open file
	GLuint shaderID = 0;
	std::string shaderString;
	std::ifstream sourceFile(file.c_str());

	//Source file loaded
	if (sourceFile)
	{
		//Get shader source
		shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());

		//Create shader ID
		shaderID = glCreateShader(shadertype);

		//Set shader source
		const GLchar* shaderSource = shaderString.c_str();
		glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

		//Compile shader source
		glCompileShader(shaderID);

		//Check shader for errors
		GLint shaderCompiled = GL_FALSE;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE)
		{
			DINFO("Failed to compile shader '" + std::to_string(shaderID) + "' with Sauce:\n" + shaderSource);
			OutputShaderLog(shaderID);
			glDeleteShader(shaderID);
			shaderID = 0;
		}
	}
	else
	{
		DERROR("Failed to open shader file: " + file);
	}

	return shaderID;
}

inline void OutputShaderLog(GLuint shader)
{
	if (glIsShader(shader))
	{
		//Shader log length
		int ActualLength = 0;
		int MaxLength = 0;

		//Get info string length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &MaxLength);

		//Allocate string
		char* infoLog = new char[MaxLength];

		//Get info log
		glGetShaderInfoLog(shader, MaxLength, &ActualLength, infoLog);
		if (ActualLength > 0)
		{
			//Print Log
			DINFO("Shader Log: " + infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		DERROR("Name '" + std::to_string(shader) + "' is not a shader");
	}
}

inline void OutputProgramLog(GLuint program)
{
	if (glIsProgram(program))
	{
		glValidateProgram(program);
		//Program log length
		int ActualLength = 0;
		int MaxLength = 0;

		//Get info string length
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &MaxLength);

		//Allocate string
		char* infoLog = new char[MaxLength];

		//Get info log
		glGetProgramInfoLog(program, MaxLength, &ActualLength, infoLog);
		if (ActualLength > 0)
		{
			//Print Log
			DINFO("Shader Log: " + infoLog);
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		DERROR("Name '" + std::to_string(program) + "' is not a program");
	}
}