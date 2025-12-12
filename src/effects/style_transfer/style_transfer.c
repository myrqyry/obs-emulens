/*
 * src/effects/style_transfer/style_transfer.c
 */

#include "style_transfer.h"

// Style Transfer has no parameters in the original code
static void style_transfer_defaults(obs_data_t *s) { 
    (void)s; 
}

const effect_info_t style_transfer_info = {
    "style_transfer_effect", "Style Transfer", "Applies artistic style transfer", "shaders/style-transfer.shader",
    NULL, 0,
    generic_create, generic_destroy, generic_update, generic_render, generic_tick, generic_properties, style_transfer_defaults
};
