#include "effects.h"
#include <obs-module.h>
#include <graphics/graphics.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <math.h>
#include <string.h>

// Helper function to validate shader path to prevent directory traversal
static bool is_valid_shader_path(const char *path)
{
	if (!path || strlen(path) == 0)
		return false;

	// Check for directory traversal patterns
	if (strstr(path, "..") || strstr(path, "//") || strchr(path, '\\') ||
	    path[0] == '/') {
		return false;
	}

	// Ensure it's in the shaders directory and has proper extension
	if (strncmp(path, "shaders/", 8) != 0)
		return false;

	const char *ext = strrchr(path, '.');
	if (!ext || strcmp(ext, ".shader") != 0)
		return false;

	return true;
}

// Helper function to load shader from file
gs_effect_t *load_shader_effect(const char *shader_path)
{
	if (!is_valid_shader_path(shader_path)) {
		blog(LOG_ERROR, "Invalid shader path: %s",
		     shader_path ? shader_path : "NULL");
		return NULL;
	}

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
    } else if (error_string) {
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
obs_properties_t *effect_properties(void *data);
void effect_defaults(obs_data_t *settings);

// Effect information for Star Burst
const effect_info_t star_burst_effect = {
	.id = "star_burst_effect",
	.name = "Star Burst",
	.description =
		"Creates dramatic star-shaped rays from bright light sources",
	.shader_path = "shaders/star-burst.shader",
	.type = EFFECT_STAR_BURST,
	.create_effect = effect_create,
	.destroy_effect = effect_destroy,
	.update_effect = effect_update,
	.video_render = effect_video_render,
	.video_tick = effect_video_tick,
	.get_properties = effect_properties,
	.get_defaults = star_burst_defaults};

// Effect information for Liteleke
const effect_info_t liteleke_effect = {
	.id = "liteleke_effect",
	.name = "Light Leak",
	.description = "Adds organic light leak effects",
	.shader_path = "shaders/light-leak.shader",
	.type = EFFECT_LITELEKE,
	.create_effect = effect_create,
	.destroy_effect = effect_destroy,
	.update_effect = effect_update,
	.video_render = effect_video_render,
	.video_tick = effect_video_tick,
	.get_properties = effect_properties,
	.get_defaults = liteleke_defaults};

// Effect information for Handheld
const effect_info_t handheld_effect = {
	.id = "handheld_effect",
	.name = "Handheld Camera",
	.description = "Simulates handheld camera movement and breathing",
	.shader_path = "shaders/handheld.shader",
	.type = EFFECT_HANDHELD,
	.create_effect = effect_create,
	.destroy_effect = effect_destroy,
	.update_effect = effect_update,
	.video_render = effect_video_render,
	.video_tick = effect_video_tick,
	.get_properties = effect_properties,
	.get_defaults = handheld_defaults};

// Effect information for Bokeh
const effect_info_t bokeh_effect = {
	.id = "bokeh_effect",
	.name = "Bokeh",
	.description = "Creates beautiful bokeh light effects",
	.shader_path = "shaders/bokeh.shader",
	.type = EFFECT_BOKEH,
	.create_effect = effect_create,
	.destroy_effect = effect_destroy,
	.update_effect = effect_update,
	.video_render = effect_video_render,
	.video_tick = effect_video_tick,
	.get_properties = effect_properties,
	.get_defaults = bokeh_defaults};

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
    if (!info) {
        return "Unknown Effect";
    }
    return info->name;
}

