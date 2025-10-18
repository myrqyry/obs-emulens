#include "effects.h"
#include <obs-module.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include <util/dstr.h>

// Helper function to load shader from file
gs_effect_t *load_shader_effect(const char *shader_path)
{
    char *full_path = obs_module_file(shader_path);
    if (!full_path) {
        blog(LOG_ERROR, "Could not find shader file: %s", shader_path);
        return NULL;
    }

    char *error_string = NULL;
    gs_effect_t *effect = gs_effect_create_from_file(full_path, &error_string);
    bfree(full_path);

    if (!effect) {
        blog(LOG_ERROR, "Failed to load shader: %s", error_string ? error_string : "Unknown error");
        bfree(error_string);
    }

    return effect;
}

// Forward declarations
const char *get_effect_name(void *type_data);
void *effect_create(obs_data_t *settings, obs_source_t *source);
void effect_destroy(void *data);
void effect_update(void *data, obs_data_t *settings);
void effect_video_render(void *data, gs_effect_t *effect);
void effect_video_tick(void *data, float seconds);
obs_properties_t *effect_get_properties(void *data);
void effect_get_defaults(obs_data_t *settings);

// Effect information for Star Burst
const effect_info_t star_burst_effect = {
    .id = "star_burst_effect",
    .name = "Star Burst",
    .description = "Creates dramatic star-shaped rays from bright light sources",
    .shader_path = "shaders/star-burst.shader",
    .create_effect = effect_create,
    .destroy_effect = effect_destroy,
    .update_effect = effect_update,
    .video_render = effect_video_render,
    .video_tick = effect_video_tick,
    .get_properties = effect_get_properties,
    .get_defaults = effect_get_defaults
};

// Effect information for Liteleke
const effect_info_t liteleke_effect = {
    .id = "liteleke_effect",
    .name = "Light Leak",
    .description = "Adds organic light leak effects",
    .shader_path = "shaders/light-leak.shader",
    .create_effect = effect_create,
    .destroy_effect = effect_destroy,
    .update_effect = effect_update,
    .video_render = effect_video_render,
    .video_tick = effect_video_tick,
    .get_properties = effect_get_properties,
    .get_defaults = effect_get_defaults
};

// Effect information for Handheld
const effect_info_t handheld_effect = {
    .id = "handheld_effect",
    .name = "Handheld Camera",
    .description = "Simulates handheld camera movement and breathing",
    .shader_path = "shaders/handheld.shader",
    .create_effect = effect_create,
    .destroy_effect = effect_destroy,
    .update_effect = effect_update,
    .video_render = effect_video_render,
    .video_tick = effect_video_tick,
    .get_properties = effect_get_properties,
    .get_defaults = effect_get_defaults
};

// Effect information for Bokeh
const effect_info_t bokeh_effect = {
    .id = "bokeh_effect",
    .name = "Bokeh",
    .description = "Creates beautiful bokeh light effects",
    .shader_path = "shaders/bokeh.shader",
    .create_effect = effect_create,
    .destroy_effect = effect_destroy,
    .update_effect = effect_update,
    .video_render = effect_video_render,
    .video_tick = effect_video_tick,
    .get_properties = effect_get_properties,
    .get_defaults = effect_get_defaults
};

// Array of all available effects
const effect_info_t *effects[] = {
    &star_burst_effect,
    &liteleke_effect,
    &handheld_effect,
    &bokeh_effect
};

const size_t num_effects = sizeof(effects) / sizeof(effects[0]);

const char *get_effect_name(void *type_data)
{
    const effect_info_t *info = type_data;
    return info->name;
}

void *effect_create(obs_data_t *settings, obs_source_t *source)
{
    const effect_info_t *info = obs_source_get_type_data(source);
    effect_data_t *data = bzalloc(sizeof(effect_data_t));
    data->context = source;
    data->elapsed_time = 0.0f;

    // Load the shader effect
    data->effect = load_shader_effect(info->shader_path);
    if (!data->effect) {
        bfree(data);
        return NULL;
    }

    // Get shader parameters
    data->image_param = gs_effect_get_param_by_name(data->effect, "image");
    data->uv_size_param = gs_effect_get_param_by_name(data->effect, "uv_size");
    data->elapsed_time_param = gs_effect_get_param_by_name(data->effect, "elapsed_time");

    return data;
}

void effect_destroy(void *data)
{
    effect_data_t *ed = data;
    if (!ed) return;
    
    if (ed->effect) {
        gs_effect_destroy(ed->effect);
    }
    
    bfree(ed);
}

void effect_update(void *data, obs_data_t *settings)
{
    effect_data_t *ed = data;
    if (!ed || !ed->effect) return;

    // Get shader parameters
    gs_eparam_t *rotation_param = gs_effect_get_param_by_name(ed->effect, "rotation");
    gs_eparam_t *intensity_param = gs_effect_get_param_by_name(ed->effect, "intensity");
    gs_eparam_t *color_param = gs_effect_get_param_by_name(ed->effect, "color");

    // Update shader parameters from settings
    if (rotation_param) {
        float rotation = (float)obs_data_get_double(settings, "rotation");
        gs_effect_set_float(rotation_param, rotation);
    }
    
    if (intensity_param) {
        float intensity = (float)obs_data_get_double(settings, "intensity");
        gs_effect_set_float(intensity_param, intensity);
    }
    
    if (color_param) {
        uint32_t color = (uint32_t)obs_data_get_int(settings, "color");
        struct vec4 color_vec;
        vec4_from_rgba(&color_vec, color);
        gs_effect_set_vec4(color_param, &color_vec);
    }
}

void effect_video_render(void *data, gs_effect_t *effect)
{
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
        // Set shader parameters
        if (ed->uv_size_param) {
            uint32_t width = obs_source_get_width(target);
            uint32_t height = obs_source_get_height(target);
            gs_effect_set_vec2(ed->uv_size_param, &(struct vec2){(float)width, (float)height});
        }

        if (ed->elapsed_time_param) {
            gs_effect_set_float(ed->elapsed_time_param, ed->elapsed_time);
        }

        // Apply the effect
        obs_source_process_filter_tech_draw(ed->context, ed->effect, "Draw");
        obs_source_process_filter_end(ed->context);
    }
}

void effect_video_tick(void *data, float seconds)
{
    effect_data_t *ed = data;
    if (!ed) return;
    
    // Update elapsed time for animations
    ed->elapsed_time += seconds;
}

obs_properties_t *effect_get_properties(void *data)
{
    const effect_info_t *info = data;
    obs_properties_t *props = obs_properties_create();
    
    if (strcmp(info->id, "star_burst_effect") == 0) {
        obs_properties_add_float_slider(props, "rotation", "Rotation", 0.0, 360.0, 1.0);
        obs_properties_add_float_slider(props, "intensity", "Intensity", 0.0, 10.0, 0.1);
        obs_properties_add_color(props, "color", "Color");
    }
    
    return props;
}

void effect_get_defaults(obs_data_t *settings)
{
    obs_data_set_default_double(settings, "rotation", 0.0);
    obs_data_set_default_double(settings, "intensity", 1.0);
    obs_data_set_default_int(settings, "color", 0xFFFFFFFF);
}
