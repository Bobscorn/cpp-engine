#pragma once

#include "Helpers/VectorHelper.h"
#include "Helpers/SDLHelper.h"
#include "Helpers/GLHelper.h"

#include "Drawing/Image.h"

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <GL/glew.h>

#include <array>

struct IRen2D;

struct ScissorThingo
{
	ScissorThingo(IRen2D *p) : m_Ren(p) {}
	~ScissorThingo();

protected:
	IRen2D *m_Ren;
};

struct IRen2D
{
	virtual ~IRen2D() {}

	virtual void DrawPoint(floaty2 point, floaty4 color) = 0;
	virtual void DrawLine(floaty2 a, floaty2 b, floaty4 color) = 0;

	// A ---- \
	// |      |
	// \ ---- B
	virtual void DrawRectangle(floaty2 topleft, floaty2 bottomright, floaty4 color) = 0;

	// A ---- B
	// |      |
	// C ---- D
	virtual void DrawRectangle(floaty2 a, floaty2 b, floaty2 c, floaty2 d, floaty4 color) = 0;

	// A ---- \
	// |      |
	// \ ---- B
	virtual void FillRectangle(floaty2 topleft, floaty2 bottomright, floaty4 color) = 0;

	// A ---- B
	// |      |
	// C ---- D
	virtual void FillRectangle(floaty2 a, floaty2 b, floaty2 c, floaty2 d, floaty4 color) = 0;

	virtual void DrawTriangle(floaty2 a, floaty2 b, floaty2 c, floaty4 color) = 0;
	virtual void FillTriangle(floaty2 a, floaty2 b, floaty2 c, floaty4 color) = 0;
	virtual void DrawVertices(std::vector<floaty2> vertices, floaty4 color) = 0; // Draw them as a Triangle List
	virtual void FillVertices(std::vector<floaty2> vertices, floaty4 color) = 0; // Fill them as a Triangle List
	virtual void DrawVertices(std::vector<floaty2> vertices, floaty4 color, GLenum mode) = 0;
	virtual void FillVertices(std::vector<floaty2> vertices, floaty4 color, GLenum mode) = 0; // Mode is vertex listing mode

	virtual void DrawImage(Drawing::SDLImage *im) = 0;

	/// Draw *all* of an image to target Rect/point (will be transformed)
	virtual void DrawImage(Drawing::SDLImage *im, PointRect target) = 0;
	virtual void DrawImage(Drawing::SDLImage *im, floaty2 target) = 0;

	/// Draw part of an image to target Rect
	/// <param name="src">The Portion of the image to fit into target</param>
	/// <param name="target">The target Rectangle to fit the image into</param>
	virtual void DrawImage(Drawing::SDLImage *im, PointRect src, PointRect target) = 0;

	// Set the transform matrix to be used for rendering
	virtual void SetTransform(Matrixy2x3 trans) = 0;
	virtual floaty2 Transform(floaty2 me) = 0;
	virtual PointRect Transform(PointRect me) = 0;
	virtual PointRect ToGLCoords(PointRect fromTopLeft) = 0;
	virtual floaty2 ToGLCoords(floaty2 fromTopLeft) = 0;

	inline virtual ScissorThingo ScissorRect(GLint x, GLint y, GLint width, GLint height) { SetScissorRect(x, y, width, height); return this; }
	virtual void SetScissorRect(GLint x, GLint y, GLint width, GLint height) = 0;
	virtual void ClearScissor() = 0;

	virtual void Engage() = 0;

	virtual bool IsGood() noexcept = 0;

	virtual void Resize(unsigned int NewWidth, unsigned int NewHeight) = 0;

	virtual void Clear(SDL_Color color) = 0;
	virtual void Present(SDL_Window *win) = 0;
};