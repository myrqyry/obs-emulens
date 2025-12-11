#include "effects.h"
#include <obs-module.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <math.h>

// --- Generic Callback Declarations ---
void *generic_create(obs_data_t *settings, obs_source_t *source);
void generic_destroy(void *data);
void generic_update(void *data, obs_data_t *settings);
void generic_render(void *data, gs_effect_t *effect);
void generic_tick(void *data, float seconds);
obs_properties_t *generic_properties(void *data);

// --- Helper for defaults ---
void set_defaults_from_params(obs_data_t *settings, const param_def_t *params, size_t count) {
    for (size_t i = 0; i < count; i++) {
        const param_def_t *def = &params[i];
        switch (def->type) {
            case PARAM_FLOAT: obs_data_set_default_double(settings, def->name, def->default_val.f_val); break;
            case PARAM_INT:   obs_data_set_default_int(settings, def->name, (long long)def->default_val.i_val); break;
            case PARAM_BOOL:  obs_data_set_default_bool(settings, def->name, def->default_val.b_val); break;
            case PARAM_COLOR: obs_data_set_default_int(settings, def->name, (long long)def->default_val.i_val); break;
        }
    }
}

// --- Parameter Definitions ---

static const param_def_t star_burst_params[] = {
    {"Threshold", "Threshold", "Brightness threshold", PARAM_FLOAT, {.f_val=0.7}, 0.33, 2.0, 0.01},
    {"Intensity", "Intensity", "Ray intensity", PARAM_FLOAT, {.f_val=3.0}, 1.0, 10.0, 0.5},
    {"StarPoints", "Star Points", "Number of points", PARAM_INT, {.i_val=8}, 4, 16, 2},
    {"RayLength", "Ray Length", "Length of rays", PARAM_FLOAT, {.f_val=0.2}, 0.05, 0.5, 0.05},
    {"RayThickness", "Ray Thickness", "Thickness of rays", PARAM_FLOAT, {.f_val=2.0}, 0.5, 5.0, 0.5},
    {"RaySmoothness", "Ray Smoothness", "Falloff smoothness", PARAM_FLOAT, {.f_val=3.0}, 1.0, 10.0, 0.5},
    {"Rotation", "Rotation", "Static rotation", PARAM_FLOAT, {.f_val=0.0}, 0.0, 6.283, 0.1},
    {"ColorizeRays", "Colorize Rays", "Enable custom ray color", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"RayColor", "Ray Color", "Custom ray color", PARAM_COLOR, {.i_val=0xFF66CCFF}, 0, 0, 0}, // ABGR (Orange-ish)
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
    {"leakColor", "Leak Color", "Primary leak color", PARAM_COLOR, {.i_val=0xFF3380FF}, 0, 0, 0}, // ABGR
    {"leakIntensity", "Intensity", "Overall opacity", PARAM_FLOAT, {.f_val=0.8}, 0.0, 3.0, 0.05},
    {"leakSpeed", "Speed", "Animation speed", PARAM_FLOAT, {.f_val=0.5}, 0.0, 5.0, 0.1},
    {"leakScale", "Scale", "Noise pattern size", PARAM_FLOAT, {.f_val=2.0}, 0.1, 10.0, 0.1},
    {"edgeFalloff", "Edge Falloff", "Edge clamping tightness", PARAM_FLOAT, {.f_val=3.0}, 0.5, 10.0, 0.1},
    {"noiseComplexity", "Complexity", "Noise detail level", PARAM_FLOAT, {.f_val=3.0}, 1.0, 8.0, 1.0},
    {"topBias", "Top Bias", "Leak visibility top", PARAM_FLOAT, {.f_val=0.25}, 0.0, 2.0, 0.05},
    {"bottomBias", "Bottom Bias", "Leak visibility bottom", PARAM_FLOAT, {.f_val=0.25}, 0.0, 2.0, 0.05},
    {"leftBias", "Left Bias", "Leak visibility left", PARAM_FLOAT, {.f_val=0.25}, 0.0, 2.0, 0.05},
    {"rightBias", "Right Bias", "Leak visibility right", PARAM_FLOAT, {.f_val=0.25}, 0.0, 2.0, 0.05},
    {"streakiness", "Streakiness", "Horizontal stretching", PARAM_FLOAT, {.f_val=1.0}, 0.1, 10.0, 0.05},
    {"leakShapeContrast", "Contrast", "Leak shape sharpness", PARAM_FLOAT, {.f_val=1.5}, 0.5, 5.0, 0.05},
    {"enablePulsing", "Enable Pulsing", "Pulse intensity over time", PARAM_BOOL, {.b_val=true}, 0, 0, 0},
    {"pulseSpeed", "Pulse Speed", "Pulsing frequency", PARAM_FLOAT, {.f_val=0.5}, 0.1, 5.0, 0.05},
    {"pulseMinAlpha", "Pulse Min", "Minimum alpha during pulse", PARAM_FLOAT, {.f_val=0.05}, 0.0, 1.0, 0.01},
    {"pulseMaxAlpha", "Pulse Max", "Maximum alpha during pulse", PARAM_FLOAT, {.f_val=0.3}, 0.0, 1.0, 0.01},
    {"enableColorShift", "Color Shift", "Enable dual-color shifting", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"secondLeakColor", "Second Color", "Secondary leak color", PARAM_COLOR, {.i_val=0xFF1A33FF}, 0, 0, 0},
    {"colorShiftSpeed", "Shift Speed", "Color transition speed", PARAM_FLOAT, {.f_val=0.2}, 0.05, 2.0, 0.05},
    {"hotspotIntensity", "Hotspot", "Core brightness boost", PARAM_FLOAT, {.f_val=0.5}, 0.0, 3.0, 0.05},
    {"hotspotExponent", "Hotspot Size", "Hotspot tightness", PARAM_FLOAT, {.f_val=3.0}, 1.0, 10.0, 0.1},
    {"hotspotColor", "Hotspot Tint", "Tint for hotspot", PARAM_COLOR, {.i_val=0xFF000D1A}, 0, 0, 0},
    {"grainAmount", "Grain", "Film grain intensity", PARAM_FLOAT, {.f_val=0.05}, 0.0, 0.5, 0.01},
    {"grainScale", "Grain Size", "Film grain scale", PARAM_FLOAT, {.f_val=50.0}, 10.0, 100.0, 1.0},
    {"blendMode", "Blend Mode", "0:Alpha 1:Add 2:Screen 3:Over 4:Soft", PARAM_INT, {.i_val=0}, 0, 4, 1}
};

