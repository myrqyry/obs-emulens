#pragma once

#include <obs-module.h>

// Logging macros with context
#define PLUGIN_LOG_ERROR(context, fmt, ...)                                    \
    blog(LOG_ERROR, "[obs-emulens:%s] " fmt, context, ##__VA_ARGS__)

#define PLUGIN_LOG_WARNING(context, fmt, ...)                                  \
    blog(LOG_WARNING, "[obs-emulens:%s] " fmt, context, ##__VA_ARGS__)

#define PLUGIN_LOG_INFO(context, fmt, ...)                                     \
    blog(LOG_INFO, "[obs-emulens:%s] " fmt, context, ##__VA_ARGS__)

#define PLUGIN_LOG_DEBUG(context, fmt, ...)                                    \
    blog(LOG_DEBUG, "[obs-emulens:%s] " fmt, context, ##__VA_ARGS__)

// Effect-specific logging
#define EFFECT_LOG_ERROR(effect, fmt, ...)                                     \
    PLUGIN_LOG_ERROR(effect->info->name, fmt, ##__VA_ARGS__)

#define EFFECT_LOG_WARNING(effect, fmt, ...)                                   \
    PLUGIN_LOG_WARNING(effect->info->name, fmt, ##__VA_ARGS__)

#define EFFECT_LOG_INFO(effect, fmt, ...)                                      \
    PLUGIN_LOG_INFO(effect->info->name, fmt, ##__VA_ARGS__)

#define EFFECT_LOG_DEBUG(effect, fmt, ...)                                     \
    PLUGIN_LOG_DEBUG(effect->info->name, fmt, ##__VA_ARGS__)

// Validation macros with logging
#define VALIDATE_POINTER(ptr, context, action)                                 \
    do {                                                                       \
        if (!(ptr)) {                                                          \
            PLUGIN_LOG_ERROR(context, "Null pointer: " #ptr);                 \
            action;                                                            \
        }                                                                      \
    } while (0)

#define VALIDATE_POINTER_RETURN(ptr, context, retval)                         \
    VALIDATE_POINTER(ptr, context, return retval)

#define VALIDATE_POINTER_RETURN_VOID(ptr, context)                            \
    VALIDATE_POINTER(ptr, context, return)
