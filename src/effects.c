/*
 * src/effects.c
 * Implements Generic Data-Driven Effect Logic
 */

#include "effects.h"
#include <obs-module.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <math.h>

// --- Helper: Default Value Setter ---
// Applies the default values defined in the struct to the OBS settings object
static void set_defaults_from_params(obs_data_t *settings, const param_def_t *params, size_t count) {
    for (size_t i = 0; i < count; i++) {
        const param_def_t *def = &params[i];
        switch (def->type) {
            case PARAM_FLOAT: obs_data_set_default_double(settings, def->name, def->default_val.f_val); break;
            case PARAM_INT:   obs_data_set_default_int(settings, def->name, def->default_val.i_val); break;
            case PARAM_BOOL:  obs_data_set_default_bool(settings, def->name, def->default_val.b_val); break;
            case PARAM_COLOR: obs_data_set_default_int(settings, def->name, def->default_val.i_val); break;
        }
    }
}

// --- 1. Parameter Definitions (The "Config") ---

static const param_def_t star_burst_params[] = {
    {"Threshold", "Threshold", "Brightness threshold", PARAM_FLOAT, {.f_val=0.7}, 0.33, 2.0, 0.01},
    {"Intensity", "Intensity", "Ray intensity", PARAM_FLOAT, {.f_val=3.0}, 1.0, 10.0, 0.5},
    {"StarPoints", "Star Points", "Number of points", PARAM_INT, {.i_val=8}, 4, 16, 2},
    {"RayLength", "Ray Length", "Length of rays", PARAM_FLOAT, {.f_val=0.2}, 0.05, 0.5, 0.05},
    {"RayThickness", "Ray Thickness", "Thickness of rays", PARAM_FLOAT, {.f_val=2.0}, 0.5, 5.0, 0.5},
    {"RaySmoothness", "Ray Smoothness", "Falloff smoothness", PARAM_FLOAT, {.f_val=3.0}, 1.0, 10.0, 0.5},
    {"Rotation", "Rotation", "Static rotation", PARAM_FLOAT, {.f_val=0.0}, 0.0, 6.283, 0.1},
    {"ColorizeRays", "Colorize Rays", "Enable custom ray color", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"RayColor", "Ray Color", "Custom ray color", PARAM_COLOR, {.i_val=0xFFFFFFFF}, 0, 0, 0},
    {"EnableRotation", "Animate Rotation", "Enable continuous rotation", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"RotationSpeed", "Rotation Speed", "Speed of rotation animation", PARAM_FLOAT, {.f_val=0.5}, -2.0, 2.0, 0.1},
    {"ExtendRays", "Extend Rays", "Extend rays beyond bright areas", PARAM_BOOL, {.b_val=true}, 0, 0, 0},
    {"AnamorphicRays", "Anamorphic", "Stretch rays horizontally", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"ray_sample_count", "Quality", "Ray sample quality", PARAM_INT, {.i_val=8}, 4, 12, 1},
    {"CoreGlowIntensity", "Core Glow", "Source glow intensity", PARAM_FLOAT, {.f_val=0.3}, 0.0, 2.0, 0.05},
    {"CoreGlowUsesRayColor", "Tint Core Glow", "Use ray color for core glow", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"RayEdgeSoftness", "Ray Softness", "Edge softness of rays", PARAM_FLOAT, {.f_val=1.5}, 0.5, 5.0, 0.1}
};

