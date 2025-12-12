/*
 * src/effects/bokeh/bokeh.c
 */

#include "bokeh.h"

static const param_def_t bokeh_params[] = {
    {"particle_density", "Density", "Particle count density", PARAM_FLOAT, {.f_val=25.0}, 1.0, 100.0, 1.0},
    {"particle_base_size", "Size", "Base particle size", PARAM_FLOAT, {.f_val=0.05}, 0.001, 0.2, 0.001},
    {"focus_strength", "Focus Strength", "Blur falloff from focus", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1},
    {"motion_blur_amount", "Motion Blur", "Trail/Motion blur amount", PARAM_FLOAT, {.f_val=0.1}, 0.0, 0.95, 0.01},
    {"bokeh_edge_softness", "Softness", "Bokeh shape edge softness", PARAM_FLOAT, {.f_val=0.5}, 0.0, 1.0, 0.01},
    {"enable_chromatic_aberration", "Chromatic Aberration", "Enable CA", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"ca_strength", "CA Strength", "Chromatic Aberration Amount", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1}
};

static void bokeh_defaults(obs_data_t *s) {
    for (size_t i = 0; i < sizeof(bokeh_params)/sizeof(bokeh_params[0]); i++) {
        const param_def_t *def = &bokeh_params[i];
        switch (def->type) {
            case PARAM_FLOAT: obs_data_set_default_double(s, def->name, def->default_val.f_val); break;
            case PARAM_INT:   obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
            case PARAM_BOOL:  obs_data_set_default_bool(s, def->name, def->default_val.b_val); break;
            case PARAM_COLOR: obs_data_set_default_int(s, def->name, def->default_val.i_val); break;
        }
    }
}

const effect_info_t bokeh_info = {
    "bokeh_effect", "Bokeh", "Creates beautiful bokeh light effects", "shaders/bokeh.shader",
    bokeh_params, sizeof(bokeh_params)/sizeof(bokeh_params[0]),
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, bokeh_defaults
};
