#pragma once

#include <memory>
#include <vector>

namespace Drawing
{
	struct SDLImage;
};

namespace Particles
{
	struct BasicParticle
	{
		floaty3 Position;
		floaty4 Color;
	};


	struct VelocityParticleData
	{
		floaty3 Velocity = { 0.f, 0.f, 0.f };
		float Age = 0.f;
		float Lifetime = 0.f;
	};

	struct IParticleDrawer
	{
		virtual size_t AddExternalParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle particle) = 0;
		virtual std::vector<size_t> AddExternalParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> particles) = 0;
		virtual void RemoveExternalParticle(Drawing::SDLImage *texture, size_t key) = 0;
		virtual void RemoveExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) = 0;

		virtual void AddVelocityParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle basic, VelocityParticleData velodat) = 0;
		virtual void AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> basic, std::vector<VelocityParticleData> props) = 0;
		virtual void AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<BasicParticle, VelocityParticleData>> particles) = 0;

		// Observers

		virtual BasicParticle *GetExternalParticle(Drawing::SDLImage *texture, size_t key) = 0;
		virtual std::vector<BasicParticle *> GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> key) = 0;

		virtual const BasicParticle *GetExternalParticle(Drawing::SDLImage *texture, size_t key) const = 0;
		virtual std::vector<const BasicParticle *> GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const = 0;
	};
}