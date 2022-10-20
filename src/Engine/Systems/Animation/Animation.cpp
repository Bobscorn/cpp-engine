#include "Animation.h"

#include "Helpers/StringHelper.h"
#include "Helpers/TransferHelper.h"

#include "Systems/Events/Events.h"

#include "Systems/Thing.h"

using namespace Animation;
using namespace Framing;

Bezier::BinomialNumbers Bezier::Binomial;

Animation::Framing::PositionFrames::PositionFrames(Interpolation::INTERPOLATE inter, std::vector<Frame<DOUBLE4>> frames, CommonResources * resources) : IFrames(inter, frames) 
{ 
	Event::ResizeEvent ass({ (int)*resources->WindowWidth, (int)*resources->WindowHeight }, *resources->UpdateID); this->Receive(&ass);
};

Framing::InterpolatedFrame<Framing::PositionFrames::FrameType> Framing::PositionFrames::GetAt(double time)
{
	if (time <= 0.0)
		return CachedFrame;

	UINT index = (UINT)PixelFrames.size() + 1u;
	for (UINT i = (UINT)PixelFrames.size(); i-- > 0; )
	{
		if (time > PixelFrames[i].Time)
		{
			index = i;
			break;
		}
	}
	// Check if all frames are later than supplied time (do this check first to prevent out of range exception)
	if (index == PixelFrames.size() + 1) // index gets set to Frames.size() + 1, and only changes if a frame is found
		return (CachedFrame = { PixelFrames.front().Data, FRAME_ANIMATION_NOT_STARTED });

	// Check if the frame found is the last one
	if (index == PixelFrames.size() - 1)
		return (CachedFrame = { PixelFrames.back().Data, FRAME_ANIMATION_ENDED });

	Frame<FrameType> &A = PixelFrames[index];
	Frame<FrameType> &B = PixelFrames[index + 1];

	double frame_diff = B.Time - A.Time;
	double timesinceframe = time - A.Time;
	double interpolation = Animation::Interpolation::Interpolate1D(timesinceframe / frame_diff, this->Interpolation);

	return (CachedFrame = { A.Data + interpolation * B.Data, 0 });
}

Animation::Framing::WorldFrames2D::WorldFrames2D(Interpolation::INTERPOLATE inter, std::vector<Frame<Matrixy2x3>> frames, CommonResources * resources): IFrames(inter, frames) 
{
	resources->Event->Add(this); Event::ResizeEvent ass({ (int)*resources->WindowWidth, (int)*resources->WindowHeight }, *resources->UpdateID); this->Receive(&ass);
}

void Animation::Framing::WorldFrames2D::Init(Interpolation::INTERPOLATE inter, std::vector<Frame<Matrixy2x3>> frames, CommonResources * resources)
{
	Frames = frames; Interpolation = inter; resources->Event->Add(this); Event::ResizeEvent ass({ (int)*resources->WindowWidth, (int)*resources->WindowHeight }, *resources->UpdateID); this->Receive(&ass);
}

Framing::InterpolatedFrame<Framing::WorldFrames2D::FrameType> Framing::WorldFrames2D::GetAt(double time)
{
	if (time <= 0.0)
		return CachedFrame;

	UINT index = (UINT)PixelFrames.size() + 1;
	for (UINT i = (UINT)PixelFrames.size(); i-- > 0; )
	{
		if (time > PixelFrames[i].Time)
		{
			index = i;
			break;
		}
	}

	// Check if all frames are later than supplied time (do this check first to prevent out of range exception)
	if (index == PixelFrames.size() + 1) // index gets set to Frames.size() + 1, and only changes if a frame is found
		return (CachedFrame = { PixelFrames.front().Data, FRAME_ANIMATION_NOT_STARTED });

	// Check if the frame found is the last one
	if (index == PixelFrames.size() - 1)
		return (CachedFrame = { PixelFrames.back().Data, FRAME_ANIMATION_ENDED });

	Framing::Frame<FrameType> &frameA = PixelFrames[index];
	Framing::Frame<FrameType> &frameB = PixelFrames[index + 1];

	double frame_diff = frameB.Time - frameA.Time;
	double timesinceframe = time - frameA.Time;
	double interpolation = Animation::Interpolation::Interpolate1D(fmin(timesinceframe / frame_diff, 1.0), this->Interpolation);

	Matrixy2x3 matrix_diff = frameB.Data - frameA.Data;

	return (CachedFrame = { frameA.Data + ((float)interpolation * matrix_diff), 0 });
}

