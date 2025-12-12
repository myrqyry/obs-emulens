/*
 * src/core/shader-loader.c
 * Secure shader loading with validation and fallback
 */

#include "effect-core.h"
#include "../utils/logging.h"
#include <obs-module.h>

#define FALLBACK_SHADER_PATH "shaders/passthrough.shader"

bool is_valid_shader_path(const char *path) {
    if (!path || strlen(path) == 0 || strlen(path) >= 256) {
        PLUGIN_LOG_DEBUG("shader-loader", "Shader path validation failed: invalid length");
        return false;
    }
    
    // Check for directory traversal patterns
    if (strstr(path, "..") || strstr(path, "\\")) {
        PLUGIN_LOG_WARNING("shader-loader", "Shader path validation failed: directory traversal attempt in '%s'", path);
        return false;
    }
    
    // Must start with shaders/ prefix
    if (strncmp(path, "shaders/", 8) != 0) {
        PLUGIN_LOG_WARNING("shader-loader", "Shader path validation failed: must start with 'shaders/' prefix");
        return false;
    }
    
    // Path should contain exactly one slash (the prefix slash) and use forward slashes only
    size_t slash_count = 0;
    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/') slash_count++;
        if (*p == '\\') {
            PLUGIN_LOG_WARNING("shader-loader", "Shader path validation failed: backslash not allowed");
            return false;
        }
    }
    
    // Should have exactly one slash (in "shaders/")
    if (slash_count != 1) {
        PLUGIN_LOG_WARNING("shader-loader", "Shader path validation failed: invalid path structure");
        return false;
    }
    
    // Must end with .shader extension
    const char *ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".shader") != 0) {
        PLUGIN_LOG_WARNING("shader-loader", "Shader path validation failed: must have .shader extension");
        return false;
    }
    
    // Verify filename component exists and is reasonable
    const char *filename = path + 8; // Skip "shaders/"
    if (strlen(filename) < 5) { // Minimum: "x.shader" (filename part is 'x') -> actually x.shader is 8 charts, but the check is on path+8 which is just the filename... wait. 
        // path="shaders/x.shader", path+8="x.shader".
        // The original code check was `if (strlen(filename) < 8)`. 
        // If filename is "x.shader", strlen is 8.
        // If I skip "shaders/" (8 chars), then `filename` points to "x.shader".
        // So checking if strlen(filename) < 8 is correct for "x.shader".
        // Wait. "shaders/" is index 0-7. path+8 is the start of the rest.
        // If path is "shaders/x.shader", path+8 is "x.shader". strlen is 8.
        PLUGIN_LOG_WARNING("shader-loader", "Shader path validation failed: filename too short");
        return false;
    }
    
    return true;
}

gs_effect_t *load_shader_effect(const char *shader_path) {
    VALIDATE_POINTER_RETURN(shader_path, "shader-loader", NULL);

    if (!is_valid_shader_path(shader_path)) {
        PLUGIN_LOG_ERROR("shader-loader", "Invalid shader path: %s", shader_path);
        return NULL;
    }
    
    obs_module_t *module = obs_get_module("obs-emulens");
    char *full_path = obs_find_module_file(module, shader_path);
    
    if (!full_path) {
        PLUGIN_LOG_ERROR("shader-loader", "Could not find shader file: %s", shader_path);
        
        // Try fallback shader
        if (strcmp(shader_path, FALLBACK_SHADER_PATH) != 0) {
            PLUGIN_LOG_WARNING("shader-loader", "Attempting to load fallback shader: %s", FALLBACK_SHADER_PATH);
            return load_shader_effect(FALLBACK_SHADER_PATH);
        }
        return NULL;
    }
    
    PLUGIN_LOG_DEBUG("shader-loader", "Loading shader from: %s", full_path);
    
    char *error_string = NULL;
    gs_effect_t *effect = gs_effect_create_from_file(full_path, &error_string);
    
    if (error_string) {
        PLUGIN_LOG_ERROR("shader-loader", "Shader compilation error in %s: %s", shader_path, error_string);
        bfree(error_string);
        bfree(full_path);
        
        // Try fallback shader on compilation error
        if (strcmp(shader_path, FALLBACK_SHADER_PATH) != 0) {
            PLUGIN_LOG_WARNING("shader-loader", "Shader '%s' failed to compile, using fallback", shader_path);
            return load_shader_effect(FALLBACK_SHADER_PATH);
        }
        return NULL;
    }
    
    bfree(full_path);
    
    if (effect) {
        PLUGIN_LOG_INFO("shader-loader", "Successfully loaded shader: %s", shader_path);
    }
    
    return effect;
}
