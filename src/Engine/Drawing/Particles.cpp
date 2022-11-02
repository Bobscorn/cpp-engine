#include "Particles.h"

#include "Drawing/Image.h"

#include "Helpers/GLHelper.h"

namespace Particles
{
	struct BillboardVertex
	{
		floaty3 PosL;
		floaty2 Tex;
	};

	struct ViewProjBuffer
	{
		Matrixy4x4 View;
		Matrixy4x4 Proj;
	};

	struct DrawGuy : FullResourceHolder
	{
		DrawGuy(CommonResources *resources, std::shared_ptr<Drawing::SDLImage> defaulttex) 
			: FullResourceHolder(resources)
			, m_Vertices{ { {-1.f, -0.5f, -2.f}, {0.f, 1.f} }, { {1.f, -0.5f, -2.f}, {1.f, 1.f} }, { {1.f, 0.5f, -2.f}, {1.f, 0.f} }, { {-1.f, 0.5f, -2.f}, {0.f, 0.f} } }
			, m_ParticleProgram(CreateParticleProgram())
			, m_SamplerLoc(BindSampler())
			, m_TexScaleLoc(GetTexScale())
			, m_VBO(CreateVertexBuffer())
			, m_VAO(CreateVertexArray())
			, m_InstanceBuffer(CreateInstanceBuffer(InitialInstanceSize))
			, m_DefaultTex(defaulttex)
		{}

		// Must be called before a Draw of the same frame
		void UpdateViewProj(Matrixy4x4 View, Matrixy4x4 Proj);

		void DrawE(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> particles, size_t count) { Draw(texture, particles, count); }
		void Draw(std::shared_ptr<Drawing::SDLImage> texture, const std::vector<BasicParticle> &particles, size_t count);

	private:
		static constexpr size_t InitialInstanceSize = 32u;
		static constexpr GLint m_ViewProjLocation = 1u;

		GLProgram CreateParticleProgram();

		GLint BindSampler();
		GLint GetTexScale();

		GLBuffer CreateInstanceBuffer(size_t instancecount);
		GLBuffer CreateVertexBuffer();
		GLBuffer CreateViewProjBuffer();
		GLVertexArray CreateVertexArray();

		BillboardVertex m_Vertices[4];
		GLProgram m_ParticleProgram;
		GLint m_SamplerLoc = 0ull;
		GLint m_TexScaleLoc = 0ull;
		GLBuffer m_VBO;
		GLVertexArray m_VAO; // Requires created vertex
		GLBuffer m_InstanceBuffer;
		GLBuffer m_ViewProjBuffer;
		size_t m_InstanceSize = 0ull;
		size_t m_VBOSize = 0ull;

		std::shared_ptr<Drawing::SDLImage> m_DefaultTex;

		constexpr static GLuint SamplerName = 2u;
	};

