/*
 * src/effects/effect-registry.h
 * Central registry for all effects
 */

#pragma once
#include "../core/effect-core.h"

extern const size_t num_effects;
extern const effect_info_t *effects[];

// Helper to get name from type data
const char *get_effect_name(void *type_data);
