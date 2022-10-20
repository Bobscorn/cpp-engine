#pragma once

#include "Math/floaty.h"

#include <string>

namespace Voxel
{
	struct Entity;
	struct TargetableEntity;

	enum DamageType
	{
		BASIC = 0,
		PROJECTILE = 1,
		MAGIC = 2,
		EFFECT = 3,
	};

	struct DamageDescription
	{
		DamageDescription(float base_damage) : BaseDamage(base_damage), DamageToDo(base_damage) {}
		DamageDescription(const DamageDescription &other) = default;

		inline DamageDescription &operator=(const DamageDescription &other) = default;

		float BaseDamage;
		float DamageToDo;
		Entity *Dealer;
		Entity *Receiver;
		DamageType Type;
	};

	struct HealDescription
	{
		HealDescription(float base_heal) : BaseHeal(base_heal), HealToDo(base_heal) {}
		float BaseHeal;
		float HealToDo;
		Entity *Healer;
		Entity *BeingHealed;
	};

	struct AdrenalinDescription
	{
		AdrenalinDescription(float base_mult) : BaseMultiplier(base_mult), MultiplyToDo(base_mult) {}
		float BaseMultiplier;
		float MultiplyToDo;
		Entity *Doer;
		Entity *Receiver;
	};

	struct DamageDoneDescription
	{
		DamageDoneDescription(float dam_attempt) : DamageAttempted(dam_attempt), DamageDone(dam_attempt) {}
		const float DamageAttempted;
		float DamageDone;
	};

	struct AbilityCastDescription
	{
		AbilityCastDescription(std::string name, bool direct, float damage) : AbilityName(name), Direct(direct), Damage(damage) {}
		const std::string AbilityName;
		float ResourceCost;
		DamageDescription Damage;
		Entity *Caster;
		Entity *TargetEntity;
		floaty3 TargetPosition;
		size_t ProjectileCount;
		bool Magic;
		const bool Direct; // true means Target Entity, false means TargetPosition
	};
}