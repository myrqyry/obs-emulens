#pragma once

#include <obs/obs.h>
#include <graphics/graphics.h>
#include <util/bmem.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <stddef.h> // For size_t

#define MAX_EFFECT_NAME_LENGTH 64
#define MAX_SHADER_PATH_LENGTH 256
#define DEFAULT_ELAPSED_TIME_STEP 0.016f  // ~60 FPS

// Add to effects.h
#define INTENSITY_MIN 0.0f
#define INTENSITY_MAX 10.0f
#define ROTATION_MIN 0.0f
#define ROTATION_MAX 360.0f
#define SCALE_MIN 0.0f
#define SCALE_MAX 5.0f

#define FLT_EPSILON 1.19209290E-07F

#ifdef __cplusplus
extern "C" {
#endif

// Create consistent error handling macros
#define LOG_AND_RETURN_NULL(msg, ...)                                  \
	do {                                                           \
		blog(LOG_ERROR, msg, ##__VA_ARGS__);                   \
		return NULL;                                           \
	} while (0)

#define LOG_AND_RETURN_VOID(msg, ...)                                  \
	do {                                                           \
		blog(LOG_ERROR, msg, ##__VA_ARGS__);                   \
		return;                                                \
	} while (0)

// Forward declarations
struct obs_source;
typedef struct obs_source obs_source_t;
struct obs_data;
typedef struct obs_data obs_data_t;
struct gs_effect;
typedef struct gs_effect gs_effect_t;
struct gs_effect_param;
typedef struct gs_effect_param gs_eparam_t;

// Enum for effect types
typedef enum {
    EFFECT_STAR_BURST,
    EFFECT_LITELEKE,
    EFFECT_HANDHELD,
    EFFECT_BOKEH
} effect_type_t;

// Structure to hold effect instance data
typedef struct {
    obs_source_t *context;
    gs_effect_t *effect;
    gs_eparam_t *image_param;
    gs_eparam_t *uv_size_param;
    gs_eparam_t *elapsed_time_param;
    // Cache effect-specific parameters
    gs_eparam_t *intensity_param;
    gs_eparam_t *rotation_param;
    gs_eparam_t *color_param;
    gs_eparam_t *scale_param;
    gs_eparam_t *speed_param;
    gs_eparam_t *shake_param;
    gs_eparam_t *zoom_param;
    gs_eparam_t *radius_param;
    gs_eparam_t *samples_param;
    float elapsed_time;
    effect_type_t type;

    // Cache previous values to avoid unnecessary GPU updates
    float cached_intensity;
    float cached_rotation;
    uint32_t cached_color;
    float cached_scale;
    float cached_speed;
    float cached_shake;
    float cached_zoom;
    float cached_radius;
    int cached_samples;
    bool values_dirty;
} effect_data_t;

// Structure to hold effect information
typedef struct {
	const char *id;
	const char *name;
	const char *description;
	const char *shader_path;
	effect_type_t type; // Add this field
	void *(*create_effect)(obs_data_t *settings, obs_source_t *source);
	void (*destroy_effect)(void *data);
	void (*update_effect)(void *data, obs_data_t *settings);
	void (*video_render)(void *data, gs_effect_t *effect);
	void (*video_tick)(void *data, float seconds);
	obs_properties_t *(*get_properties)(void *data);
	void (*get_defaults)(obs_data_t *settings);
} effect_info_t;

// Function declarations
obs_properties_t *effect_properties(void *data);
void effect_defaults(obs_data_t *settings);

// Declarations for effect-specific default functions
void star_burst_defaults(obs_data_t *settings);
void liteleke_defaults(obs_data_t *settings);
void handheld_defaults(obs_data_t *settings);
void bokeh_defaults(obs_data_t *settings);

// Declare effects
extern const effect_info_t star_burst_effect;
extern const effect_info_t liteleke_effect;
extern const effect_info_t handheld_effect;
extern const effect_info_t bokeh_effect;

// Array of all available effects
extern const effect_info_t *effects[];
extern const size_t num_effects;

// Function declarations
const char *get_effect_name(void *type_data);
void *effect_create(obs_data_t *settings, obs_source_t *source);
void effect_destroy(void *data);
void effect_update(void *data, obs_data_t *settings);
void effect_video_render(void *data, gs_effect_t *effect);
void effect_video_tick(void *data, float seconds);

#ifdef __cplusplus
}
#endif