	void DrawGuy::UpdateViewProj(Matrixy4x4 View, Matrixy4x4 Proj)
	{
		// Calculate new Vertex positions that align with camera frustum
		// Rotate the vertices by the inverse of the View matrix's rotation

		Matrixy3x3 InverseRot = Matrixy4x4::InvertedOrIdentity(View).As3x3();

		constexpr static BillboardVertex MansNotHot[4] = { { {-0.1f, -0.1f, 0.f}, {0.f, 1.f} }, { {0.1f, -0.1f, 0.f}, {1.f, 1.f} }, { {0.1f, 0.1f, 0.f}, {1.f, 0.f} }, { {-0.1f, 0.1f, 0.f}, {0.f, 0.f} } };

		BillboardVertex invertedverts[4];
		invertedverts[0].PosL = InverseRot.Transform(MansNotHot[0].PosL);
		invertedverts[0].Tex = MansNotHot[0].Tex;
		invertedverts[1].PosL = InverseRot.Transform(MansNotHot[1].PosL);
		invertedverts[1].Tex = MansNotHot[1].Tex;
		invertedverts[2].PosL = InverseRot.Transform(MansNotHot[2].PosL);
		invertedverts[2].Tex = MansNotHot[2].Tex;
		invertedverts[3].PosL = InverseRot.Transform(MansNotHot[3].PosL);
		invertedverts[3].Tex = MansNotHot[3].Tex;

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO.Get());
		glBufferSubData(GL_ARRAY_BUFFER, NULL, sizeof(BillboardVertex) * 4, (GLvoid *)invertedverts);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		ViewProjBuffer viewproj = { View, Proj };
		glBindBuffer(GL_UNIFORM_BUFFER, m_ViewProjBuffer.Get());
		glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(ViewProjBuffer), (GLvoid *)&viewproj);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void DrawGuy::Draw(std::shared_ptr<Drawing::SDLImage> texture, const std::vector<BasicParticle> &particles, size_t count)
	{
		if (!count)
			return;
		if (count > m_InstanceSize)
		{
			m_InstanceBuffer = CreateInstanceBuffer(particles.size());
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_InstanceBuffer.Get());
		glBufferSubData(GL_ARRAY_BUFFER, NULL, sizeof(BasicParticle) * count, (GLvoid *)particles.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (texture)
		{
			texture->LoadGL();
			glActiveTexture(GL_TEXTURE0 + SamplerName);
			glBindTexture(GL_TEXTURE_2D, texture->Get());
			glUniform1i(m_SamplerLoc, SamplerName);
			glUniform2f(m_TexScaleLoc, texture->GetWidthScale(), texture->GetHeightScale());
		}
		else
		{
			m_DefaultTex->LoadGL();
			glActiveTexture(GL_TEXTURE0 + SamplerName);
			glBindTexture(GL_TEXTURE_2D, m_DefaultTex->Get());
			glUniform1i(m_SamplerLoc, SamplerName);
			glUniform2f(m_TexScaleLoc, m_DefaultTex->GetWidthScale(), m_DefaultTex->GetHeightScale());
		}

		glBindVertexArray(m_VAO.Get());

		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)count);

		glBindVertexArray(0);
	}

	GLProgram DrawGuy::CreateParticleProgram()
	{
		return GLProgram(
			{
				{"Shaders/Particles.glvs", GL_VERTEX_SHADER},
				{"Shaders/Particles.glfs", GL_FRAGMENT_SHADER}
			}
		);
	}
	GLint DrawGuy::BindSampler()
	{
		return glGetUniformLocation(m_ParticleProgram.Get(), "ParticleTexture");
	}
	GLint DrawGuy::GetTexScale()
	{
		return glGetUniformLocation(m_ParticleProgram.Get(), "TextureScale");
	}
	GLBuffer DrawGuy::CreateInstanceBuffer(size_t instancecount)
	{
		m_InstanceSize = instancecount;
		GLuint out = 0;
		glGenBuffers(1, &out);

		glBindBuffer(GL_ARRAY_BUFFER, out);
		glBufferData(GL_ARRAY_BUFFER, sizeof(BasicParticle) * instancecount, NULL, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, out);
		glBindVertexArray(m_VAO.Get());
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(floaty3), (GLvoid *)offsetof(BasicParticle, BasicParticle::Position));
		glVertexAttribDivisor(2, 1);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(floaty4), (GLvoid *)offsetof(BasicParticle, BasicParticle::Color));
		glVertexAttribDivisor(3, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		return out;
	}
	GLBuffer DrawGuy::CreateVertexBuffer()
	{
		GLuint out = 0;
		glGenBuffers(1, &out);
		glBindBuffer(GL_ARRAY_BUFFER, out);
		glBufferData(GL_ARRAY_BUFFER, sizeof(BillboardVertex) * 4, (GLvoid *)m_Vertices, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERR("Creating Vertex Buffer");

		return out;
	}
	GLBuffer DrawGuy::CreateViewProjBuffer()
	{
		GLint index = glGetUniformBlockIndex(m_ParticleProgram.Get(), "ViewProj");
		glUniformBlockBinding(m_ParticleProgram.Get(), index, m_ViewProjLocation);
		CHECK_GL_ERR("Binding ViewProj Buffer");

		GLuint out = 0;
		glGenBuffers(1, &out);
		glBindBuffer(GL_UNIFORM_BUFFER, out);
		glBindBufferBase(GL_UNIFORM_BUFFER, m_ViewProjLocation, out);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ViewProjBuffer), NULL, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		CHECK_GL_ERR("Initializing ViewProj Buffer");

		return out;

	}
	GLVertexArray DrawGuy::CreateVertexArray()
	{
		GLuint vao = 0;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vao);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(floaty3), (GLvoid*)offsetof(BillboardVertex, BillboardVertex::PosL));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(floaty2), (GLvoid *)offsetof(BillboardVertex, BillboardVertex::Tex));
		
		glBindVertexArray(0);

		return vao;
	}
}