Animation::Bezier::BinomialNumbers::BinomialNumbers()
	: CoEfficients(
		{
						  { 1u },
						{ 1u, 1u },
					  { 1u, 2u, 1u },
					{ 1u, 3u, 3u, 1u },
				  { 1u, 4u, 6u, 4u, 1u },
				{ 1u, 5u, 10u, 10u, 5u, 1u },
			  { 1u, 6u, 15u, 20u, 15u, 6u, 1u },
			{ 1u, 7u, 21u, 35u, 35u, 21u, 7u, 1u }
		}
	)
{
}

double Animation::Bezier::BinomialNumbers::operator()(UINT n, UINT k)
{
	while (n >= CoEfficients.size())
	{
		UINT s = (UINT)CoEfficients.size();
		UINT *nextRow = new UINT[s + 1];
		nextRow[0] = nextRow[s] = 1u;
		for (UINT i = s; i-- > 1; )
			nextRow[i] = CoEfficients.back()[i - 1] + CoEfficients.back()[i];
		CoEfficients.push_back(std::vector<UINT>(nextRow, nextRow + (s + 1)));
		delete[] nextRow;
	}
	return CoEfficients[n][k];
}

double Animation::Bezier::GetBezier1D(UINT n, double t, std::vector<double> weights)
{
	if (n == 0)
	{
		DWARNING("given n = 0");
		return 0.0;
	}
	if (weights.size() <= n)
	{
		DWARNING("given too little weights for given polynomial degree");
		return 0.0;
	}

	std::vector<double> &w = weights;
	if (n == 1)
	{
		DINFO("given n = 1, not undefined behaviour, but usually beziers are only used when n > 1");
		return w[0] * t;
	}
	if (n == 2)
	{
		double t2 = t * t;
		double mt = 1.0 - t;
		double mt2 = mt * mt;
		return w[0] * mt2 + w[1] * 2 * mt * t + w[2] * t2;
	}
	else if (n == 3)
	{
		double t2 = t * t;
		double t3 = t2 * t;
		double mt = 1.0 - t;
		double mt2 = mt * mt;
		double mt3 = mt2 * mt;
		return w[0] * 1 * mt3
			+ w[1] * 3 * mt2 * t
			+ w[2] * 3 * mt * t2
			+ w[3] * 1 * t3;
	}
	else
	{
		double sum = 0;
		for (UINT k = n; k-- > 0; )
			sum += w[k] * Binomial(n, k) * pow(1.0 - t, n - k) * pow(t, k);
		return sum;
	}
}

