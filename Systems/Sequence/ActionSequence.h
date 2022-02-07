#pragma once

namespace Sequence
{
	struct ISequenceElement;

	struct Sequence
	{

	protected:
		ISequenceElement *Front{ nullptr };
		ISequenceElement *Back{ nullptr };
	};

	struct ISequenceElement
	{
		virtual ISequenceElement *GetPrevious() = 0;
		virtual ISequenceElement *GetNext() = 0;


	};
}