static const param_def_t liteleke_params[] = {
    {"leakIntensity", "Intensity", "Opacity of the light leak", PARAM_FLOAT, {.f_val=0.8}, 0.0, 3.0, 0.05},
    {"leakColor", "Leak Color", "Primary leak color", PARAM_COLOR, {.i_val=0xFF3380FF}, 0, 0, 0}, // ABGR format often used in OBS
    {"leakScale", "Scale", "Noise pattern size", PARAM_FLOAT, {.f_val=2.0}, 0.1, 10.0, 0.1},
    {"leakSpeed", "Speed", "Animation speed", PARAM_FLOAT, {.f_val=0.5}, 0.0, 5.0, 0.1},
    {"edgeFalloff", "Edge Falloff", "Edge clamping tightness", PARAM_FLOAT, {.f_val=3.0}, 0.5, 10.0, 0.1},
    {"noiseComplexity", "Complexity", "Noise detail level", PARAM_FLOAT, {.f_val=3.0}, 1.0, 8.0, 1.0},
    {"streakiness", "Streakiness", "Horizontal stretching", PARAM_FLOAT, {.f_val=1.0}, 0.1, 10.0, 0.05},
    {"enablePulsing", "Enable Pulsing", "Pulse intensity over time", PARAM_BOOL, {.b_val=true}, 0, 0, 0},
    {"pulseSpeed", "Pulse Speed", "Pulsing frequency", PARAM_FLOAT, {.f_val=0.5}, 0.1, 5.0, 0.05},
    {"blendMode", "Blend Mode", "0:Alpha 1:Add 2:Screen 3:Over 4:Soft", PARAM_INT, {.i_val=0}, 0, 4, 1}
};

static const param_def_t handheld_params[] = {
    {"preset", "Preset", "0:Stable 1:Breath 2:Handheld 3:Shaky 4:Quake 99:Custom", PARAM_INT, {.i_val=2}, 0, 99, 1},
    {"masterIntensity", "Master Intensity", "Global strength multiplier", PARAM_FLOAT, {.f_val=1.0}, 0.0, 2.0, 0.05},
    {"positionSpeed", "Pos Speed", "Position shake frequency", PARAM_FLOAT, {.f_val=1.5}, 0.1, 10.0, 0.1},
    {"rotationSpeed", "Rot Speed", "Rotation shake frequency", PARAM_FLOAT, {.f_val=1.0}, 0.1, 10.0, 0.1},
    {"zoomSpeed", "Zoom Speed", "Zoom breathing frequency", PARAM_FLOAT, {.f_val=0.8}, 0.1, 10.0, 0.1},
    {"enableDynamicBlur", "Motion Blur", "Enable dynamic motion blur", PARAM_BOOL, {.b_val=true}, 0, 0, 0},
    {"blurAmount", "Blur Amount", "Strength of motion blur", PARAM_FLOAT, {.f_val=1.0}, 0.0, 5.0, 0.1}
};

static const param_def_t bokeh_params[] = {
    {"particle_density", "Density", "Particle count density", PARAM_FLOAT, {.f_val=25.0}, 1.0, 100.0, 1.0},
    {"particle_base_size", "Size", "Base particle size", PARAM_FLOAT, {.f_val=0.05}, 0.001, 0.2, 0.001},
    {"focus_strength", "Focus Strength", "Blur falloff from focus", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1},
    {"motion_blur_amount", "Motion Blur", "Trail/Motion blur amount", PARAM_FLOAT, {.f_val=0.1}, 0.0, 0.95, 0.01},
    {"bokeh_edge_softness", "Softness", "Bokeh shape edge softness", PARAM_FLOAT, {.f_val=0.5}, 0.0, 1.0, 0.01},
    {"enable_chromatic_aberration", "Chromatic Aberration", "Enable CA", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"ca_strength", "CA Strength", "Chromatic Aberration Amount", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1}
};

// --- 2. Specific Defaults Wrappers (Needed for function pointers) ---
void star_burst_defaults(obs_data_t *s) { set_defaults_from_params(s, star_burst_params, 17); }
void liteleke_defaults(obs_data_t *s) { set_defaults_from_params(s, liteleke_params, 10); }
void handheld_defaults(obs_data_t *s) { set_defaults_from_params(s, handheld_params, 7); }
void bokeh_defaults(obs_data_t *s) { set_defaults_from_params(s, bokeh_params, 7); }
void style_transfer_defaults(obs_data_t *s) { (void)s; }

// --- 3. Effect Registry ---

static const effect_info_t star_burst_info = {
    "star_burst_effect", "Star Burst", "Creates dramatic star-shaped rays", "shaders/star-burst.shader",
    star_burst_params, 17,
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, star_burst_defaults
};

