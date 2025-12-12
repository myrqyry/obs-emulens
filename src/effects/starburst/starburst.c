/*
 * src/effects/starburst/starburst.c
 */

#include "starburst.h"

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

static void star_burst_defaults(obs_data_t *s) {
    for (size_t i = 0; i < sizeof(star_burst_params)/sizeof(star_burst_params[0]); i++) {
        const param_def_t *def = &star_burst_params[i];
        switch (def->type) {
            case PARAM_FLOAT: obs_data_set_default_double(s, def->name, def->default_val.f_val); break;
            case PARAM_INT:   obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
            case PARAM_BOOL:  obs_data_set_default_bool(s, def->name, def->default_val.b_val); break;
            case PARAM_COLOR: obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
        }
    }
}

const effect_info_t star_burst_info = {
    "star_burst_effect", "Star Burst", "Creates dramatic star-shaped rays", "shaders/star-burst.shader",
    star_burst_params, sizeof(star_burst_params)/sizeof(star_burst_params[0]),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, star_burst_defaults
};
