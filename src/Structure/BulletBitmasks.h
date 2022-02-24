#pragma once

constexpr int DYNAMIC_PERVIOUS			= 1 << 6;
constexpr int DYNAMIC_PERVIOUS_INVERTED = DYNAMIC_PERVIOUS << 1;
constexpr int ENTITY_GENERAL			= DYNAMIC_PERVIOUS_INVERTED << 1;
constexpr int ENVIRONMENT				= (ENTITY_GENERAL << 1) | 2;
constexpr int PLAYER			= (ENVIRONMENT & ~2) << 1;
constexpr int PERVIOUS_WALL				= PLAYER << 1;
constexpr int INTERACTION_RAY			= PERVIOUS_WALL << 1;
constexpr int INTERACTABLE				= INTERACTION_RAY << 1;
constexpr int ACTIVATOR_BUTTON			= INTERACTABLE << 1;
constexpr int ACTIVATOR_PAD				= ACTIVATOR_BUTTON << 1;
constexpr int DISABLED_PERVIOUS_WALL	= ACTIVATOR_PAD << 1;
constexpr int PAD_PRESSURER				= DISABLED_PERVIOUS_WALL << 1;
constexpr int PLAYER_FEET				= PAD_PRESSURER << 1;
constexpr int PLAYER_FEET_DETECTOR		= PLAYER_FEET << 1;
constexpr int PROJECTILE_GENERAL		= PLAYER_FEET_DETECTOR << 1;