static const param_def_t handheld_params[] = {
    {"preset", "Preset", "0:Stable 1:Breath 2:Handheld 3:Shaky 4:Quake 99:Custom", PARAM_INT, {.i_val=2}, 0, 99, 1},
    {"masterIntensity", "Master Intensity", "Global strength multiplier", PARAM_FLOAT, {.f_val=1.0}, 0.0, 2.0, 0.05},
    {"positionAmount", "Pos Amount", "Position shake amplitude", PARAM_FLOAT, {.f_val=0.005}, 0.0, 0.1, 0.001},
    {"rotationAmount", "Rot Amount", "Rotation shake amplitude", PARAM_FLOAT, {.f_val=0.5}, 0.0, 10.0, 0.1},
    {"zoomAmount", "Zoom Amount", "Zoom breathing amplitude", PARAM_FLOAT, {.f_val=0.01}, 0.0, 0.2, 0.002},
    {"positionSpeed", "Pos Speed", "Position shake frequency", PARAM_FLOAT, {.f_val=1.5}, 0.1, 10.0, 0.1},
    {"rotationSpeed", "Rot Speed", "Rotation shake frequency", PARAM_FLOAT, {.f_val=1.0}, 0.1, 10.0, 0.1},
    {"zoomSpeed", "Zoom Speed", "Zoom breathing frequency", PARAM_FLOAT, {.f_val=0.8}, 0.1, 10.0, 0.1},
    {"enableDynamicBlur", "Motion Blur", "Enable dynamic motion blur", PARAM_BOOL, {.b_val=true}, 0, 0, 0},
    {"blurAmount", "Blur Max", "Maximum blur amount", PARAM_FLOAT, {.f_val=1.0}, 0.0, 5.0, 0.1},
    {"blurSpeed", "Blur Speed", "Blur fluctuation speed", PARAM_FLOAT, {.f_val=1.5}, 0.1, 10.0, 0.1},
    {"staticBlurAmount", "Static Blur", "Constant base blur", PARAM_FLOAT, {.f_val=0.0}, 0.0, 3.0, 0.05},
    {"edgeFeatherAmount", "Edge Feather", "Softness of video edges", PARAM_FLOAT, {.f_val=0.05}, 0.0, 0.25, 0.005}
};

