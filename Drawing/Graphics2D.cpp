#include "Graphics2D.h"

#include "Helpers/ExceptionHelper.h"

ScissorThingo::~ScissorThingo()
{ m_Ren->ClearScissor();
}