Particles::VelocityParticlePool::VelocityParticlePool(std::shared_ptr<Drawing::SDLImage> texture, size_t initialsize)
	: m_Texture(std::move(texture))
	, m_ParticlesDraw(initialsize, BasicParticle{})
	, m_ParticlesBehaviour(initialsize, VelocityParticleData{})
	, m_DeadParticleFlags(initialsize, false)
	, m_ActiveCount(0ull)
{

}

void Particles::VelocityParticlePool::AddParticle(BasicParticle basic, VelocityParticleData props)
{
	if (m_ActiveCount == m_ParticlesDraw.size())
	{
		Expand(1ull);
	}
	m_ParticlesDraw[m_ActiveCount] = basic;
	m_ParticlesBehaviour[m_ActiveCount] = props;
	++m_ActiveCount;
}

void Particles::VelocityParticlePool::AddParticles(std::vector<BasicParticle> basics, std::vector<VelocityParticleData> props)
{
	size_t count = (basics.size() < props.size() ? basics.size() : props.size());
	if (count != props.size() || count != basics.size())
		DWARNING("Wtf why you give me different size vectors you douche");
	if (m_ActiveCount + count > m_ParticlesDraw.size())
	{
		Expand(count);
	}
	std::copy(basics.begin(), basics.begin() + count, m_ParticlesDraw.begin() + m_ActiveCount);
	std::copy(props.begin(), props.begin() + count, m_ParticlesBehaviour.begin() + m_ActiveCount);
	std::fill(m_DeadParticleFlags.begin() + m_ActiveCount, m_DeadParticleFlags.begin() + m_ActiveCount + count, false);
	m_ActiveCount += count;
}

void Particles::VelocityParticlePool::AddParticles(std::vector<std::pair<BasicParticle, VelocityParticleData>> particles)
{
	size_t count = particles.size();
	if (m_ActiveCount + count > m_ParticlesDraw.size())
	{
		Expand(count);
	}
	for (size_t i = 0; i < count; ++i)
	{
		m_ParticlesDraw[m_ActiveCount + i] = particles[i].first;
		m_ParticlesBehaviour[m_ActiveCount + i] = particles[i].second;
	}
	std::fill(m_DeadParticleFlags.begin() + m_ActiveCount, m_DeadParticleFlags.begin() + m_ActiveCount + count, false);
	m_ActiveCount += count;
}

void Particles::VelocityParticlePool::Update(float deltatime)
{
	for (size_t i = m_ActiveCount; i-- > 0; )
	{
		auto &d = m_ParticlesDraw[i];
		auto &b = m_ParticlesBehaviour[i];

		b.Age += deltatime;
		if (b.Age > b.Lifetime)
		{
			m_DeadParticleFlags[i] = true;
			continue;
		}

		d.Position += b.Velocity * deltatime;
	}
	Cull();
}

void Particles::VelocityParticlePool::Expand(size_t minincrease)
{
	if (m_ParticlesDraw.size() < minincrease)
	{
		m_ParticlesDraw.resize(m_ParticlesDraw.size() + minincrease);
		m_ParticlesBehaviour.resize(m_ParticlesBehaviour.size() + minincrease);
		m_DeadParticleFlags.resize(m_DeadParticleFlags.size() + minincrease);
	}
	else
	{
		m_ParticlesDraw.resize(m_ParticlesDraw.size() * 2);
		m_ParticlesBehaviour.resize(m_ParticlesBehaviour.size() * 2);
		m_DeadParticleFlags.resize(m_DeadParticleFlags.size() * 2);
	}
}

