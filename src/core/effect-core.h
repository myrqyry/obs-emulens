/*
 * src/core/effect-core.h
 * Core definitions for the data-driven effect system
 */

#pragma once

#include <obs/obs.h>
#include <graphics/graphics.h>
#include <util/bmem.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <stddef.h> 

#define MAX_EFFECT_NAME_LENGTH 64
#define MAX_SHADER_PATH_LENGTH 256
#define DEFAULT_ELAPSED_TIME_STEP 0.016f

#ifdef __cplusplus
extern "C" {
#endif

// --- Data-Driven Parameter System ---

typedef enum {
    PARAM_FLOAT,
    PARAM_INT,
    PARAM_COLOR,
    PARAM_BOOL
} param_type_t;

// Schema definition for a single parameter
typedef struct {
    const char *name;           // Shader uniform name AND OBS property name
    const char *display_name;   // GUI Label
    const char *description;    // Tooltip (Optional)
    param_type_t type;
    
    // Default values union
    union {
        double f_val;
        long long i_val;
        bool   b_val;
    } default_val;

    // Range constraints (for sliders)
    double min;
    double max;
    double step;
} param_def_t;

// --- Effect Structures ---

// Forward declarations
struct obs_source;
typedef struct obs_source obs_source_t;
struct obs_data;
typedef struct obs_data obs_data_t;
struct gs_effect;
typedef struct gs_effect gs_effect_t;
struct gs_effect_param;
typedef struct gs_effect_param gs_eparam_t;
struct obs_properties;
typedef struct obs_properties obs_properties_t;

typedef struct {
    const char *id;
    const char *name;
    const char *description;
    const char *shader_path;
    
    // Parameter Metadata (The Contract)
    const param_def_t *params;
    size_t num_params;

    // Standard callbacks (can point to generics or specifics)
    void *(*create)(obs_data_t *settings, obs_source_t *source);
    void (*destroy)(void *data);
    void (*update)(void *data, obs_data_t *settings);
    void (*video_render)(void *data, gs_effect_t *effect);
    void (*video_tick)(void *data, float seconds);
    obs_properties_t *(*get_properties)(void *data);
    void (*get_defaults)(obs_data_t *settings);
} effect_info_t;

// Runtime data for an active effect instance
typedef struct {
    obs_source_t *context;
    const effect_info_t *info;
    gs_effect_t *effect;
    
    // Standard Uniforms
    gs_eparam_t *param_image;
    gs_eparam_t *param_uv_size;
    gs_eparam_t *param_uv_pixel_interval;
    gs_eparam_t *param_elapsed_time;

    // Dynamic Parameter Handles (index matches info->params index)
    gs_eparam_t **param_handles;
    
    // Caching for dirty checks
    float *cached_float_values;
    int *cached_int_values;
    bool *cached_bool_values;
    uint32_t *cached_color_values;
    
    float elapsed_time;
} effect_data_t;

// --- Function Prototypes ---

// Lifecycle
void *generic_create(obs_data_t *settings, obs_source_t *source);
void generic_destroy(void *data);
void generic_render(void *data, gs_effect_t *effect);
void generic_tick(void *data, float seconds);

// Shader Loading
gs_effect_t *load_shader_effect(const char *shader_path);
bool is_valid_shader_path(const char *path);

// Parameter System
void bind_effect_parameters(effect_data_t *ed);
void generic_update(void *data, obs_data_t *settings);
obs_properties_t *generic_properties(void *data);

#ifdef __cplusplus
}
#endif
