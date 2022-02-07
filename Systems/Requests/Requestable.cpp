#include "Requestable.h"
#include "Requester.h"

Requests::IRequestable::~IRequestable()
{
	if (master)
		master->Remove(this);
}
