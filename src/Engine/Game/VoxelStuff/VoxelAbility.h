#pragma once

#include "Helpers/VectorHelper.h"

#include "Entities/VoxelProjectiles.h"

#include "Game/VoxelStuff/VoxelDamage.h"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include <typeinfo>
#include <typeindex>

template <class T>
inline void hash_combine(std::size_t &seed, const T &v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

inline size_t hashes_combined(size_t _1, size_t _2)
{
	return _1 ^ (_2 + 0x9e3779b9 + (_1 << 6) + (_1 >> 2));
}

namespace Voxel
{
	struct TriggeredEffect;
	struct TargettedEffect;
	struct PreDamageEffect;
	struct PostDamageEffect;

	enum EntityType
	{
		TARGETABLE = 1,
		TAKES_KNOCKBACK = 2,
		USES_ADRENALIN = 3,
		HAS_RAGE = 4, // Experimental
	};

	struct Effect
	{
		virtual bool Keep() = 0; // Checks all effects are supposed to be there
		virtual std::string GetName() = 0;
		virtual size_t GetWeight() const = 0;
		virtual std::type_index GetTypeIndex() = 0;
	};

	struct Ability
	{
		virtual std::string GetName() const = 0;
		virtual AbilityCastDescription CreateDescription() = 0;
		virtual void Cast(const AbilityCastDescription &desc) = 0;
	};

	struct EffectKey
	{
		std::string Name;
		std::type_index Type;
	};
}

namespace std
{
	template<>
	struct hash<Voxel::EffectKey>
	{
		inline size_t operator()(const Voxel::EffectKey &key) const noexcept
		{
			return hashes_combined(std::hash<std::string>()(key.Name), std::hash<std::type_index>()(key.Type));
		}
	};
}

namespace Voxel
{
	struct CompEffect
	{
		inline bool operator()(const std::unique_ptr<Effect> &a, const std::unique_ptr<Effect> &b) const
		{
			return a->GetWeight() < b->GetWeight();
		}
	};

	struct VoxelWorld;

	struct Entity
	{
		Entity(VoxelWorld *world) : m_World(world) {}
		virtual ~Entity() {}

		virtual void BeforeDraw() = 0;
		virtual void AfterDraw() = 0;

		void CheckEffects();

		// Returns whether to do on hit
		virtual bool HitByProjectile(Projectile *proj, floaty3 hit_point) = 0;

		virtual void CastAbility(std::string name);
		// Applis this Entity's effects to an AbilityCastDescription, does not cast the ability itself
		virtual void TriggerAbilityEffects(AbilityCastDescription &desc);

		virtual void Attack(Entity *other, float base_damage);
		virtual void AbilityAttack(Entity *other, DamageDescription damage);
		virtual DamageDoneDescription TakeDamage(DamageDescription desc, Entity *from);
		virtual bool TakeDamageDirect(float amount); // Returns Whether it (will) die

		virtual void TakeKnockback(floaty3 knock, Entity *from);
		virtual void TakeKnockbackDirect(floaty3 knock) = 0;

		virtual void Heal(float amount, Entity *healer);
		virtual void HealDirect(float amount);
		virtual float GetCurrentHealth() const;
		virtual float GetMaxHealth() const;

		virtual void MultiplyAdrenalin(float multiplier, Entity *from);
		virtual void MultiplyAdrenalinDirect(float multiplier);
		virtual float GetAdrenalin() const;
		virtual float GetAdrenalinScale() const;

		virtual float GetAccuracy() const;

		int GetType();

		void AddEffect(std::string entry, std::unique_ptr<Effect> effect);
		void RemoveEffect(std::string entry, std::string effect_name);
		void ClearEffects(std::string entry);

		void DoPreDamageEffects(DamageDescription &desc);
		void DoPostDamageEffects(Entity *receiver, const DamageDoneDescription &desc, DamageType dam_type);
		void DoDamageTakenEffects(Entity *attacker, DamageDoneDescription &desc, DamageType type);

		virtual void Die(); // Expected to kill the entity *later*

		virtual floaty3 GetCentre() = 0;

		using stupid_it = std::unordered_map<std::string, std::unordered_map<std::type_index, std::set<std::unique_ptr<Effect>, CompEffect>>>::iterator;
	protected:

		std::unordered_map<std::string, std::unordered_map<std::type_index, std::set<std::unique_ptr<Effect>, CompEffect>>> m_Effects;
		std::unordered_map<std::string, std::unique_ptr<Ability>> m_Abilities;

		VoxelWorld *m_World;
		float m_Health;
		float m_MaxHealth;

		// Adrenalin is (generally) between 0.0-1.0, it is a resource that affects cooldowns, speed, and damage intake and outtake
		// While adrenalin is low, the entity experiences less damage from big hits, and especially from fatal damage (low adrenalin can stop fatal damage)
		// While adrenalin is high, the entity experiences reduced cooldowns, faster movements, reduced debuffs, and increased damage output
		// As adrenalin peaks at around 1.0 debuffs are much less effective, higher speed/cooldowns, however,
		// With adrenalin comes large pieces of damage scaling, as adrenalin gets higher, smaller of pieces of damage are scaled upwards
		// Adrenalin affects damage taken 
		// It is designed to give players a second chance 
		float m_Adrenalin;
		float m_AdrenalinScale = 1.05f;

		// Experimental mechanic
		float m_Rage;

		// Multiplies Skillshot angles, m_Accuracy == 1.f generally means normal accuracy (no positive or negative effect)
		float m_Accuracy;

		int m_Type;
	};

	struct TriggeredEffect : Effect
	{
		virtual void Activate() = 0;
	};

	struct TargettedEffect : Effect
	{
		virtual void Activate(Entity *target) = 0;
	};

	struct PreDamageEffect : Effect
	{
		virtual void Modify(DamageDescription &desc) = 0;
	};

	struct PostDamageEffect : Effect
	{
		virtual void Activate(Entity *dealer, Entity *receiver, DamageDoneDescription desc) = 0;
	};

	struct OnDamagedEffect : Effect
	{
		virtual void Modify(Entity *dealer, Entity *receiver, DamageDoneDescription &dam) = 0;
	};

	struct OnKnockedBackEffect : Effect
	{
		virtual void Modify(Entity *knockbacker, Entity *receiver, floaty3 &knockback) = 0;
	};

	struct OnHealedEffect : Effect
	{
		virtual void Modify(HealDescription &desc) = 0;
	};

	struct OnAdrenalinChange : Effect
	{
		virtual void Modify(AdrenalinDescription &desc) = 0;
	};

	struct OnAbilityCast : Effect
	{
		// Target may be null or equal to caster
		virtual void OnCast(AbilityCastDescription &desc) = 0;
	};
}