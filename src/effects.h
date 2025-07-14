#pragma once

#include <graphics/graphics.h>
#include <util/bmem.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <stddef.h> // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
struct obs_source;
typedef struct obs_source obs_source_t;
struct obs_data;
typedef struct obs_data obs_data_t;
struct gs_effect;
typedef struct gs_effect gs_effect_t;
struct gs_effect_param;
typedef struct gs_effect_param gs_eparam_t;

// Structure to hold effect instance data
typedef struct {
    obs_source_t *context;
    gs_effect_t *effect;
    gs_eparam_t *image_param;
    gs_eparam_t *uv_size_param;
    gs_eparam_t *elapsed_time_param;
    float elapsed_time;
} effect_data_t;

// Structure to hold effect information
typedef struct {
    const char *id;
    const char *name;
    const char *description;
    const char *shader_path;
    void *(*create_effect)(obs_data_t *settings, obs_source_t *source);
    void (*destroy_effect)(void *data);
    void (*update_effect)(void *data, obs_data_t *settings);
    void (*video_render)(void *data, gs_effect_t *effect);
    void (*video_tick)(void *data, float seconds);
    obs_properties_t *(*get_properties)(void *data);
    void (*get_defaults)(obs_data_t *settings);
} effect_info_t;

// Declare effects
extern const effect_info_t star_burst_effect;
extern const effect_info_t liteleke_effect;
extern const effect_info_t handheld_effect;
extern const effect_info_t bokeh_effect;

// Array of all available effects
#define NUM_EFFECTS 4
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
