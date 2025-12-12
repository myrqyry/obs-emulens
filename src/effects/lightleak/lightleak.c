/*
 * src/effects/lightleak/lightleak.c
 */

#include "lightleak.h"

static const param_def_t light_leak_params[] = {
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

static void light_leak_defaults(obs_data_t *s) {
    for (size_t i = 0; i < sizeof(light_leak_params)/sizeof(light_leak_params[0]); i++) {
        const param_def_t *def = &light_leak_params[i];
        switch (def->type) {
            case PARAM_FLOAT: obs_data_set_default_double(s, def->name, def->default_val.f_val); break;
            case PARAM_INT:   obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
            case PARAM_BOOL:  obs_data_set_default_bool(s, def->name, def->default_val.b_val); break;
            case PARAM_COLOR: obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
        }
    }
}

const effect_info_t light_leak_info = {
    "liteleke_effect", "Light Leak", "Adds organic light leaks", "shaders/light-leak.shader",
    light_leak_params, sizeof(light_leak_params)/sizeof(light_leak_params[0]),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, light_leak_defaults
};