void Particles::VelocityParticlePool::Remove(size_t index)
{
	size_t i = m_ActiveCount;
	bool live_particles = false;
	for (; i-- > 0; )
	{
		if (m_DeadParticleFlags[i] == false)
		{
			live_particles = true;
			break;
		}
	}

	if (live_particles)
	{
		std::swap(m_ParticlesDraw[index], m_ParticlesDraw[i]);
		std::swap(m_ParticlesBehaviour[index], m_ParticlesBehaviour[i]);
		auto tmp = m_DeadParticleFlags[index];
		m_DeadParticleFlags[index] = m_DeadParticleFlags[i];
		m_DeadParticleFlags[i] = tmp;
	}

	--m_ActiveCount;
}

void Particles::VelocityParticlePool::Cull()
{
	bool more = true;
	while (more)
	{
		for (size_t i = m_ActiveCount; i-- > 0; )
		{
			if (m_DeadParticleFlags[i])
				Remove(i);
		}
		bool gotem = true;
		if (!m_ActiveCount)
			gotem = false;
		else
		{
			for (size_t i = m_ActiveCount; i-- > 0; )
				if (m_DeadParticleFlags[i])
					gotem = false;
		}
		more |= gotem;
	}
}

void Particles::VelocityParticleManager::AddParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle basic, VelocityParticleData props)
{
	for (size_t i = m_Pools.size(); i-- > 0; )
	{
		if (m_Pools[i].first == texture)
		{
			m_Pools[i].second.AddParticle(basic, props);
			return;
		}
	}

	m_Pools.push_back(std::make_pair(texture, VelocityParticlePool{texture}));
	m_Pools.back().second.AddParticle(basic, props);
}

void Particles::VelocityParticleManager::AddParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<BasicParticle, VelocityParticleData>> particles)
{
	for (size_t i = m_Pools.size(); i-- > 0; )
	{
		if (m_Pools[i].first == texture)
		{
			m_Pools[i].second.AddParticles(particles);
			return;
		}
	}

	m_Pools.push_back(std::make_pair(texture, VelocityParticlePool{texture}));
	m_Pools.back().second.AddParticles(particles);
}

void Particles::VelocityParticleManager::AddParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> basics, std::vector<VelocityParticleData> props)
{
	for (size_t i = m_Pools.size(); i-- > 0; )
	{
		if (m_Pools[i].first == texture)
		{
			m_Pools[i].second.AddParticles(basics, props);
			return;
		}
	}

	m_Pools.push_back(std::make_pair(texture, VelocityParticlePool{ texture }));
	m_Pools.back().second.AddParticles(basics, props);
}

void Particles::VelocityParticleManager::Update(float deltatime)
{
	for (auto &pool : m_Pools)
		pool.second.Update(deltatime);
}


Particles::ExternalUpdateParticlePool::ExternalUpdateParticlePool(std::shared_ptr<Drawing::SDLImage> texture, size_t initialsize)
	: m_Texture(texture)
	, m_Particles(initialsize, BasicParticle{})
	, m_KeyToIndex()
	, m_Active(0ull)
{
	m_KeyToIndex.reserve(initialsize);
}

Particles::ExternalUpdateParticlePool::~ExternalUpdateParticlePool()
{
}

size_t Particles::ExternalUpdateParticlePool::AddParticle(BasicParticle particle)
{
	if (m_Active == m_Particles.size())
	{
		Expand(1ull);
	}

	m_Particles[m_Active] = particle;
	
	m_KeyToIndex[m_NextKey] = m_Active;
	m_IndexToKey[m_Active] = m_NextKey;

	++m_Active;

	return m_NextKey++;
}

void Particles::ExternalUpdateParticlePool::RemoveParticle(size_t key)
{
	auto it = m_KeyToIndex.find(key);
	if (it == m_KeyToIndex.end())
		return;
	
	size_t index = it->second;
	if (index == m_Active - 1)
	{
		// If the one being removed is at the back
		--m_Active;
		m_KeyToIndex.erase(key);
		m_IndexToKey.erase(it->second);

		return;
	}

	auto backit = m_IndexToKey.find(m_Active - 1);
	if (backit == m_IndexToKey.end())
		return;

	size_t swappedkey = backit->second;

	std::swap(m_Particles[it->second], m_Particles.back());
	m_KeyToIndex.erase(key);
	m_IndexToKey.erase(index);
	m_KeyToIndex[swappedkey] = index;
	m_IndexToKey[index] = swappedkey;
}

