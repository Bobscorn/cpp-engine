#include "DrawCallReference.h"

#include "GLRen2.h"

namespace Drawing
{
	DrawCallReference::DrawCallReference(DrawCallReference&& other)
		: _key(other._key)
		, _ren(other._ren)
	{
		other._key = 0ull;
		other._ren = nullptr;
	}

	DrawCallReference::~DrawCallReference()
	{
		Reset();
	}

	DrawCallReference& DrawCallReference::operator=(DrawCallReference&& other)
	{
		Reset(other._key, other._ren);

		other._key = 0ull;
		other._ren = nullptr;

		return *this;
	}

	void DrawCallReference::Reset(size_t key, DrawCallRenderer* ren)
	{
		if (_key != 0ull && _ren != nullptr)
		{
			_ren->RemoveDrawCall(_key);
		}
		_key = key;
		_ren = ren;
	}
}