static const param_def_t bokeh_params[] = {
    {"particle_density", "Density", "Particle count density", PARAM_FLOAT, {.f_val=25.0}, 1.0, 100.0, 1.0},
    {"particle_base_size", "Size", "Base particle size", PARAM_FLOAT, {.f_val=0.05}, 0.001, 0.2, 0.001},
    {"particle_size_variation", "Size Variation", "Random size variance", PARAM_FLOAT, {.f_val=0.5}, 0.0, 1.0, 0.01},
    {"animation_speed", "Speed", "Animation speed", PARAM_FLOAT, {.f_val=0.3}, 0.0, 5.0, 0.01},
    {"particle_color_start", "Color Start", "Color at start of life", PARAM_COLOR, {.i_val=0xCCFFCCFF}, 0, 0, 0},
    {"particle_color_end", "Color End", "Color at end of life", PARAM_COLOR, {.i_val=0x00803333}, 0, 0, 0},
    {"enable_source_brightness_affect", "Source Affect", "Source brightness affects alpha", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"source_brightness_strength", "Source Strength", "Strength of source influence", PARAM_FLOAT, {.f_val=0.75}, 0.0, 1.0, 0.01},
    {"source_brightness_threshold", "Source Threshold", "Brightness threshold", PARAM_FLOAT, {.f_val=0.2}, 0.0, 1.0, 0.01},
    {"focus_point_x", "Focus X", "Focus Center X", PARAM_FLOAT, {.f_val=0.5}, -0.5, 1.5, 0.01},
    {"focus_point_y", "Focus Y", "Focus Center Y", PARAM_FLOAT, {.f_val=0.5}, -0.5, 1.5, 0.01},
    {"focus_strength", "Focus Strength", "Blur falloff from focus", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1},
    {"motion_blur_amount", "Motion Blur", "Trail/Motion blur amount", PARAM_FLOAT, {.f_val=0.1}, 0.0, 0.95, 0.01},
    {"bokeh_edge_softness", "Softness", "Bokeh shape edge softness", PARAM_FLOAT, {.f_val=0.5}, 0.0, 1.0, 0.01},
    {"use_polygons", "Polygons", "Use polygonal shapes", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"poly_sides", "Sides", "Polygon sides", PARAM_INT, {.i_val=6}, 3, 10, 1},
    {"poly_rotation", "Rotation", "Polygon rotation", PARAM_FLOAT, {.f_val=0.0}, 0.0, 360.0, 1.0},
    {"poly_rotation_speed", "Rot Speed", "Polygon rotation speed", PARAM_FLOAT, {.f_val=0.0}, -360.0, 360.0, 1.0},
    {"enable_chromatic_aberration", "Chromatic Aberration", "Enable CA", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"ca_strength", "CA Strength", "Chromatic Aberration Amount", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1},
    {"enable_onion_rings", "Onion Rings", "Enable onion ring artifact", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"onion_ring_frequency", "Ring Freq", "Number of rings", PARAM_FLOAT, {.f_val=5.0}, 1.0, 25.0, 0.5},
    {"onion_ring_strength", "Ring Strength", "Visibility of rings", PARAM_FLOAT, {.f_val=0.4}, 0.0, 1.0, 0.01},
    {"onion_ring_animation_speed", "Ring Speed", "Ring animation speed", PARAM_FLOAT, {.f_val=0.0}, -5.0, 5.0, 0.1}
};

// --- Defaults Wrappers ---
void star_burst_defaults(obs_data_t *s) { set_defaults_from_params(s, star_burst_params, sizeof(star_burst_params)/sizeof(param_def_t)); }
void liteleke_defaults(obs_data_t *s) { set_defaults_from_params(s, liteleke_params, sizeof(liteleke_params)/sizeof(param_def_t)); }
void handheld_defaults(obs_data_t *s) { set_defaults_from_params(s, handheld_params, sizeof(handheld_params)/sizeof(param_def_t)); }
void bokeh_defaults(obs_data_t *s) { set_defaults_from_params(s, bokeh_params, sizeof(bokeh_params)/sizeof(param_def_t)); }
void style_transfer_defaults(obs_data_t *s) { (void)s; }

// --- Effect Registry ---

static const effect_info_t star_burst_info = {
    "star_burst_effect", "Star Burst", "Creates dramatic star-shaped rays", "shaders/star-burst.shader",
    star_burst_params, sizeof(star_burst_params)/sizeof(param_def_t),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, star_burst_defaults
};

static const effect_info_t liteleke_info = {
    "liteleke_effect", "Light Leak", "Adds organic light leaks", "shaders/light-leak.shader",
    liteleke_params, sizeof(liteleke_params)/sizeof(param_def_t),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, liteleke_defaults
};

static const effect_info_t handheld_info = {
    "handheld_effect", "Handheld Camera", "Simulates handheld camera movement", "shaders/handheld.shader",
    handheld_params, sizeof(handheld_params)/sizeof(param_def_t),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, handheld_defaults
};

