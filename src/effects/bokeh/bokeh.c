/*
 * src/effects/bokeh/bokeh.c
 */

#include "bokeh.h"

static const param_def_t bokeh_params[] = {
    {"particle_density", "Density", "Particle count density", PARAM_FLOAT, {.f_val=25.0}, 1.0, 100.0, 1.0},
    {"particle_base_size", "Size", "Base particle size", PARAM_FLOAT, {.f_val=0.05}, 0.001, 0.2, 0.001},
    {"particle_size_variation", "Size Variation", "Randomizes particle size", PARAM_FLOAT, {.f_val=0.5}, 0.0, 1.0, 0.01},
    {"animation_speed", "Animation Speed", "Speed of the effect", PARAM_FLOAT, {.f_val=0.3}, 0.0, 5.0, 0.01},
    {"particle_color_start", "Color Start", "Particle color at start of life", PARAM_COLOR, {.i_val=0xCCFFCCCC}, 0, 0, 0}, // approx 0.8, 0.8, 1.0, 0.8 (RGBA) -> ABGR? OBS uses 0xAABBGGRR usually? No, OBS color is often 0xFFRRGGBB or ABGR.
    // defaults: 0.8, 0.8, 1.0, 0.8. In hex (ARGB): 0xCC CCCC FF. 
    // OBS `obs_data_set_default_int` for color usually takes 0xBBGGRR or 0xAABBGGRR.
    // Let's assume standard integer color. 
    {"particle_color_end", "Color End", "Particle color at end of life", PARAM_COLOR, {.i_val=0x00803333}, 0, 0, 0},

    {"enable_source_brightness_affect", "Source Brightness Affect", "Particles affected by source brightness", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"source_brightness_strength", "Brightness Strength", "Strength of source brightness effect", PARAM_FLOAT, {.f_val=0.75}, 0.0, 1.0, 0.01},
    {"source_brightness_threshold", "Brightness Threshold", "Threshold for source brightness", PARAM_FLOAT, {.f_val=0.2}, 0.0, 1.0, 0.01},

    {"focus_point_x", "Focus X", "Focus Point X", PARAM_FLOAT, {.f_val=0.5}, -0.5, 1.5, 0.01},
    {"focus_point_y", "Focus Y", "Focus Point Y", PARAM_FLOAT, {.f_val=0.5}, -0.5, 1.5, 0.01},
    {"focus_strength", "Focus Strength", "Blur falloff from focus", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1},
    {"motion_blur_amount", "Motion Blur", "Trail/Motion blur amount", PARAM_FLOAT, {.f_val=0.1}, 0.0, 0.95, 0.01},
    {"bokeh_edge_softness", "Softness", "Bokeh shape edge softness", PARAM_FLOAT, {.f_val=0.5}, 0.0, 1.0, 0.01},

    {"use_polygons", "Use Polygons", "Use polygonal shapes instead of circles", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"poly_sides", "Polygon Sides", "Number of sides for polygon", PARAM_INT, {.i_val=6}, 3, 10, 1},
    {"poly_rotation", "Polygon Rotation", "Static rotation of polygons", PARAM_FLOAT, {.f_val=0.0}, 0.0, 360.0, 1.0},
    {"poly_rotation_speed", "Rotation Speed", "Speed of polygon rotation", PARAM_FLOAT, {.f_val=0.0}, -360.0, 360.0, 1.0},

    {"enable_chromatic_aberration", "Chromatic Aberration", "Enable CA", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"ca_strength", "CA Strength", "Chromatic Aberration Amount", PARAM_FLOAT, {.f_val=2.0}, 0.0, 10.0, 0.1},

    {"enable_onion_rings", "Onion Rings", "Enable Onion Ring artifact", PARAM_BOOL, {.b_val=false}, 0, 0, 0},
    {"onion_ring_frequency", "Ring Frequency", "Frequency of onion rings", PARAM_FLOAT, {.f_val=5.0}, 1.0, 25.0, 0.5},
    {"onion_ring_strength", "Ring Strength", "Strength of onion rings", PARAM_FLOAT, {.f_val=0.4}, 0.0, 1.0, 0.01},
    {"onion_ring_animation_speed", "Ring Speed", "Animation speed of rings", PARAM_FLOAT, {.f_val=0.0}, -5.0, 5.0, 0.1}
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
