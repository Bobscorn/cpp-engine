#include "VoxelAbility.h"

#include "VoxelWorld.h"

inline std::string GetDamageTypeName(std::string prefix, Voxel::DamageType type)
{
	switch (type)
	{
	default:
	case Voxel::BASIC:
		return prefix + "_BASIC";
	case Voxel::PROJECTILE:
		return prefix + "_PROJECTILE";
	case Voxel::MAGIC:
		return prefix + "_MAGIC";
	case Voxel::EFFECT:
		return prefix + "_EFFECT";
	}
}

int Voxel::Entity::GetType()
{
	return m_Type;
}

void Voxel::Entity::AddEffect(std::string entry, std::unique_ptr<Voxel::Effect> effect)
{
	m_Effects["entry"][effect->GetTypeIndex()].emplace(std::move(effect));
}

void Voxel::Entity::RemoveEffect(std::string entry, std::string effect_name)
{
	auto it = m_Effects.find(entry);
	if (it != m_Effects.end())
	{
		bool done = false;
		while (!done)
		{
			done = true;
			for (auto it_2 = it->second.begin(); it_2 != it->second.end(); ++it_2)
				for (auto it_3 = it_2->second.begin(); it_3 != it_2->second.end(); ++it_3)
					if ((*it_3)->GetName() == effect_name)
					{
						done = false;
						it_2->second.erase(it_3);
						break;
					}

		}
	}
}

void Voxel::Entity::ClearEffects(std::string entry)
{
	m_Effects.erase(entry);
}

void DoDamageThing(Voxel::DamageDescription &desc, Voxel::Entity *other, Voxel::Entity::stupid_it it)
{
	(void)other;
	using namespace Voxel;
	auto type_it = it->second.find(typeid(PreDamageEffect));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
		{
			auto *effect = static_cast<Voxel::PreDamageEffect *>(effect_base.get());
			effect->Modify(desc);
		}

	type_it = it->second.find(typeid(TargettedEffect));
	if (type_it != it->second.end())
		for (auto &effect_p : type_it->second)
			static_cast<TargettedEffect *>(effect_p.get())->Activate(desc.Receiver);

	type_it = it->second.find(typeid(TriggeredEffect));
	if (type_it != it->second.end())
		for (auto &effect_p : type_it->second)
			static_cast<TriggeredEffect *>(effect_p.get())->Activate();
}

inline void DoPostDamageThing(Voxel::Entity *me, Voxel::Entity *other, Voxel::DamageDoneDescription desc, Voxel::Entity::stupid_it it)
{
	using namespace Voxel;
	auto type_it = it->second.find(typeid(PostDamageEffect));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
			static_cast<PostDamageEffect *>(effect_base.get())->Activate(me, other, desc);

	type_it = it->second.find(typeid(TargettedEffect));
	if (type_it != it->second.end())
		for (auto &effect_p : type_it->second)
			static_cast<TargettedEffect *>(effect_p.get())->Activate(other);

	type_it = it->second.find(typeid(TriggeredEffect));
	if (type_it != it->second.end())
		for (auto &effect_p : type_it->second)
			static_cast<TriggeredEffect *>(effect_p.get())->Activate();
}

inline void DoDamageTakenThing(Voxel::Entity *from, Voxel::Entity *to, Voxel::DamageDoneDescription &desc, Voxel::Entity::stupid_it it)
{
	using namespace Voxel;
	auto type_it = it->second.find(typeid(OnDamagedEffect));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
			static_cast<OnDamagedEffect *>(effect_base.get())->Modify(from, to, desc);

	type_it = it->second.find(typeid(TargettedEffect));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
			static_cast<TargettedEffect *>(effect_base.get())->Activate(from);

	type_it = it->second.find(typeid(TriggeredEffect));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
			static_cast<TriggeredEffect *>(effect_base.get())->Activate();
}

inline void DoAbilityCastThing(Voxel::AbilityCastDescription &desc, Voxel::Entity::stupid_it it)
{
	using namespace Voxel;
	auto type_it = it->second.find(typeid(OnAbilityCast));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
			static_cast<OnAbilityCast *>(effect_base.get())->OnCast(desc);

	if (desc.TargetEntity)
	{
		type_it = it->second.find(typeid(TargettedEffect));
		if (type_it != it->second.end())
			for (auto &effect_base : type_it->second)
				static_cast<TargettedEffect *>(effect_base.get())->Activate(desc.TargetEntity);
	}

	type_it = it->second.find(typeid(TriggeredEffect));
	if (type_it != it->second.end())
		for (auto &effect_base : type_it->second)
			static_cast<TriggeredEffect *>(effect_base.get())->Activate();
}

