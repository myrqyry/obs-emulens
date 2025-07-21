/*
OBS Emulens - Shader Effects Plugin
Copyright (C) 2025 Myrqyry

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>
#include <obs.h>
#include <obs-frontend-api.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include <util/bmem.h>
#include <util/dstr.h>
#include <stdbool.h>

#include "effects.h"

// Forward declarations from effects.c
const char *get_effect_name(void *type_data);
void *effect_create(obs_data_t *settings, obs_source_t *source);
void effect_destroy(void *data);
void effect_update(void *data, obs_data_t *settings);
void effect_video_render(void *data, gs_effect_t *effect);
void effect_video_tick(void *data, float seconds);
obs_properties_t *effect_properties(void *data);
void effect_defaults(obs_data_t *settings);

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

// Structure to hold effect registration data
struct obs_source_info effect_info = {
    .id = "", // Will be set for each effect
    .type = OBS_SOURCE_TYPE_FILTER,
    .output_flags = OBS_SOURCE_VIDEO,
    .get_name = get_effect_name,
    .create = effect_create,
    .destroy = effect_destroy,
    .update = effect_update,
    .video_render = effect_video_render,
    .video_tick = effect_video_tick,
};

bool obs_module_load(void)
{
    obs_log(LOG_INFO, "Loading OBS Emulens plugin");
    
    for (size_t i = 0; i < NUM_EFFECTS; i++) {
        struct obs_source_info info = {
            .id = effects[i]->id,
            .type = OBS_SOURCE_TYPE_FILTER,
            .output_flags = OBS_SOURCE_VIDEO,
            .get_name = get_effect_name,
            .create = effect_create,
            .destroy = effect_destroy,
            .update = effect_update,
            .video_render = effect_video_render,
            .video_tick = effect_video_tick,
            .get_properties = effect_properties,
            .get_defaults = effect_defaults
        };
        obs_register_source(&info);
    }
    
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "OBS Emulens plugin unloaded");
}
