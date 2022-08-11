#include "VoxelPlayerUI.h"

#include "Drawing/Graphics2D.h"

void Voxel::Crosshair::IDraw()
{
	// Top
	mResources->Ren2->FillRectangle(m_CrosshairCentre + floaty2{ -2.f, 10.f }, m_CrosshairCentre + floaty2{ 2.f, 3.f }, { 0.6f, 0.6f, 0.6f, 1.f });

	// Right
	mResources->Ren2->FillRectangle(m_CrosshairCentre + floaty2{ 3.f, 2.f }, m_CrosshairCentre + floaty2{ 10.f, -2.f }, { 0.6f, 0.6f, 0.6f, 1.f });

	// Bottom
	mResources->Ren2->FillRectangle(m_CrosshairCentre + floaty2{ -2.f, -3.f }, m_CrosshairCentre + floaty2{ 2.f, -10.f }, { 0.6f, 0.6f, 0.6f, 1.f });

	// Left
	mResources->Ren2->FillRectangle(m_CrosshairCentre + floaty2{ -10.f, 2.f }, m_CrosshairCentre + floaty2{ -3.f, -2.f }, { 0.6f, 0.6f, 0.6f, 1.f });
}

Stringy Voxel::Crosshair::GetName() const
{
	return "Player Crosshair";
}

void Voxel::Crosshair::CalculateCrosshairCentre()
{
	m_CrosshairCentre = { 0.f, 0.f };
}
