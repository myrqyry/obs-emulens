/*
 * src/effects/handheld/handheld.c
 */

#include "handheld.h"

static const param_def_t handheld_params[] = {
    {"preset", "Preset", "0:Stable 1:Breath 2:Handheld 3:Shaky 4:Quake 99:Custom", PARAM_INT, {.i_val=2}, 0, 99, 1},
    {"masterIntensity", "Master Intensity", "Global strength multiplier", PARAM_FLOAT, {.f_val=1.0}, 0.0, 2.0, 0.05},
    {"positionSpeed", "Pos Speed", "Position shake frequency", PARAM_FLOAT, {.f_val=1.5}, 0.1, 10.0, 0.1},
    {"rotationSpeed", "Rot Speed", "Rotation shake frequency", PARAM_FLOAT, {.f_val=1.0}, 0.1, 10.0, 0.1},
    {"zoomSpeed", "Zoom Speed", "Zoom breathing frequency", PARAM_FLOAT, {.f_val=0.8}, 0.1, 10.0, 0.1},
    {"enableDynamicBlur", "Motion Blur", "Enable dynamic motion blur", PARAM_BOOL, {.b_val=true}, 0, 0, 0},
    {"blurAmount", "Blur Amount", "Strength of motion blur", PARAM_FLOAT, {.f_val=1.0}, 0.0, 5.0, 0.1}
};

static void handheld_defaults(obs_data_t *s) {
    for (size_t i = 0; i < sizeof(handheld_params)/sizeof(handheld_params[0]); i++) {
        const param_def_t *def = &handheld_params[i];
        switch (def->type) {
            case PARAM_FLOAT: obs_data_set_default_double(s, def->name, def->default_val.f_val); break;
            case PARAM_INT:   obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
            case PARAM_BOOL:  obs_data_set_default_bool(s, def->name, def->default_val.b_val); break;
            case PARAM_COLOR: obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
        }
    }
}

const effect_info_t handheld_info = {
    "handheld_effect", "Handheld Camera", "Simulates handheld camera movement", "shaders/handheld.shader",
    handheld_params, sizeof(handheld_params)/sizeof(handheld_params[0]),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, handheld_defaults
};
