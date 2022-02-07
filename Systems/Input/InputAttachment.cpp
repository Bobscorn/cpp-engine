#include "InputAttachment.h"

InputAttach::IAttachable::~IAttachable()
{
	if (manager) manager->Remove(this);
}
