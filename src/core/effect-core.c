/*
 * src/core/effect-core.c
 * Generic effect lifecycle management
 */

#include "effect-core.h"
#include "../utils/logging.h"
#include <math.h>

void *generic_create(obs_data_t *settings, obs_source_t *source) {
    const effect_info_t *info = obs_source_get_type_data(source);
    
    effect_data_t *ed = bzalloc(sizeof(effect_data_t));
    ed->context = source;
    ed->info = info;

    PLUGIN_LOG_DEBUG("effect-core", "Creating effect: %s", info->name);

    obs_enter_graphics();
    ed->effect = load_shader_effect(info->shader_path);
    
    if (!ed->effect) {
        obs_leave_graphics();
        PLUGIN_LOG_ERROR("effect-core", "Failed to create effect: %s", info->name);
        bfree(ed);
        return NULL;
    }

    // Allocate cache arrays if we have parameters
    if (info->num_params > 0) {
        ed->cached_float_values = bzalloc(sizeof(float) * info->num_params);
        ed->cached_int_values = bzalloc(sizeof(int) * info->num_params);
        ed->cached_bool_values = bzalloc(sizeof(bool) * info->num_params);
        ed->cached_color_values = bzalloc(sizeof(uint32_t) * info->num_params);
    }

    // Use the central binding helper
    bind_effect_parameters(ed);
    
    obs_leave_graphics();

    obs_source_update(source, settings);
    return ed;
}

void generic_destroy(void *data) {
    effect_data_t *ed = data;
    if (!ed) return;

    PLUGIN_LOG_DEBUG("effect-core", "Destroying effect: %s", ed->info ? ed->info->name : "Unknown");

    obs_enter_graphics();
    
    // Properly cleanup effect
    if (ed->effect) {
        gs_effect_destroy(ed->effect);
        ed->effect = NULL;
    }
    
    obs_leave_graphics();
    
    // Cleanup parameter handles array with verification
    if (ed->param_handles) {
        // Nullify handles before freeing container to catch use-after-free
        if (ed->info && ed->info->num_params > 0) {
            for (size_t i = 0; i < ed->info->num_params; i++) {
                ed->param_handles[i] = NULL;
            }
        }
        bfree(ed->param_handles);
        ed->param_handles = NULL;
    }
    
    // Cleanup cached values
    if (ed->cached_float_values) bfree(ed->cached_float_values);
    if (ed->cached_int_values) bfree(ed->cached_int_values);
    if (ed->cached_bool_values) bfree(ed->cached_bool_values);
    if (ed->cached_color_values) bfree(ed->cached_color_values);
    
    bfree(ed);
}

void generic_render(void *data, gs_effect_t *effect) {
    (void)effect; // Use internal effect
    effect_data_t *ed = data;
    
    if (!ed || !ed->effect) {
        obs_source_skip_video_filter(ed->context);
        return;
    }

    obs_source_t *target = obs_filter_get_target(ed->context);
    if (!target) {
        obs_source_skip_video_filter(ed->context);
        return;
    }

    if (obs_source_process_filter_begin(ed->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING)) {
        float width = (float)obs_source_get_width(target);
        float height = (float)obs_source_get_height(target);

        // Update Standard Uniforms
        if (ed->param_uv_size) {
            struct vec2 uv;
            vec2_set(&uv, width, height);
            gs_effect_set_vec2(ed->param_uv_size, &uv);
        }
        
        if (ed->param_uv_pixel_interval) {
            struct vec2 interval;
            vec2_set(&interval, 1.0f / width, 1.0f / height);
            gs_effect_set_vec2(ed->param_uv_pixel_interval, &interval);
        }

        if (ed->param_elapsed_time) {
            gs_effect_set_float(ed->param_elapsed_time, ed->elapsed_time);
        }
        
        // Use standard end function which handles techniques automatically
        obs_source_process_filter_end(ed->context, ed->effect, 0, 0);
    }
}

void generic_tick(void *data, float seconds) {
    effect_data_t *ed = data;
    if (ed) {
        ed->elapsed_time += seconds;
        // Basic overflow protection
        if (ed->elapsed_time > 86400.0f) ed->elapsed_time = fmodf(ed->elapsed_time, 86400.0f);
    }
}
