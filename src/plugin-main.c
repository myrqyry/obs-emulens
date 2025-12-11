/*
 * src/plugin-main.c
 */

#include <obs-module.h>
#include "effects.h"
#include "plugin-support.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
    blog(LOG_INFO, "Loading %s plugin (v%s)", PLUGIN_NAME, PLUGIN_VERSION);

    for (size_t i = 0; i < num_effects; i++) {
        const effect_info_t *info = effects[i];
        if (!info || !info->id || !info->shader_path) {
            blog(LOG_ERROR, "Skipping invalid effect at index %zu", i);
            continue;
        }

        struct obs_source_info source_info = {
            .id = info->id,
            .type = OBS_SOURCE_TYPE_FILTER,
            .output_flags = OBS_SOURCE_VIDEO,
            .get_name = get_effect_name,
            .create = info->create,
            .destroy = info->destroy,
            .update = info->update,
            .video_render = info->video_render,
            .video_tick = info->video_tick,
            .get_properties = info->get_properties,
            .get_defaults = info->get_defaults,
            .type_data = (void*)info
        };
        obs_register_source(&source_info);
    }
    
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "Unloaded %s", PLUGIN_NAME);
}