static const effect_info_t bokeh_info = {
    "bokeh_effect", "Bokeh", "Creates beautiful bokeh light effects", "shaders/bokeh.shader",
    bokeh_params, sizeof(bokeh_params)/sizeof(param_def_t),
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

// --- Implementation Helpers ---

const char *get_effect_name(void *type_data) {
    const effect_info_t *info = type_data;
    return info ? info->name : "Unknown Effect";
}

static bool is_valid_shader_path(const char *path) {
    if (!path || strlen(path) == 0 || strlen(path) > 256) return false;
    if (strstr(path, "..") || strstr(path, "//") || strchr(path, '\\')) return false;
    if (strncmp(path, "shaders/", 8) != 0) return false;
    const char *ext = strrchr(path, '.');
    return (ext && strcmp(ext, ".shader") == 0);
}

// --- Includes for effect internals ---
#include <graphics/effect.h>

gs_effect_t *load_shader_effect(const char *shader_path) {
    if (!is_valid_shader_path(shader_path)) return NULL;
    // Explicitly find the module file using the project name
    obs_module_t *module = obs_get_module("obs-emulens");
    char *full_path = obs_find_module_file(module, shader_path);
    
    if (!full_path) return NULL;
    char *error_string = NULL;
    gs_effect_t *effect = gs_effect_create_from_file(full_path, &error_string);
    bfree(full_path);
    if (error_string) {
        blog(LOG_ERROR, "Shader load error (%s): %s", shader_path, error_string);
        bfree(error_string);
    }
    return effect;
}

// --- Generic Implementation ---

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

    ed->param_image = gs_effect_get_param_by_name(ed->effect, "image");
    ed->param_uv_size = gs_effect_get_param_by_name(ed->effect, "uv_size");
    ed->param_uv_pixel_interval = gs_effect_get_param_by_name(ed->effect, "uv_pixel_interval");
    ed->param_elapsed_time = gs_effect_get_param_by_name(ed->effect, "elapsed_time");

    if (info->num_params > 0) {
        ed->param_handles = bzalloc(sizeof(gs_eparam_t*) * info->num_params);
        for (size_t i = 0; i < info->num_params; i++) {
            ed->param_handles[i] = gs_effect_get_param_by_name(ed->effect, info->params[i].name);
            if (!ed->param_handles[i]) {
                blog(LOG_WARNING, "[%s] Parameter '%s' not found in shader", info->name, info->params[i].name);
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

    for (size_t i = 0; i < ed->info->num_params; i++) {
        const param_def_t *def = &ed->info->params[i];
        gs_eparam_t *handle = ed->param_handles[i];
        if (!handle) continue;

        switch (def->type) {
            case PARAM_FLOAT: {
                double val = obs_data_get_double(settings, def->name);
                if (val < def->min) val = def->min;
                if (val > def->max) val = def->max;
                gs_effect_set_float(handle, (float)val);
                break;
            }
            case PARAM_INT: {
                long long val = obs_data_get_int(settings, def->name);
                if (val < (long long)def->min) val = (long long)def->min;
                if (val > (long long)def->max) val = (long long)def->max;
                gs_effect_set_int(handle, (int)val);
                break;
            }
            case PARAM_BOOL: {
                bool val = obs_data_get_bool(settings, def->name);
                gs_effect_set_bool(handle, val);
                break;
            }
            case PARAM_COLOR: {
                long long val = obs_data_get_int(settings, def->name);
                struct vec4 color_vec;
                vec4_from_rgba(&color_vec, (uint32_t)val);
                gs_effect_set_vec4(handle, &color_vec);
                break;
            }
        }
    }
}

void generic_render(void *data, gs_effect_t *effect) {
    (void)effect; 
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

        // Standard Uniforms
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
        
        gs_technique_t *tech = gs_effect_get_technique(ed->effect, "Draw");
        if (!tech) tech = gs_effect_get_technique(ed->effect, "Default");
        if (!tech) tech = gs_effect_get_technique(ed->effect, 0); 
        
        if (tech) {
            gs_technique_begin(tech);
            gs_technique_begin_pass(tech, 0);
            obs_source_process_filter_tech_end(ed->context, ed->effect, 0, 0, "Draw");
            gs_technique_end_pass(tech);
            gs_technique_end(tech);
        } else {
            obs_source_skip_video_filter(ed->context);
        }
        obs_source_process_filter_end(ed->context, ed->effect, 0, 0);
    }
}

void generic_tick(void *data, float seconds) {
    effect_data_t *ed = data;
    if (ed) {
        ed->elapsed_time += seconds;
        if (ed->elapsed_time > 86400.0f) ed->elapsed_time = fmod(ed->elapsed_time, 86400.0f);
    }
}

obs_properties_t *generic_properties(void *data) {
    obs_properties_t *props = obs_properties_create();
    effect_data_t *ed = data;
    if (!ed || !ed->info) return props;

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