static const effect_info_t liteleke_info = {
    "liteleke_effect", "Light Leak", "Adds organic light leaks", "shaders/light-leak.shader",
    liteleke_params, 10,
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, liteleke_defaults
};

static const effect_info_t handheld_info = {
    "handheld_effect", "Handheld Camera", "Simulates handheld camera movement", "shaders/handheld.shader",
    handheld_params, 7,
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, handheld_defaults
};

static const effect_info_t bokeh_info = {
    "bokeh_effect", "Bokeh", "Creates beautiful bokeh light effects", "shaders/bokeh.shader",
    bokeh_params, 7,
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, bokeh_defaults
};

static const effect_info_t style_transfer_info = {
    "style_transfer_effect", "Style Transfer", "Applies artistic style transfer", "shaders/style-transfer.shader",
    NULL, 0,
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, style_transfer_defaults
};

const effect_info_t *effects[] = {
    &star_burst_info, &liteleke_info, &handheld_info, &bokeh_info, &style_transfer_info
};
const size_t num_effects = sizeof(effects) / sizeof(effects[0]);

// --- 4. Implementation Helpers ---

const char *get_effect_name(void *type_data) {
    const effect_info_t *info = type_data;
    return info ? info->name : "Unknown Effect";
}

static bool is_valid_shader_path(const char *path) {
    if (!path || strlen(path) == 0 || strlen(path) > 256) return false;
    // Basic directory traversal check
    if (strstr(path, "..") || strstr(path, "//") || strchr(path, '\\')) return false;
    // Strict folder and extension check
    if (strncmp(path, "shaders/", 8) != 0) return false;
    const char *ext = strrchr(path, '.');
    return (ext && strcmp(ext, ".shader") == 0);
}

// --- 5. Generic Implementation ---

#include <graphics/effect.h> // Ensure we have effect definitions

gs_effect_t *load_shader_effect(const char *shader_path) {
    if (!is_valid_shader_path(shader_path)) {
        blog(LOG_ERROR, "Invalid shader path: %s", shader_path);
        return NULL;
    }
    
    // Explicitly find the module file using the project name "obs-emulens"
    obs_module_t *module = obs_get_module("obs-emulens");
    char *full_path = obs_find_module_file(module, shader_path);

    if (!full_path) {
        blog(LOG_ERROR, "Could not find shader file: %s", shader_path);
        return NULL;
    }
    
    char *error_string = NULL;
    gs_effect_t *effect = gs_effect_create_from_file(full_path, &error_string);
    bfree(full_path);
    if (error_string) {
        blog(LOG_ERROR, "Shader load error (%s): %s", shader_path, error_string);
        bfree(error_string);
    }
    return effect;
}

void *generic_create(obs_data_t *settings, obs_source_t *source) {
    const effect_info_t *info = obs_source_get_type_data(source);
    
    effect_data_t *ed = bzalloc(sizeof(effect_data_t));
    ed->context = source;
    ed->info = info;

    obs_enter_graphics();
    ed->effect = load_shader_effect(info->shader_path);
    
    if (!ed->effect) {
        obs_leave_graphics();
        bfree(ed);
        return NULL;
    }

    // Bind Standard Uniforms
    ed->param_image = gs_effect_get_param_by_name(ed->effect, "image");
    ed->param_uv_size = gs_effect_get_param_by_name(ed->effect, "uv_size");
    ed->param_uv_pixel_interval = gs_effect_get_param_by_name(ed->effect, "uv_pixel_interval");
    ed->param_elapsed_time = gs_effect_get_param_by_name(ed->effect, "elapsed_time");

    // Dynamic Parameter Binding
    if (info->num_params > 0) {
        ed->param_handles = bzalloc(sizeof(gs_eparam_t*) * info->num_params);
        for (size_t i = 0; i < info->num_params; i++) {
            ed->param_handles[i] = gs_effect_get_param_by_name(ed->effect, info->params[i].name);
            if (!ed->param_handles[i]) {
                blog(LOG_WARNING, "[%s] Warning: Shader parameter '%s' not found in shader", info->name, info->params[i].name);
            }
        }
    }
    obs_leave_graphics();

    obs_source_update(source, settings);
    return ed;
}

