#pragma once

#include "Systems/Events/Events.h"
#include "Systems/Timer/Timer.h"
#include "Helpers/VectorHelper.h"
#include "Helpers/TransferHelper.h"

#include <vector>
#include <map>
#include <array>

struct CommonResources;

#include <assimp/scene.h>

inline Matrixy2x3 operator*(const float factor, const Matrixy2x3 &matrix) { return { matrix.m11 * factor, matrix.m12 * factor, matrix.m21 * factor, matrix.m22 * factor, matrix.dx * factor, matrix.dy * factor }; }
// Only multiply the offsets, because rotations and other shit don't depend of exact values
inline Matrixy2x3 operator*(const Matrixy2x3 &matrix, const DOUBLE2 &dims) { return { matrix.m11, matrix.m12, matrix.m21, matrix.m22, (float)(matrix.dx * dims.x), (float)(matrix.dy * dims.y) }; } 
inline Matrixy2x3 operator*(const DOUBLE2 &dims, const Matrixy2x3 &matrix) { return matrix * dims; }

inline Matrixy2x3 operator-(const Matrixy2x3 &left, const Matrixy2x3 &right) { return { left.m11 - right.m11, left.m12 - right.m12, left.m21 - right.m21, left.m22 - right.m22, left.dx - right.dx, left.dy - right.dy }; }
inline Matrixy2x3 operator+(const Matrixy2x3 &left, const Matrixy2x3 &right) { return { left.m11 + right.m11, left.m12 + right.m12, left.m21 + right.m21, left.m22 + right.m22, left.dx + right.dx, left.dy + right.dy }; }

namespace Animation
{
	namespace Interpolation
	{
		enum INTERPOLATE : char
		{
			INTERPOLATE_LINEAR, // f = t
			INTERPOLATE_SMOOTH_IN_OUT, // Less values when t is near 0.5, more values when t < 0.35 and t > 0.65
			INTERPOLATE_SMOOTH_IN_FAST_OUT, // t = 2/3, f = 0.5 (weight = 0, 0.1, 1)
			INTERPOLATE_FAST_IN_SMOOTH_OUT, // t = 1/3, f = 0.5 (weights = 0, 0.9, 1)
			INTERPOLATE_CUSTOM
		};
	}

	namespace Loading
	{
		struct LoadedAnimation
		{
			std::string Name;
			std::string Type;
			Interpolation::INTERPOLATE Intertype;

			std::vector<std::vector<double>> Frames;

			bool Load(std::string name, std::map<std::string, std::vector<std::string>> from);
		};

		struct LoadedAnimations
		{
			std::map<std::string, std::vector<LoadedAnimation>> Animations;

			bool Load(std::string filename);
		};
	}

	namespace Bezier
	{
		template<size_t weight_count>
		using Bezier1D = std::array<double, weight_count>;
		using LinearBezier1D = Bezier1D<2>;
		using QuadBezier1D = Bezier1D<3>;
		using CubicBezier1D = Bezier1D<4>;

		const CubicBezier1D Smooth = { 0.0, 0.1, 0.9, 1.0 };
		const QuadBezier1D FastOut = { 0.0, 0.1, 1.0 };
		const QuadBezier1D FastIn = { 0.0, 0.9, 1.0 };

		template<size_t weight_count>
		using Bezier2D = std::array<DOUBLE2, weight_count>;
		using LinearBezier2D = Bezier2D<2>;
		using QuadBezier2D = Bezier2D<3>;
		using CubicBezier2D = Bezier2D<4>;

		template<size_t weight_count>
		using Bezier3D = std::array<DOUBLE3, weight_count>;
		using LinearBezier3D = Bezier3D<2>;
		using QuadBezier3D = Bezier3D<3>;
		using CubicBezier3D = Bezier3D<4>;

		struct BinomialNumbers
		{
			BinomialNumbers();
			std::vector<std::vector<UINT>> CoEfficients;
			double operator() (UINT n, UINT k);
		};

		extern BinomialNumbers Binomial;

