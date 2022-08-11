#pragma once

#include "Helpers/TransferHelper.h"
#include "Helpers/VectorHelper.h"

#include "IParticles.h"

#include <unordered_map>

namespace Drawing
{
	struct SDLImage;
}

namespace Particles
{
	constexpr size_t InitialParticlePoolSize = 32u;
	
	
	// Particle pool designed for easy and fast external access
	class ExternalUpdateParticlePool
	{
		friend class ParticleManager;
		friend class ExternalUpdateParticleManager;
	public:
		ExternalUpdateParticlePool(std::shared_ptr<Drawing::SDLImage> texture, size_t initialsize = InitialParticlePoolSize);
		~ExternalUpdateParticlePool();

		size_t AddParticle(BasicParticle particle);
		void RemoveParticle(size_t key);

		std::vector<size_t> AddParticles(std::vector<BasicParticle> particles);
		void RemoveParticles(std::vector<size_t> keys);
		
		BasicParticle *GetParticle(size_t key);
		const BasicParticle *GetParticle(size_t key) const;

		std::vector<BasicParticle *> GetParticles(std::vector<size_t> keys);
		std::vector<const BasicParticle *> GetParticles(std::vector<size_t> keys) const;

	private:
		void Expand(size_t minincrease);

		std::shared_ptr<Drawing::SDLImage> m_Texture;
		std::vector<BasicParticle> m_Particles;
		std::unordered_map<size_t, size_t> m_KeyToIndex;
		std::unordered_map<size_t, size_t> m_IndexToKey;
		size_t m_Active = 0ull;
		size_t m_NextKey = 1ull;
	};

	class ExternalUpdateParticleManager
	{
		friend class ParticleManager;
	public:
		ExternalUpdateParticleManager();
		~ExternalUpdateParticleManager();

		size_t AddParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle particle);
		void RemoveParticle(Drawing::SDLImage *texture, size_t key);

		std::vector<size_t> AddParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> particles);
		void RemoveParticles(Drawing::SDLImage *texture, std::vector<size_t> keys);

		BasicParticle *GetParticle(Drawing::SDLImage *texture, size_t key);
		const BasicParticle *GetParticle(Drawing::SDLImage *texture, size_t key) const;

		std::vector<BasicParticle *> GetParticles(Drawing::SDLImage *texture, std::vector<size_t> keys);
		std::vector<const BasicParticle *> GetParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const;

	private:

		std::unordered_map<Drawing::SDLImage *, ExternalUpdateParticlePool> m_Pools;
	};

	// Once its in this particle pool it will only be removed via lifetime or pool destruction
	class VelocityParticlePool
	{
		friend class ParticleManager;
		friend class VelocityParticleManager;
	public:
		VelocityParticlePool(std::shared_ptr<Drawing::SDLImage> image, size_t initialsize = InitialParticlePoolSize);

		void AddParticle(BasicParticle basic, VelocityParticleData props);
		void AddParticles(std::vector<BasicParticle> basics, std::vector<VelocityParticleData> props);
		void AddParticles(std::vector<std::pair<BasicParticle, VelocityParticleData>> particles);

		void Update(float deltatime); // Updates position based on velocity, sets m_DeadParticleFlags if the new age is older than its lifetime, then calls cull()

	private:
		void Expand(size_t minincrease);
		void Remove(size_t index);
		void Cull(); // Remove all particles with a true m_DeadParticleFlags index, clears all m_DeadParticleFlags
		
		std::shared_ptr<Drawing::SDLImage> m_Texture;
		std::vector<BasicParticle> m_ParticlesDraw;
		std::vector<VelocityParticleData> m_ParticlesBehaviour;
		std::vector<bool> m_DeadParticleFlags;
		size_t m_ActiveCount = 0ull;
	};

	class VelocityParticleManager
	{
		friend class ParticleManager;
	public:

		void AddParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle basic, VelocityParticleData props);
		void AddParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<BasicParticle, VelocityParticleData>> particles);
		void AddParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> basics, std::vector<VelocityParticleData> props);
		
		void Update(float deltatime);

	private:
		std::vector<std::pair<std::shared_ptr<Drawing::SDLImage>, VelocityParticlePool>> m_Pools;
	};

	struct ForceParticleData : VelocityParticleData
	{
		floaty3 Force;
	};

	class ForceParticlePool
	{
		// I can't be screwed right now
	};

	struct DrawGuy;

	struct ParticleEmitter
	{
		virtual void EmitParticles(size_t count) = 0;
	};

	struct CubeEmitter : ParticleEmitter
	{
		CubeEmitter(floaty3 dimensions, floaty3 origin);
		~CubeEmitter();

		virtual void EmitParticles(size_t count) override;

	protected:

		floaty3 m_Dimensions;
		floaty3 m_Origin;
	};

	class ParticleManager : virtual FullResourceHolder
	{
	public:
		ParticleManager(CommonResources *resources, std::shared_ptr<Drawing::SDLImage> defaulttexture);
		~ParticleManager();

		// Modifiers
		size_t AddExternalParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle particle);
		std::vector<size_t> AddExternalParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> particles);
		void RemoveExternalParticle(Drawing::SDLImage *texture, size_t key);
		void RemoveExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys);
		
		void AddVelocityParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle basic, VelocityParticleData velodat);
		void AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> basic, std::vector<VelocityParticleData> props);
		void AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<BasicParticle, VelocityParticleData>> particles);

		// Observers

		BasicParticle *GetExternalParticle(Drawing::SDLImage *texture, size_t key);
		std::vector<BasicParticle *> GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> key);

		const BasicParticle *GetExternalParticle(Drawing::SDLImage *texture, size_t key) const;
		std::vector<const BasicParticle *> GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const;

		// Frame operations

		void DrawParticles();
		void Update(float deltatime);

	private:

		std::unique_ptr<DrawGuy> m_DrawGuy;

		VelocityParticleManager m_VelocityParticles;
		ExternalUpdateParticleManager m_ExternallyManagedParticles;

	};
}