void Voxel::Entity::DoPreDamageEffects(DamageDescription &desc)
{
	auto it = m_Effects.find("PreDamage");
	if (it != m_Effects.end())
	{
		DoDamageThing(desc, desc.Receiver, it);
	}

	it = m_Effects.find(GetDamageTypeName("PreDamage", desc.Type));
	if (it != m_Effects.end())
	{
		DoDamageThing(desc, desc.Receiver, it);
	}
}

void Voxel::Entity::DoPostDamageEffects(Entity *receiver, const DamageDoneDescription &desc, DamageType dam_type)
{
	auto it = m_Effects.find("PostDamage");
	if (it != m_Effects.end())
	{
		DoPostDamageThing(this, receiver, desc, it);
	}

	it = m_Effects.find(GetDamageTypeName("PostDamage", dam_type));
	if (it != m_Effects.end())
	{
		DoPostDamageThing(this, receiver, desc, it);
	}
}

void Voxel::Entity::DoDamageTakenEffects(Entity *attacker, DamageDoneDescription &desc, DamageType type)
{
	auto it = m_Effects.find("OnDamaged");
	if (it != m_Effects.end())
		DoDamageTakenThing(attacker, this, desc, it);

	it = m_Effects.find(GetDamageTypeName("OnDamaged", type));
	if (it != m_Effects.end())
		DoDamageTakenThing(attacker, this, desc, it);
}

void Voxel::Entity::Die()
{
	this->m_World->RemoveEntity(this);
}

void Voxel::Entity::CheckEffects()
{
	bool done = false;
	while (!done)
	{
		done = true;
		bool keep_going = true;
		for (auto it = m_Effects.begin(); it != m_Effects.end() && keep_going; ++it)
			for (auto it2 = it->second.begin(); it2 != it->second.end() && keep_going; ++it2)
				for (auto it3 = it2->second.begin(); it3 != it2->second.end() && keep_going; ++it3)
					if (!(*it3)->Keep())
					{
						it2->second.erase(it3);
						if (it2->second.empty())
							it->second.erase(it2);
						if (it->second.empty())
							m_Effects.erase(it);
						done = false;
						keep_going = false;
						break;
					}
	}
}

void Voxel::Entity::CastAbility(std::string ability_name)
{
	auto it = m_Abilities.find(ability_name);
	if (it == m_Abilities.end())
		return;

	AbilityCastDescription desc = it->second->CreateDescription();

	TriggerAbilityEffects(desc);

	it->second->Cast(desc);
}

void Voxel::Entity::TriggerAbilityEffects(AbilityCastDescription &desc)
{
	bool is_proj = desc.ProjectileCount;
	auto it = m_Effects.find("OnAbilityCast");
	if (it != m_Effects.end())
	{
		DoAbilityCastThing(desc, it);
	}

	if (desc.Magic)
	{
		it = m_Effects.find("OnAbilityCast_MAGIC");
		if (it != m_Effects.end())
			DoAbilityCastThing(desc, it);
	}

	if (is_proj)
	{
		it = m_Effects.find("OnAbilityCast_PROJECTILE");
		if (it != m_Effects.end())
			DoAbilityCastThing(desc, it);
	}

	if (!desc.Magic && !is_proj)
	{
		it = m_Effects.find("OnAbilityCast_NEITHER");
		if (it != m_Effects.end())
			DoAbilityCastThing(desc, it);
	}
}

void Voxel::Entity::Attack(Entity *other, float base_damage)
{
	DamageDescription desc(base_damage);
	desc.Dealer = this;
	desc.Receiver = other;
	desc.Type = BASIC;

	auto it = m_Effects.find("PreDamage");
	if (it != m_Effects.end())
	{
		DoDamageThing(desc, other, it);
	}

	it = m_Effects.find("PreDamage_BASIC");
	if (it != m_Effects.end())
	{
		DoDamageThing(desc, other, it);
	}

	auto post_damage = other->TakeDamage(desc, this);

	it = m_Effects.find("PostDamage");
	if (it != m_Effects.end())
	{
		DoPostDamageThing(this, other, post_damage, it);
	}

	it = m_Effects.find("PostDamage_BASIC");
	if (it != m_Effects.end())
		DoPostDamageThing(this, other, post_damage, it);
}

void Voxel::Entity::AbilityAttack(Entity *other, DamageDescription damage)
{
	auto it = m_Effects.find("PreDamage");
	if (it != m_Effects.end())
		DoDamageThing(damage, other, it);

	it = m_Effects.find(GetDamageTypeName("PreDamage", damage.Type));
	if (it != m_Effects.end())
		DoDamageThing(damage, other, it);

	auto damage_done = other->TakeDamage(damage, this);

	it = m_Effects.find("PostDamage");
	if (it != m_Effects.end())
		DoPostDamageThing(this, other, damage_done, it);

	it = m_Effects.find(GetDamageTypeName("PostDamage", damage.Type));
	if (it != m_Effects.end())
		DoPostDamageThing(this, other, damage_done, it);
}