		template<size_t weight_count>
		inline double GetBezier1D(UINT n, double t, Bezier1D<weight_count> weights)
		{
			if (n == 0)
			{
				DWARNING("given n = 0, this is mathematically possible, however I can imagine no use of a 0th degree polynomial");
				return 1.0;
			}
			if (weight_count <= n)
			{
				DWARNING("given too little weights for given polynomial degree \n");
				return 0.0;
			}

			Bezier1D<weight_count> &w = weights;
			if (n == 1)
			{
				DWARNING("given n = 1, not undefined behaviour, but usually beziers are only used when n > 1");
				return w[0] * (1.0 - t);
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
		double GetBezier1D(UINT n, double t, std::vector<double> weights);

		template<size_t weight_count>
		inline DOUBLE2 GetBezier2D(UINT n, double t, Bezier2D<weight_count> weights)
		{
			if (n == 0)
			{
				DWARNING("given n = 0");
				return { 0.0, 0.0 };
			}
			if (weight_count <= n)
			{
				DWARNING("given too little weights for given polynomial degree");
				return { 0.0, 0.0 };
			}

			Bezier2D<weight_count> &w = weights;
			if (n == 1)
			{
				DWARNING("given = 1, not bad behaviour, however use of beziers with degrees below 2 are unknown");
				return { w[0].x * t, w[0].y * t };
			}
			if (n == 2)
			{
				double t2 = t * t;
				double mt = 1.0 - t;
				double mt2 = mt * mt;
				double &term1 = mt; // 1 * t^3
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
		DOUBLE2 GetBezier2D(UINT n, double t, std::vector<DOUBLE2> weights);
	}

	namespace Interpolation
	{
		double Interpolate1D(double at, INTERPOLATE type);
		inline double InterpolateLinear1D(double at) { return fmin(1.0, fmax(0.0, at)); };
		double InterpolateSmooth1D(double at);
		double InterpolateFastIn1D(double at);
		double InterpolateFastOut1D(double at);

		struct QuaternionInterpolator
		{
			aiQuaternion operator() (aiQuaternion p1, aiQuaternion p2, double fac)
			{
				aiQuaternion out;
				aiQuaternion::Interpolate(out, p1, p2, (ai_real)fac);
				out = out.Normalize();
				return out;
			}

			floaty4 operator() (floaty4 p1, floaty4 p2, double fac)
			{
				return (floaty4)btQuaternion{ p1.x, p1.y, p1.z, p1.w }.slerp(btQuaternion{ p2.x, p2.y, p2.z, p2.w }, btScalar(fac));
			}
		};

		template<class DataType = double>
		struct LinearInterpolator
		{
			DataType operator() (DataType p1, DataType p2, double time) 
			{ 
				double factor = Interpolate1D(time, INTERPOLATE_LINEAR);
				return (1 - factor) * p1 + factor * p2;
			}
		};
	}

	namespace Framing
	{
		template<class T>
		struct Frames;
		template<class T>
		struct Frame;
		struct OpacityAnimator;
		struct ColorAnimator;

		enum FrameInfo
		{
			NONE						= 0,
			FRAME_ANIMATION_ENDED		= 1,
			FRAME_ANIMATION_NOT_STARTED = 2,

		};

		template<class T>
		struct Frame
		{
			T Data;
			double Time;
		};

		template<class T>
		struct InterpolatedFrame
		{
			T Data;
			UINT Info = 0;   // Combinations of the FrameInfo enumeration
		};

		template<class T>
		struct IFrames : Listener::IResizeListener
		{
			typedef T FrameType;
			IFrames(Interpolation::INTERPOLATE inter = Interpolation::INTERPOLATE_LINEAR, std::vector<Frame<FrameType>> frames = std::vector<Frame<FrameType>>()) : Interpolation(inter), Frames(frames) { if (frames.size()) CachedFrame = { frames[0].Data, FRAME_ANIMATION_NOT_STARTED }; }
			Interpolation::INTERPOLATE Interpolation;
			virtual InterpolatedFrame<FrameType> GetAt(double time) = 0;
			virtual bool Receive(Events::IEvent *event);
			virtual ~IFrames() {};
		protected:
			InterpolatedFrame<FrameType> CachedFrame;
			std::vector<Frame<FrameType>> PixelFrames;
			std::vector<Frame<FrameType>> Frames;
		};
		template<class T>
		inline bool IFrames<T>::Receive(Events::IEvent * event)
		{
			if (event->Type != Events::Type::ResizeWindowEvent)
				return false;

			if (auto *revent = Events::ConvertEvent<Event::ResizeEvent>(event))
			{
				// Resize the PixelFrames with new dimensions
				auto &dim = revent->Dimensions;
				if (PixelFrames.size() != Frames.size())
					PixelFrames.resize(Frames.size());

				for (size_t i = Frames.size(); i-- > 0; )
				{
					PixelFrames[i].Data = Frames[i].Data * dim;
					PixelFrames[i].Time = Frames[i].Time;
				}

				return true;
			}
			else
			{
				DERROR("given incorrect event");
				return false;
			}
		}

		struct PositionFrames : IFrames<DOUBLE4>
		{
			PositionFrames(Interpolation::INTERPOLATE inter, std::vector<Frame<DOUBLE4>> frames, CommonResources *resources);
			virtual InterpolatedFrame<FrameType> GetAt(double time) override;
			virtual ~PositionFrames() {};
		};

		struct WorldFrames2D : IFrames<Matrixy2x3>
		{
			//WorldFrames2D() {};
			WorldFrames2D(Interpolation::INTERPOLATE inter, std::vector<Frame<Matrixy2x3>> frames, CommonResources *resources);
			void Init(Interpolation::INTERPOLATE inter, std::vector<Frame<Matrixy2x3>> frames, CommonResources *resources);
			virtual InterpolatedFrame<FrameType> GetAt(double time) override;
			virtual ~WorldFrames2D() {};
		};
		
	}

	struct IAnimator
	{
		GameTimer AnimTimer;

		virtual void Start(std::string name = "Default") = 0;
		inline virtual void Stop() { AnimTimer.Stop(); }
		inline virtual void Continue() { AnimTimer.Start(); }
		virtual void Load(std::vector<Loading::LoadedAnimation> &Animations, CommonResources *resources) = 0;

		virtual ~IAnimator() {};
	};
	
	struct TransformAnimator2D : IAnimator
	{
		TransformAnimator2D() : CurrentAnimation(nullptr) {};
		TransformAnimator2D(std::map<std::string, Framing::WorldFrames2D> animations) : Animations(animations), CurrentAnimation(nullptr) {};
		TransformAnimator2D(Interpolation::INTERPOLATE inter, std::vector<Framing::Frame<Matrixy2x3>> frames, CommonResources *resources) : CurrentAnimation(nullptr) { Animations.emplace(std::make_pair("Default", Framing::WorldFrames2D(inter, frames, resources))); };
		void Load(std::vector<Loading::LoadedAnimation> &Animations, CommonResources * resources) override;
		std::map<std::string, Framing::WorldFrames2D> Animations;
		Framing::WorldFrames2D *CurrentAnimation;
		void Start(std::string name = "Default") override;
		Matrixy2x3 GetMatrix();
	};
}