std::vector<size_t> Particles::ExternalUpdateParticlePool::AddParticles(std::vector<BasicParticle> particles)
{
	if (m_Active + particles.size() > m_Particles.size())
		Expand(particles.size());

	std::copy(particles.begin(), particles.end(), m_Particles.begin() + m_Active);
	
	std::vector<size_t> keys{ particles.size(), 0ull };
	for (size_t i = 0; i < particles.size(); ++i)
	{
		m_KeyToIndex[m_NextKey] = m_Active + i;
		m_IndexToKey[m_Active + i] = m_NextKey;
		keys[i] = m_NextKey;
		++m_NextKey;
	}

	m_Active += particles.size();

	return keys;
}

void Particles::ExternalUpdateParticlePool::RemoveParticles(std::vector<size_t> keys)
{
	for (auto &key : keys)
		RemoveParticle(key);
}

Particles::BasicParticle *Particles::ExternalUpdateParticlePool::GetParticle(size_t key)
{
	auto it = m_KeyToIndex.find(key);
	if (it == m_KeyToIndex.end())
		return nullptr;
	return &m_Particles[it->second];
}

const Particles::BasicParticle *Particles::ExternalUpdateParticlePool::GetParticle(size_t key) const
{
	auto it = m_KeyToIndex.find(key);
	if (it == m_KeyToIndex.end())
		return nullptr;
	return &m_Particles[it->second];
}

std::vector<Particles::BasicParticle *> Particles::ExternalUpdateParticlePool::GetParticles(std::vector<size_t> keys)
{
	std::vector<Particles::BasicParticle *> out{ keys.size(), nullptr };
	for (size_t i = 0; i < keys.size(); ++i)
		out[i] = GetParticle(keys[i]);
	return out;
}

std::vector<const Particles::BasicParticle *> Particles::ExternalUpdateParticlePool::GetParticles(std::vector<size_t> keys) const
{
	std::vector<const Particles::BasicParticle *> out{ keys.size(), nullptr };
	for (size_t i = 0; i < keys.size(); ++i)
		out[i] = GetParticle(keys[i]);
	return out;
}

void Particles::ExternalUpdateParticlePool::Expand(size_t minincrease)
{
	if (m_Particles.size() < minincrease)
	{
		m_Particles.resize(m_Particles.size() + minincrease);
	}
	else
	{
		m_Particles.resize(m_Particles.size() * 2);
	}
}

Particles::ExternalUpdateParticleManager::ExternalUpdateParticleManager()
{
}

Particles::ExternalUpdateParticleManager::~ExternalUpdateParticleManager()
{
}

size_t Particles::ExternalUpdateParticleManager::AddParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle particle)
{
	if (!texture)
		return 0ull;
	auto it = m_Pools.find(texture.get());
	if (it != m_Pools.end())
	{
		return it->second.AddParticle(particle);
	}

	return (m_Pools.try_emplace(texture.get(), ExternalUpdateParticlePool{ texture })).first->second.AddParticle(particle);
}

void Particles::ExternalUpdateParticleManager::RemoveParticle(Drawing::SDLImage *texture, size_t key)
{
	auto it = m_Pools.find(texture);
	if (it != m_Pools.end())
		it->second.RemoveParticle(key);
}

std::vector<size_t> Particles::ExternalUpdateParticleManager::AddParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> particles)
{
	auto it = m_Pools.find(texture.get());
	if (it != m_Pools.end())
		return it->second.AddParticles(particles);

	return (m_Pools.try_emplace(texture.get(), ExternalUpdateParticlePool{ texture })).first->second.AddParticles(particles);
}

void Particles::ExternalUpdateParticleManager::RemoveParticles(Drawing::SDLImage *texture, std::vector<size_t> keys)
{
	auto it = m_Pools.find(texture);
	if (it != m_Pools.end())
		it->second.RemoveParticles(keys);
}

Particles::BasicParticle *Particles::ExternalUpdateParticleManager::GetParticle(Drawing::SDLImage *texture, size_t key)
{
	auto it = m_Pools.find(texture);
	if (it != m_Pools.end())
		return it->second.GetParticle(key);

	return nullptr;
}

