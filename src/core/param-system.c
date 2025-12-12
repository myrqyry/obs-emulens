/*
 * src/core/param-system.c
 * Parameter binding, updates, and UI generation
 */

#include "effect-core.h"
#include "../utils/logging.h"
#include <graphics/effect.h>
#include <math.h>

void bind_effect_parameters(effect_data_t *ed) {
    if (!ed || !ed->effect || !ed->info) return;

    // Bind Standard Uniforms
    ed->param_image = gs_effect_get_param_by_name(ed->effect, "image");
    ed->param_uv_size = gs_effect_get_param_by_name(ed->effect, "uv_size");
    ed->param_uv_pixel_interval = gs_effect_get_param_by_name(ed->effect, "uv_pixel_interval");
    ed->param_elapsed_time = gs_effect_get_param_by_name(ed->effect, "elapsed_time");

    // Dynamic Parameter Binding
    if (ed->info->num_params > 0) {
        // Free existing handles if any (though usually this starts empty)
        if (ed->param_handles) {
            bfree(ed->param_handles);
            ed->param_handles = NULL;
        }

        // Allocate new handles with verification
        ed->param_handles = bzalloc(sizeof(gs_eparam_t *) * ed->info->num_params);
        if (!ed->param_handles) {
            PLUGIN_LOG_ERROR("param-system", "%s: Failed to allocate parameter handles for %zu params", 
                 ed->info->name, ed->info->num_params);
            return;
        }
        
        for (size_t i = 0; i < ed->info->num_params; i++) {
            const char *param_name = ed->info->params[i].name;
            ed->param_handles[i] = gs_effect_get_param_by_name(ed->effect, param_name);
            
            if (!ed->param_handles[i]) {
                PLUGIN_LOG_WARNING("param-system", "[%s] Shader parameter '%s' not found", 
                    ed->info->name, param_name);
            }
        }
    }
}

void generic_update(void *data, obs_data_t *settings) {
    effect_data_t *ed = data;
    if (!ed || !ed->effect || !ed->info) return;

    bool any_changed = false;

    // Iterate through metadata and update shader parameters based on type
    for (size_t i = 0; i < ed->info->num_params; i++) {
        const param_def_t *def = &ed->info->params[i];
        gs_eparam_t *handle = ed->param_handles[i];
        if (!handle) continue;

        // Robust Type Checking: Ensure we match the shader's expectation
        enum gs_shader_param_type type = handle->type;
        
        bool param_changed = false;

        PLUGIN_LOG_DEBUG("param-trace", "Update param '%s': type=%d", def->name, (int)type);

        switch (def->type) {
            case PARAM_FLOAT: {
                double val = obs_data_get_double(settings, def->name);
                if (val < def->min) val = def->min;
                if (val > def->max) val = def->max;
                
                float fval = (float)val;
                
                if (type == GS_SHADER_PARAM_FLOAT) {
                    if (fabsf(fval - ed->cached_float_values[i]) > 0.0001f) {
                        gs_effect_set_float(handle, fval);
                        ed->cached_float_values[i] = fval;
                        param_changed = true;
                    }
                } else if (type == GS_SHADER_PARAM_INT) {
                     int ival = (int)val;
                     if (ival != ed->cached_int_values[i]) {
                        gs_effect_set_int(handle, ival);
                        ed->cached_int_values[i] = ival;
                        param_changed = true;
                     }
                }
                break;
            }
            case PARAM_INT: {
                long long val = obs_data_get_int(settings, def->name);
                if (val < (long long)def->min) val = (long long)def->min;
                if (val > (long long)def->max) val = (long long)def->max;

                int ival = (int)val;

                if (type == GS_SHADER_PARAM_INT) {
                    if (ival != ed->cached_int_values[i]) {
                        gs_effect_set_int(handle, ival);
                        ed->cached_int_values[i] = ival;
                        param_changed = true;
                    }
                } else if (type == GS_SHADER_PARAM_FLOAT) {
                    float fval = (float)val;
                    if (fabsf(fval - ed->cached_float_values[i]) > 0.0001f) {
                        gs_effect_set_float(handle, fval);
                        ed->cached_float_values[i] = fval;
                        param_changed = true;
                    }
                }
                break;
            }
            case PARAM_BOOL: {
                bool val = obs_data_get_bool(settings, def->name);
                
                if (val != ed->cached_bool_values[i]) {
                    PLUGIN_LOG_DEBUG("param-trace", "Bool Param '%s' -> %d (Shader Type: %d)", def->name, val, type);
                    if (type == GS_SHADER_PARAM_BOOL) {
                        // Use set_int to match standard 4-byte bool expectation in GLSL/HLSL uniforms to verify size
                        gs_effect_set_int(handle, val ? 1 : 0);
                    } else if (type == GS_SHADER_PARAM_INT) {
                        gs_effect_set_int(handle, val ? 1 : 0);
                    } else if (type == GS_SHADER_PARAM_FLOAT) {
                        gs_effect_set_float(handle, val ? 1.0f : 0.0f);
                    }
                    ed->cached_bool_values[i] = val;
                    param_changed = true;
                }
                break;
            }
            case PARAM_COLOR: {
                long long val = obs_data_get_int(settings, def->name);
                uint32_t color_val = (uint32_t)val;
                
                if (color_val != ed->cached_color_values[i]) {
                    struct vec4 color_vec;
                    vec4_from_rgba(&color_vec, color_val); // OBS math helper

                    if (type == GS_SHADER_PARAM_VEC4) {
                        gs_effect_set_vec4(handle, &color_vec);
                    } else if (type == GS_SHADER_PARAM_INT) {
                        gs_effect_set_int(handle, color_val);
                    }
                    ed->cached_color_values[i] = color_val;
                    param_changed = true;
                }
                break;
            }
        }
        
        if (param_changed) any_changed = true;
    }
    
    if (any_changed) PLUGIN_LOG_DEBUG("param-system", "%s: Parameters updated", ed->info->name);
}

obs_properties_t *generic_properties(void *data) {
    obs_properties_t *props = obs_properties_create();
    effect_data_t *ed = data;
    if (!ed || !ed->info) return props;

    // Generate UI from Metadata
    for (size_t i = 0; i < ed->info->num_params; i++) {
        const param_def_t *def = &ed->info->params[i];
        
        switch (def->type) {
            case PARAM_FLOAT:
                obs_properties_add_float_slider(props, def->name, def->display_name, def->min, def->max, def->step);
                break;
            case PARAM_INT:
                obs_properties_add_int_slider(props, def->name, def->display_name, (int)def->min, (int)def->max, (int)def->step);
                break;
            case PARAM_BOOL:
                obs_properties_add_bool(props, def->name, def->display_name);
                break;
            case PARAM_COLOR:
                obs_properties_add_color(props, def->name, def->display_name);
                break;
        }
    }

    return props;
}
