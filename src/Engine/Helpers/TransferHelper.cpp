#include "TransferHelper.h"

FullResourceHolder::FullResourceHolder(CommonResources* resources)
	: mResources(resources)
{
}

FullResourceHolder::~FullResourceHolder() {}

void FullResourceHolder::SetResources(CommonResources* resources)
{
	mResources = resources;
}

CommonResources* FullResourceHolder::GetResources() const
{
	return mResources;
}