DOUBLE2 Animation::Bezier::GetBezier2D(UINT n, double t, std::vector<DOUBLE2> weights)
{
	if (n == 0)
	{
		DWARNING("given n = 0");
		return { 0.0, 0.0 };
	}
	if (weights.size() <= n)
	{
		DERROR("given too little weights for given polynomial degree");
		return { 0.0, 0.0 };
	}

	std::vector<DOUBLE2> &w = weights;
	if (n == 1)
	{
		DINFO("given = 1, not bad behaviour, however use of beziers with degrees below 2 are unknown (as in idk why you would use them)");
		return { w[0].x * t, w[0].y * t };
	}
	if (n == 2)
	{
		double t2 = t * t;
		double mt = 1.0 - t;
		double mt2 = mt * mt;
		double &term1 = mt2; // 1 * t^3
		double term2 = 2 * mt * t;
		double &term3 = t2;
		return { w[0].x * term1 + w[1].x * term2 + w[2].x * term3,
				 w[0].y * term1 + w[1].y * term2 + w[2].y * term3 };
	}
	if (n == 3)
	{
		double t2 = t * t;
		double t3 = t2 * t;
		double mt = 1.0 - t;
		double mt2 = mt * mt;
		double mt3 = mt2 * mt;
		double &term1 = mt3;		// 1 * (1 - t)^3 * t^0
		double term2 = 3 * mt2 * t; // 3 * (1 - t)^2 * t^1
		double term3 = 3 * mt * t2; // 3 * (1 - t)^1 * t^2
		double &term4 = t3;			// 1 * (1 - t)^0 * t^3
		return { w[0].x * term1 + w[1].x * term2 + w[2].x * term3 + w[3].x * term4,   // x1 * (1 - t)^3  +  x2 * 3 * (1 - t)^2 * t  +  x3 * 3 * (1 - t) * t^2  +  x4 * t^3
				 w[0].y * term1 + w[1].y * term2 + w[2].y * term3 + w[3].y * term4 }; // y1 * (1 - t)^3  +  y2 * 3 * (1 - t)^2 * t  +  y3 * 3 * (1 - t) * t^2  +  y4 * t^3
	}
	else
	{
		double sumx = 0.0;
		double sumy = 0.0;
		for (UINT k = n; k-- > 0; )
		{
			double term = Binomial(n, k) * pow(1.0 - t, n - k) * pow(t, k);
			sumx += w[k].x * term;
			sumy += w[k].y * term;
		}
		return { sumx, sumy };
	}
}

double Animation::Interpolation::Interpolate1D(double at, INTERPOLATE type)
{
	switch (type)
	{
	default:
		return fmin(1.0, fmax(0.0, at));

	case INTERPOLATE_SMOOTH_IN_OUT:
		return InterpolateSmooth1D(at);

	case INTERPOLATE_FAST_IN_SMOOTH_OUT:
		return InterpolateFastIn1D(at);

	case INTERPOLATE_SMOOTH_IN_FAST_OUT:
		return InterpolateFastOut1D(at);
	}
}

double Animation::Interpolation::InterpolateSmooth1D(double at)
{
	return Bezier::GetBezier1D(3, at, Bezier::Smooth);
}

double Animation::Interpolation::InterpolateFastIn1D(double at)
{
	return Bezier::GetBezier1D(2, at, Bezier::FastIn);
}

double Animation::Interpolation::InterpolateFastOut1D(double at)
{
	return Bezier::GetBezier1D(2, at, Bezier::FastOut);
}

void Animation::TransformAnimator2D::Load(std::vector<Loading::LoadedAnimation> &anims, CommonResources * resources)
{
	std::vector<Framing::WorldFrames2D> Converted;
	for (size_t i = anims.size(); i-- > 0; )
	{
		// Somehow convert std::vector<std::vector<double>> to std::vector<Matrixy2x3>
		// Then convert the std::vector<Matrixy2x3> to WorldFrames2D
		
		// First conversion
		// v
		std::vector<Frame<Matrixy2x3>> ConvertedFrames(anims[i].Frames.size(), Frame<Matrixy2x3>());
		for (size_t j = anims[i].Frames.size(); j-- > 0; )
		{
			// Checks if 
			Frame<Matrixy2x3> &cframe = ConvertedFrames[j];
			std::vector<double> &uframe = anims[i].Frames[j];
			cframe.Data = Matrixy2x3::Identity();
			cframe.Time = 1.0;
			if (uframe.size())
			{
				cframe.Time = uframe[0];

				if (uframe.size() > 1)
				{
					cframe.Data.m11 = (float)uframe[1];

					if (uframe.size() > 2)
					{
						cframe.Data.m21 = (float)uframe[2];

						if (uframe.size() > 3)
						{
							cframe.Data.dx = (float)uframe[3];

							if (uframe.size() > 4)
							{
								cframe.Data.m12 = (float)uframe[4];

								if (uframe.size() > 5)
								{
									cframe.Data.m22 = (float)uframe[5];

									if (uframe.size() > 6)
										cframe.Data.dy = (float)uframe[6];
								}
							}
						}
					}
				}
			}
		}
		this->Animations.emplace(std::make_pair(anims[i].Name, Framing::WorldFrames2D(anims[i].Intertype, ConvertedFrames, resources)));
	}
}

