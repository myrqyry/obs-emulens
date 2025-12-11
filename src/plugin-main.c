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
#include "plugin-support.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
    blog(LOG_INFO, "Loading OBS Emulens plugin (v%s)", PLUGIN_VERSION);

    // Validate effect registry before registration
    for (size_t i = 0; i < num_effects; i++) {
        const effect_info_t *effect_info = effects[i];
        if (!effect_info || !effect_info->id || !effect_info->name || !effect_info->shader_path) {
            blog(LOG_ERROR, "Invalid effect info at index %zu", i);
            return false;
        }
    }
    
    for (size_t i = 0; i < num_effects; i++) {
        const effect_info_t *effect_info = effects[i];
        if (!effect_info) {
            continue; 
        }

        struct obs_source_info info = {
            .id = effect_info->id,
            .type = OBS_SOURCE_TYPE_FILTER,
            .output_flags = OBS_SOURCE_VIDEO,
            .get_name = get_effect_name,
            .create = effect_info->create,
            .destroy = effect_info->destroy,
            .update = effect_info->update,
            .video_render = effect_info->video_render,
            .video_tick = effect_info->video_tick,
            .get_properties = effect_info->get_properties,
            .get_defaults = effect_info->get_defaults,
            .type_data = (void*)effect_info
        };
        obs_register_source(&info);
    }
    
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "OBS Emulens plugin unloaded");
}
