#pragma once


#include <exception>
#include <string>

#ifndef __glew_h__
#include <GL/glew.h> // Ensure glew.h is included before SDL_opengl.h
#endif
#ifdef __linux__
#include <SDL2/SDL_opengl.h>
#else
#include <SDL_opengl.h>
#endif

namespace Debug
{
	class ProbablyBadFileException : std::exception
	{
		[[nodiscard]] char const *what() const noexcept override
		{
			return "Probably a bad file exception";
		}
	};

	class GLExc : public std::exception
	{
	public:
		GLExc(GLenum er, const char *failedat, const char * func, const char *file, const char *line) { Final = std::string(((const char*)gluErrorString(er))) + " in Function: " + func + " when calling: " + failedat + " in file: " + file + "(" + line + ")"; }
		GLExc(GLenum er, std::string failedat, const char * func, const char *file, const char *line) { Final = std::string(((const char*)gluErrorString(er))) + " in Function: " + func + " when calling: " + failedat + " in file: " + file + "(" + line + ")"; }
		GLExc(const char *er) : Final(er) {}


		[[nodiscard]] const char *what() const noexcept override
		{
			return Final.c_str();
		}

	protected:
		std::string Final;
	};

#define GET_GL_ERRE(x) std::string((const char*)gluErrorString(x))
#define GET_GL_ERR GET_GL_ERRE(glGetError())
//#define CHECK_GL_ERR(x) {auto er = glGetError(); if (er != GL_NO_ERROR) DWARNING(GET_GL_ERRE(er)); } //throw Debug::GLExc(er, x, __FUNCTION_NAME__, __FILE__, std::to_string(__LINE__).c_str());}
#ifdef NDEBUG
#define CHECK_GL_ERR(x) 
#else
#define CHECK_GL_ERR(x) {auto er = glGetError(); if (er != GL_NO_ERROR) throw Debug::GLExc(er, x, __FUNCTION_NAME__, __FILE__, std::to_string(__LINE__).c_str());}
#endif
#define GL_EXCEPT(x) Debug::GLExc(glGetError(), x, __FUNCTION_NAME__, __FILE__, std::to_string(__LINE__).c_str())
#define PRINT_GL_ERR(x) { auto er = glGetError(); if (er != GL_NO_ERROR) DINFO("OpenGL Error: " + Debug::GLEtoC(glGetError()) + " in " + __FUNCTION_NAME__ + " in file " + __FILE__ + std::to_string(__LINE__).c_str() + ", " + x); }

	inline std::string GLEtoC(GLenum er)
	{
		switch (er)
		{
		default:
		case GL_NO_ERROR:
			return "No Error";
		case GL_INVALID_ENUM:
			return "Invalid Enumeration Value";
		case GL_INVALID_VALUE:
			return "Invalid Value";
		case GL_STACK_OVERFLOW:
			return "Stack Overflow";
		case GL_STACK_UNDERFLOW:
			return "Stack Underflow";
		case GL_OUT_OF_MEMORY:
			return "Out of memory";
		}
	}


	struct SDLException : std::exception
	{
		SDLException() {}
		SDLException(const char *er) : Error(er) {}

	protected:
		const char *Error = "General SDL Failure";
	};
}