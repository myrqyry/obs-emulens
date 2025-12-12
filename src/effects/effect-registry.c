/*
 * src/effects/effect-registry.c
 */

#include "effect-registry.h"
#include "starburst/starburst.h"
#include "lightleak/lightleak.h"
#include "handheld/handheld.h"
#include "bokeh/bokeh.h"
#include "style_transfer/style_transfer.h"

const effect_info_t *effects[] = {
    &star_burst_info, 
    &light_leak_info, 
    &handheld_info, 
    &bokeh_info, 
    &style_transfer_info
};

const size_t num_effects = sizeof(effects) / sizeof(effects[0]);

const char *get_effect_name(void *type_data) {
    const effect_info_t *info = type_data;
    return info ? info->name : "Unknown Effect";
}