void Animation::TransformAnimator2D::Start(std::string name)
{
	if (Animations.find(name) != Animations.end())
	{
		CurrentAnimation = &Animations.at(name);
		AnimTimer.Reset();
		AnimTimer.Start();
	}
	else
	{
		DERROR("given invalid animation");
	}
}

Matrixy2x3 Animation::TransformAnimator2D::GetMatrix()
{
	if (CurrentAnimation)
	{
		AnimTimer.Tick();
		InterpolatedFrame<Matrixy2x3> frame = CurrentAnimation->GetAt(AnimTimer.TotalTime());
		if (frame.Info & FRAME_ANIMATION_ENDED)
			AnimTimer.Stop();
		return frame.Data;
	}
	else
		return Matrixy2x3::Identity();
}

bool Animation::Loading::LoadedAnimation::Load(std::string name, std::map<std::string, std::vector<std::string>> from)
{
	this->Name = name;
	if (from.find("Type") == from.end() || from["Type"].empty())
		DWARNING("Loading Animation, Type not found, assuming Matrix");

	Interpolation::INTERPOLATE itype = Interpolation::INTERPOLATE_LINEAR;
	if (from.find("Interpolate") != from.end() && from["Interpolate"].size())
		StringHelper::IfInterpolation(from["Interpolate"][0], itype);
	
	Intertype = itype;

	Type = from["Type"][0];

	for (auto& thing : from)
	{
		const std::string &ass = thing.first;
		if (ass.size() > 5 && ass[0] == *"F" && ass[1] == *"r" && ass[2] == *"a" && ass[3] == *"m" && ass[4] == *"e")
		{
			UINT frame_number = 0u;
			// Check if the characters after the 5 characters 'Frame' are a number
#pragma warning(suppress:4239)
			if (StringHelper::IfUINT(std::string(ass.begin() + 5, ass.end()), &frame_number))
			{
				if (frame_number >= Frames.size())
					Frames.resize(frame_number + 1);

				if (thing.second.size())
				{
					Frames[frame_number].resize(thing.second.size());
					for (size_t i = thing.second.size(); i-- > 0; )
					{
						double filevalue = 0.0;
						if (StringHelper::IfDouble(thing.second[i], &filevalue))
							Frames[frame_number][i] = filevalue;
					}
				}
			}
		}
	}
	return Frames.size();
}

bool Animation::Loading::LoadedAnimations::Load(std::string filename)
{
	if (filename.empty())
	{
		DERROR("given empty filename");
		return false;
	}

	std::map<std::string, std::map<std::string, std::map<std::string, std::vector<std::string>>>> ass;
	//if (Config::Storage::ProcessFile(filename, ass))
	//{
	//	// Go through objects, load up all their animations
	//	// Then enter them into the member variable map
	//	for (auto& entry : ass)
	//	{
	//		// Make vector of animations
	//		std::vector<LoadedAnimation> ass2(entry.second.size(), LoadedAnimation());
	//		size_t index = entry.second.size();
	//		for (auto& entry2 : entry.second)
	//		{
	//			--index;
	//			ass2[index].Load(entry2.first, entry2.second);
	//		}
	//		// Chuck the now processed vector into the map
	//		Animations[entry.first] = std::move(ass2);
	//	}
	//	return true;
	//}
	//else
	//{
	//	DERROR("Failed to process file '" + filename + "' ");
	//	return false;
	//}
	DWARNING("using unupdated Animation pipeline that relies on dead code as such this function does nothing and no loading has occured");
	return true;
}