void generic_destroy(void *data) {
    effect_data_t *ed = data;
    if (ed) {
        obs_enter_graphics();
        if (ed->effect) gs_effect_destroy(ed->effect);
        obs_leave_graphics();
        
        if (ed->param_handles) bfree(ed->param_handles);
        bfree(ed);
    }
}

void generic_update(void *data, obs_data_t *settings) {
    effect_data_t *ed = data;
    if (!ed || !ed->effect) return;

    // Iterate through metadata and update shader parameters based on type
    for (size_t i = 0; i < ed->info->num_params; i++) {
        const param_def_t *def = &ed->info->params[i];
        gs_eparam_t *handle = ed->param_handles[i];
        if (!handle) continue;

        // Robust Type Checking: Ensure we match the shader's expectation
        enum gs_shader_param_type type = handle->type;

        switch (def->type) {
            case PARAM_FLOAT: {
                double val = obs_data_get_double(settings, def->name);
                if (val < def->min) val = def->min;
                if (val > def->max) val = def->max;
                
                if (type == GS_SHADER_PARAM_FLOAT) {
                    gs_effect_set_float(handle, (float)val);
                } else if (type == GS_SHADER_PARAM_INT) {
                    gs_effect_set_int(handle, (int)val);
                } else {
                    blog(LOG_WARNING, "Param '%s' mismatch: C=Float, Shader=%d", def->name, (int)type);
                }
                break;
            }
            case PARAM_INT: {
                long long val = obs_data_get_int(settings, def->name);
                if (val < (long long)def->min) val = (long long)def->min;
                if (val > (long long)def->max) val = (long long)def->max;

                if (type == GS_SHADER_PARAM_INT) {
                    gs_effect_set_int(handle, (int)val);
                } else if (type == GS_SHADER_PARAM_FLOAT) {
                    gs_effect_set_float(handle, (float)val);
                } else {
                    blog(LOG_WARNING, "Param '%s' mismatch: C=Int, Shader=%d", def->name, (int)type);
                }
                break;
            }
            case PARAM_BOOL: {
                bool val = obs_data_get_bool(settings, def->name);
                if (type == GS_SHADER_PARAM_BOOL) {
                    gs_effect_set_bool(handle, val);
                } else if (type == GS_SHADER_PARAM_INT) {
                    gs_effect_set_int(handle, val ? 1 : 0);
                } else if (type == GS_SHADER_PARAM_FLOAT) {
                    gs_effect_set_float(handle, val ? 1.0f : 0.0f);
                } else {
                    blog(LOG_WARNING, "Param '%s' mismatch: C=Bool, Shader=%d", def->name, (int)type);
                }
                break;
            }
            case PARAM_COLOR: {
                long long val = obs_data_get_int(settings, def->name);
                struct vec4 color_vec;
                vec4_from_rgba(&color_vec, (uint32_t)val);

                if (type == GS_SHADER_PARAM_VEC4) {
                    gs_effect_set_vec4(handle, &color_vec);
                } else if (type == GS_SHADER_PARAM_INT) {
                    gs_effect_set_int(handle, (uint32_t)val);
                } else {
                     blog(LOG_WARNING, "Param '%s' mismatch: C=Color, Shader=%d", def->name, (int)type);
                }
                break;
            }
        }
    }
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
        // This avoids double-rendering issues
        obs_source_process_filter_end(ed->context, ed->effect, 0, 0);
    }
}

void generic_tick(void *data, float seconds) {
    effect_data_t *ed = data;
    if (ed) {
        ed->elapsed_time += seconds;
        // Basic overflow protection, though float precision issues at 24h are negligible for shaders
        if (ed->elapsed_time > 86400.0f) ed->elapsed_time = fmod(ed->elapsed_time, 86400.0f);
    }
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
