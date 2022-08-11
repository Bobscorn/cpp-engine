#pragma once

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#endif
#else
#ifndef DBG_NEW
#define DBG_NEW new
#endif
#endif  // _DEBUG*/

#ifndef B3_DEBUG_MEMORY_ALLOCATIONS
#define B3_DEBUG_MEMORY_ALLOCATIONS
#endif
#endif