void *effect_create(obs_data_t *settings, obs_source_t *source)
{
	const effect_info_t *info = obs_source_get_type_data(source);
	if (!info || !info->shader_path)
		LOG_AND_RETURN_NULL(
			"Invalid effect info or missing shader path");

	effect_data_t *data = bzalloc(sizeof(effect_data_t));
	if (!data)
		LOG_AND_RETURN_NULL(
			"Failed to allocate memory for effect data");

	data->context = source;
	data->elapsed_time = 0.0f;

	// Load the shader effect
	data->effect = load_shader_effect(info->shader_path);
	if (!data->effect) {
		bfree(data);
		LOG_AND_RETURN_NULL("Failed to load shader: %s",
				    info->shader_path);
	}

    // Get shader parameters
    data->image_param = gs_effect_get_param_by_name(data->effect, "image");
    data->uv_size_param = gs_effect_get_param_by_name(data->effect, "uv_size");
    data->elapsed_time_param = gs_effect_get_param_by_name(data->effect, "elapsed_time");

    // Set effect type for optimized lookups
    data->type = info->type;

    switch (data->type) {
    case EFFECT_STAR_BURST:
        data->intensity_param = gs_effect_get_param_by_name(data->effect, "intensity");
        data->rotation_param = gs_effect_get_param_by_name(data->effect, "rotation");
        data->color_param = gs_effect_get_param_by_name(data->effect, "color");
        break;
    case EFFECT_LITELEKE:
        data->intensity_param = gs_effect_get_param_by_name(data->effect, "intensity");
        data->scale_param = gs_effect_get_param_by_name(data->effect, "scale");
        break;
    case EFFECT_HANDHELD:
        data->speed_param = gs_effect_get_param_by_name(data->effect, "shake_speed");
        data->shake_param = gs_effect_get_param_by_name(data->effect, "shake_intensity");
        data->zoom_param = gs_effect_get_param_by_name(data->effect, "zoom_amount");
        break;
    case EFFECT_BOKEH:
        data->radius_param = gs_effect_get_param_by_name(data->effect, "radius");
        data->samples_param = gs_effect_get_param_by_name(data->effect, "samples");
        break;
    }

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

    const effect_info_t *info = obs_source_get_type_data(ed->context);
    if (!info) return;

    // Effect-specific parameters
    switch (ed->type) {
    case EFFECT_STAR_BURST: {
        if (ed->intensity_param) {
            float intensity = (float)obs_data_get_double(settings, "intensity");
            intensity = fmax(0.0f, fmin(10.0f, intensity));
            gs_effect_set_float(ed->intensity_param, intensity);
        } else {
		blog(LOG_WARNING, "Intensity parameter not found in shader");
	}
        if (ed->rotation_param) {
            float rotation = (float)obs_data_get_double(settings, "rotation");
            rotation = fmax(0.0f, fmin(360.0f, rotation));
            gs_effect_set_float(ed->rotation_param, rotation);
        }
        if (ed->color_param) {
            uint32_t color = (uint32_t)obs_data_get_int(settings, "color");
            struct vec4 color_vec;
            vec4_from_rgba(&color_vec, color);
            color_vec.w = 1.0f;
            gs_effect_set_vec4(ed->color_param, &color_vec);
        }
        break;
    }
    case EFFECT_LITELEKE: {
        if (ed->intensity_param) {
            float intensity = (float)obs_data_get_double(settings, "intensity");
            intensity = fmax(0.0f, fmin(1.0f, intensity));
            gs_effect_set_float(ed->intensity_param, intensity);
        } else {
		blog(LOG_WARNING, "Intensity parameter not found in shader");
	}
        if (ed->scale_param) {
            float scale = (float)obs_data_get_double(settings, "scale");
            scale = fmax(0.0f, fmin(5.0f, scale));
            gs_effect_set_float(ed->scale_param, scale);
        }
        break;
    }
    case EFFECT_HANDHELD: {
        if (ed->speed_param) {
            float speed = (float)obs_data_get_double(settings, "shake_speed");
            speed = fmax(0.0f, fmin(20.0f, speed));
            gs_effect_set_float(ed->speed_param, speed);
        }
        if (ed->shake_param) {
            float shake = (float)obs_data_get_double(settings, "shake_intensity");
            shake = fmax(0.0f, fmin(1.0f, shake));
            gs_effect_set_float(ed->shake_param, shake);
        }
        if (ed->zoom_param) {
            float zoom = (float)obs_data_get_double(settings, "zoom_amount");
            zoom = fmax(0.0f, fmin(0.5f, zoom));
            gs_effect_set_float(ed->zoom_param, zoom);
        }
        break;
    }
    case EFFECT_BOKEH: {
        if (ed->radius_param) {
            float radius = (float)obs_data_get_double(settings, "radius");
            radius = fmax(0.0f, fmin(50.0f, radius));
            gs_effect_set_float(ed->radius_param, radius);
        }
        if (ed->samples_param) {
            int samples = (int)obs_data_get_int(settings, "samples");
            samples = (samples < 1) ? 1 : ((samples > 64) ? 64 : samples);
            gs_effect_set_int(ed->samples_param, samples);
        }
        break;
    }
    }
}

