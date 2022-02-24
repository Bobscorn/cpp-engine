#include "TimeSequence.h"

void Sequence::TimeSequence::Resume()
{
	SequenceTimer.Start();
}

void Sequence::TimeSequence::Reset()
{
	SequenceTimer.Reset();
}

void Sequence::TimeSequence::Pause()
{
	SequenceTimer.Stop();
}

bool Sequence::TimeSequence::Apply()
{
	if (CurrentIndex >= Elements.size())
		return true;

	SequenceTimer.Tick();
	double time = SequenceTimer.TotalTime();
	if (time == LastTime)
		return false;

	LastTime = time;

	while (time > 0.0)
	{
		if (CurrentIndex < Elements.size())
		{
			double temp = Elements[CurrentIndex]->Apply(time);
			if (temp > 0.0)
			{
				LastTime += time - temp;
				if ((CurrentIndex + 1) == Elements.size())
					DINFO("Finishing Sequence");
				else
					DINFO("Moving to next action sequence");
				++CurrentIndex;
			}
			time = temp;
		}
		else
			break;
	}

	return CurrentIndex >= Elements.size();
}
