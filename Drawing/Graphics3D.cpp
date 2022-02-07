#include "Graphics3D.h"

#include "Drawing/IRen3D.h"

void GeoThing::Reset()
{
	if (m_Key && m_Ren)
		m_Ren->RemoveGeometry(m_Key);
	m_Ren = nullptr;
	m_Key = 0ull;
}