const Particles::BasicParticle *Particles::ExternalUpdateParticleManager::GetParticle(Drawing::SDLImage *texture, size_t key) const
{
	auto it = m_Pools.find(texture);
	if (it != m_Pools.end())
		return it->second.GetParticle(key);

	return nullptr;
}

std::vector<Particles::BasicParticle *> Particles::ExternalUpdateParticleManager::GetParticles(Drawing::SDLImage *texture, std::vector<size_t> keys)
{
	auto it = m_Pools.find(texture);
	if (it != m_Pools.end())
		return it->second.GetParticles(keys);

	return std::vector<Particles::BasicParticle *>{keys.size(), nullptr};
}

std::vector<const Particles::BasicParticle *> Particles::ExternalUpdateParticleManager::GetParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const
{
	auto it = m_Pools.find(texture);
	if (it != m_Pools.end())
		return it->second.GetParticles(keys);

	return std::vector<const Particles::BasicParticle *>{keys.size(), nullptr};
}

Particles::ParticleManager::ParticleManager(CommonResources *resources, std::shared_ptr<Drawing::SDLImage> defaulttex) : FullResourceHolder(resources), m_DrawGuy(std::make_unique<DrawGuy>(resources, defaulttex))
{
}

Particles::ParticleManager::~ParticleManager()
{
}

size_t Particles::ParticleManager::AddExternalParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle particle)
{
	return m_ExternallyManagedParticles.AddParticle(texture, particle);
}

std::vector<size_t> Particles::ParticleManager::AddExternalParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> particles)
{
	return m_ExternallyManagedParticles.AddParticles(texture, particles);
}

void Particles::ParticleManager::RemoveExternalParticle(Drawing::SDLImage *texture, size_t key)
{
	m_ExternallyManagedParticles.RemoveParticle(texture, key);
}

void Particles::ParticleManager::RemoveExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys)
{
	m_ExternallyManagedParticles.RemoveParticles(texture, keys);
}

void Particles::ParticleManager::AddVelocityParticle(std::shared_ptr<Drawing::SDLImage> texture, BasicParticle basic, VelocityParticleData velodat)
{
	m_VelocityParticles.AddParticle(texture, basic, velodat);
}

void Particles::ParticleManager::AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<BasicParticle> basic, std::vector<VelocityParticleData> props)
{
	m_VelocityParticles.AddParticles(texture, basic, props);
}

void Particles::ParticleManager::AddVelocityParticles(std::shared_ptr<Drawing::SDLImage> texture, std::vector<std::pair<BasicParticle, VelocityParticleData>> particles)
{
	m_VelocityParticles.AddParticles(texture, particles);
}

Particles::BasicParticle *Particles::ParticleManager::GetExternalParticle(Drawing::SDLImage *texture, size_t key)
{
	return m_ExternallyManagedParticles.GetParticle(texture, key);
}

std::vector<Particles::BasicParticle *> Particles::ParticleManager::GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys)
{
	return m_ExternallyManagedParticles.GetParticles(texture, keys);
}

const Particles::BasicParticle *Particles::ParticleManager::GetExternalParticle(Drawing::SDLImage *texture, size_t key) const
{
	return m_ExternallyManagedParticles.GetParticle(texture, key);
}

std::vector<const Particles::BasicParticle *> Particles::ParticleManager::GetExternalParticles(Drawing::SDLImage *texture, std::vector<size_t> keys) const
{
	return m_ExternallyManagedParticles.GetParticles(texture, keys);
}

void Particles::ParticleManager::DrawParticles()
{
	for (auto &pool : m_VelocityParticles.m_Pools)
		m_DrawGuy->Draw(pool.first, pool.second.m_ParticlesDraw, pool.second.m_ActiveCount);

	for (auto &pool : m_ExternallyManagedParticles.m_Pools)
		m_DrawGuy->Draw(pool.second.m_Texture, pool.second.m_Particles, pool.second.m_Active);
}

void Particles::ParticleManager::Update(float deltatime)
{
	m_VelocityParticles.Update(deltatime);
}