Voxel::DamageDoneDescription Voxel::Entity::TakeDamage(DamageDescription desc, Voxel::Entity *from)
{
	Voxel::DamageDoneDescription out(desc.DamageToDo);
	
	DoDamageTakenEffects(from, out, desc.Type);

	this->TakeDamageDirect(out.DamageDone);

	return out;
}

bool Voxel::Entity::TakeDamageDirect(float amount)
{
	m_Health = (m_Health - amount < 0 ? 0 : m_Health - amount);

	if (m_Health <= 0.f)
		this->Die();

	return m_Health <= 0.f;
}

void Voxel::Entity::TakeKnockback(floaty3 knock, Entity *from)
{
	auto it = m_Effects.find("OnKnockedBack");
	if (it != m_Effects.end())
	{
		auto type_it = it->second.find(typeid(OnKnockedBackEffect));
		if (type_it != it->second.end())
			for (auto &effect_p : type_it->second)
				static_cast<OnKnockedBackEffect *>(effect_p.get())->Modify(from, this, knock);

		type_it = it->second.find(typeid(TargettedEffect));
		if (type_it != it->second.end())
			for (auto &effect_p : type_it->second)
				static_cast<TargettedEffect *>(effect_p.get())->Activate(from);

		type_it = it->second.find(typeid(TriggeredEffect));
		if (type_it != it->second.end())
			for (auto &effect_p : type_it->second)
				static_cast<TriggeredEffect *>(effect_p.get())->Activate();
	}

	TakeKnockbackDirect(knock);
}

void Voxel::Entity::Heal(float amount, Entity *healer)
{
	HealDescription desc(amount);
	desc.Healer = healer;
	desc.BeingHealed = this;

	auto it = m_Effects.find("OnHealed");
	if (it != m_Effects.end())
	{
		auto type_it = it->second.find(typeid(OnHealedEffect));
		if (type_it != it->second.end())
			for (auto &effect_p : type_it->second)
				static_cast<OnHealedEffect *>(effect_p.get())->Modify(desc);

		type_it = it->second.find(typeid(TargettedEffect));
		if (type_it != it->second.end())
			for (auto &effect_p : type_it->second)
				static_cast<TargettedEffect *>(effect_p.get())->Activate(healer);

		type_it = it->second.find(typeid(TriggeredEffect));
		if (type_it != it->second.end())
			for (auto &effect_p : type_it->second)
				static_cast<TriggeredEffect *>(effect_p.get())->Activate();
	}

	HealDirect(desc.HealToDo);
}

void Voxel::Entity::HealDirect(float amount)
{
	m_Health = (m_Health + amount < m_MaxHealth ? m_Health + amount : m_MaxHealth);
}

float Voxel::Entity::GetCurrentHealth() const
{
	return m_Health;
}

float Voxel::Entity::GetMaxHealth() const
{
	return m_MaxHealth;
}

void Voxel::Entity::MultiplyAdrenalin(float multiplier, Entity *from)
{
	AdrenalinDescription desc(multiplier);
	desc.Doer = from;
	desc.Receiver = this;

	auto it = m_Effects.find("OnAdrenalinChange");
	if (it != m_Effects.end())
	{
		auto type_it = it->second.find(typeid(OnAdrenalinChange));
		if (type_it != it->second.end())
			for (auto &effect_ptr : type_it->second)
				static_cast<OnAdrenalinChange *>(effect_ptr.get())->Modify(desc);

		type_it = it->second.find(typeid(TargettedEffect));
		if (type_it != it->second.end())
			for (auto &effect_ptr : type_it->second)
				static_cast<TargettedEffect *>(effect_ptr.get())->Activate(from);

		type_it = it->second.find(typeid(TriggeredEffect));
		if (type_it != it->second.end())
			for (auto &effect_ptr : type_it->second)
				static_cast<TriggeredEffect *>(effect_ptr.get())->Activate();
	}

	MultiplyAdrenalinDirect(desc.MultiplyToDo);
}

void Voxel::Entity::MultiplyAdrenalinDirect(float multiplier)
{
	m_Adrenalin *= multiplier;
}

float Voxel::Entity::GetAdrenalin() const
{
	return m_Adrenalin;
}

float Voxel::Entity::GetAdrenalinScale() const
{
	return m_AdrenalinScale;
}

float Voxel::Entity::GetAccuracy() const
{
	return m_Accuracy;
}
