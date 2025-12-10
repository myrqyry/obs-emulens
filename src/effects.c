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
// Defaults requires specific wrappers, declared below.

// --- Parameter Definitions ---

static const param_def_t star_burst_params[] = {
    {"intensity", "Intensity", "Strength of the rays", PARAM_FLOAT, {.f_val=1.0}, 0.0, 10.0, 0.1},
    {"rotation", "Rotation", "Rotation angle in degrees", PARAM_FLOAT, {.f_val=0.0}, 0.0, 360.0, 1.0},
    {"color", "Color", "Tint color of the rays", PARAM_COLOR, {.i_val=0xFFFFFFFF}, 0, 0, 0}
};

static const param_def_t liteleke_params[] = {
    {"intensity", "Intensity", "Opacity of the light leak", PARAM_FLOAT, {.f_val=1.0}, 0.0, 1.0, 0.05},
    {"scale", "Scale", "Size of the leak pattern", PARAM_FLOAT, {.f_val=1.0}, 0.0, 5.0, 0.1}
};

static const param_def_t handheld_params[] = {
    {"shake_speed", "Shake Speed", "Speed of camera movement", PARAM_FLOAT, {.f_val=10.0}, 0.0, 20.0, 0.5},
    {"shake_intensity", "Shake Intensity", "Amplitude of shake", PARAM_FLOAT, {.f_val=0.05}, 0.0, 1.0, 0.05},
    {"zoom_amount", "Zoom Amount", "Zoom breathing effect", PARAM_FLOAT, {.f_val=0.02}, 0.0, 0.5, 0.01}
};

static const param_def_t bokeh_params[] = {
    {"radius", "Radius", "Blur radius", PARAM_FLOAT, {.f_val=10.0}, 0.0, 50.0, 1.0},
    {"samples", "Samples", "Number of blur samples (Quality)", PARAM_INT, {.i_val=32}, 1.0, 64.0, 1.0}
};

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

// --- Specific Defaults Wrappers ---
void star_burst_defaults(obs_data_t *s) { set_defaults_from_params(s, star_burst_params, sizeof(star_burst_params)/sizeof(param_def_t)); }
void liteleke_defaults(obs_data_t *s) { set_defaults_from_params(s, liteleke_params, sizeof(liteleke_params)/sizeof(param_def_t)); }
void handheld_defaults(obs_data_t *s) { set_defaults_from_params(s, handheld_params, sizeof(handheld_params)/sizeof(param_def_t)); }
void bokeh_defaults(obs_data_t *s) { set_defaults_from_params(s, bokeh_params, sizeof(bokeh_params)/sizeof(param_def_t)); }
void style_transfer_defaults(obs_data_t *s) { (void)s; /* No params */ }

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
    NULL, 0, // No params yet
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

gs_effect_t *load_shader_effect(const char *shader_path) {
    if (!is_valid_shader_path(shader_path)) return NULL;
    char *full_path = obs_module_file(shader_path);
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
    (void)settings;
    const effect_info_t *info = obs_source_get_type_data(source);
    
    effect_data_t *ed = bzalloc(sizeof(effect_data_t));
    ed->context = source;
    ed->info = info;
    ed->effect = load_shader_effect(info->shader_path);
    
    if (!ed->effect) {
        bfree(ed);
        return NULL;
    }

    ed->param_image = gs_effect_get_param_by_name(ed->effect, "image");
    ed->param_uv_size = gs_effect_get_param_by_name(ed->effect, "uv_size");
    ed->param_elapsed_time = gs_effect_get_param_by_name(ed->effect, "elapsed_time");

    // Dynamic Parameter Binding
    if (info->num_params > 0) {
        ed->param_handles = bzalloc(sizeof(gs_eparam_t*) * info->num_params);
        for (size_t i = 0; i < info->num_params; i++) {
            ed->param_handles[i] = gs_effect_get_param_by_name(ed->effect, info->params[i].name);
            if (!ed->param_handles[i]) {
                blog(LOG_WARNING, "[%s] Warning: Shader parameter '%s' not found in %s", 
                     info->name, info->params[i].name, info->shader_path);
            }
        }
    }

    obs_source_update(source, settings);
    return ed;
}

void generic_destroy(void *data) {
    effect_data_t *ed = data;
    if (ed) {
        if (ed->effect) gs_effect_destroy(ed->effect);
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
                // Clamp
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
                // Shader often expects vec4 for color, ensure w=1.0 if not using alpha in hex
                // But obs_properties_add_color returns int with alpha. 
                // We should respect the alpha.
                gs_effect_set_vec4(handle, &color_vec);
                break;
            }
        }
    }
}

void generic_render(void *data, gs_effect_t *effect) {
    (void)effect; // We use the internal effect from data
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
        // Standard Uniforms
        if (ed->param_uv_size) {
            struct vec2 uv = {
                .x = (float)obs_source_get_width(target),
                .y = (float)obs_source_get_height(target)
            };
            gs_effect_set_vec2(ed->param_uv_size, &uv);
        }
        if (ed->param_elapsed_time) {
            gs_effect_set_float(ed->param_elapsed_time, ed->elapsed_time);
        }
        
        // Draw
        gs_technique_t *tech = gs_effect_get_technique(ed->effect, "Draw");
        if (!tech) tech = gs_effect_get_technique(ed->effect, "Default");
        if (!tech) tech = gs_effect_get_technique(ed->effect, 0); // last resort
        
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
        if (ed->elapsed_time > 86400.0f) ed->elapsed_time = 0.0f;
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