void effect_video_render(void *data, gs_effect_t *effect)
{
	effect_data_t *ed = data;
	if (!ed) {
		LOG_AND_RETURN_VOID("Effect data is NULL in video_render");
	}

	if (!ed->effect) {
		obs_source_skip_video_filter(ed->context);
		LOG_AND_RETURN_VOID("Effect shader is NULL for effect type %d",
				    ed->type);
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

obs_properties_t *effect_properties(void *data)
{
    effect_data_t *ed = data;
    if (!ed || !ed->context) {
        return obs_properties_create();
    }

    const effect_info_t *info = obs_source_get_type_data(ed->context);
    if (!info) {
        return obs_properties_create();
    }

    obs_properties_t *props = obs_properties_create();

    switch (info->type) {
    case EFFECT_STAR_BURST:
        obs_properties_add_float_slider(props, "rotation", "Rotation", 0.0, 360.0, 1.0);
        obs_properties_add_float_slider(props, "intensity", "Intensity", 0.0, 10.0, 0.1);
        obs_properties_add_color(props, "color", "Color");
        break;
    case EFFECT_LITELEKE:
        obs_properties_add_float_slider(props, "intensity", "Intensity", 0.0, 1.0, 0.05);
        obs_properties_add_float_slider(props, "scale", "Scale", 0.0, 5.0, 0.1);
        break;
    case EFFECT_HANDHELD:
        obs_properties_add_float_slider(props, "shake_speed", "Shake Speed", 0.0, 20.0, 0.5);
        obs_properties_add_float_slider(props, "shake_intensity", "Shake Intensity", 0.0, 1.0, 0.05);
        obs_properties_add_float_slider(props, "zoom_amount", "Zoom Amount", 0.0, 0.5, 0.01);
        break;
    case EFFECT_BOKEH:
        obs_properties_add_float_slider(props, "radius", "Radius", 0.0, 50.0, 1.0);
        obs_properties_add_int_slider(props, "samples", "Samples", 1, 64, 1);
        break;
    }
    
    return props;
}

void star_burst_defaults(obs_data_t *settings)
{
    obs_data_set_default_double(settings, "rotation", 0.0);
    obs_data_set_default_double(settings, "intensity", 1.0);
    obs_data_set_default_int(settings, "color", 0xFFFFFFFF);
}

void liteleke_defaults(obs_data_t *settings)
{
    obs_data_set_default_double(settings, "intensity", 1.0);
    obs_data_set_default_double(settings, "scale", 1.0);
}

void handheld_defaults(obs_data_t *settings)
{
    obs_data_set_default_double(settings, "shake_speed", 10.0);
    obs_data_set_default_double(settings, "shake_intensity", 0.05);
    obs_data_set_default_double(settings, "zoom_amount", 0.02);
}

void bokeh_defaults(obs_data_t *settings)
{
    obs_data_set_default_double(settings, "radius", 10.0);
    obs_data_set_default_int(settings, "samples", 32);
}

void effect_defaults(obs_data_t *settings)
{
    // This function is now a placeholder and can be removed if all effects have their own defaults.
    // For now, we'll leave it empty.
}
