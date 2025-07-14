// --- Constants ---
#define PI 3.14159265359

// --- User-defined parameters (uniforms) ---
// These are implicitly available global uniforms provided by ShaderFilter's wrapper:
// uniform texture2d image;
// uniform sampler_state textureSampler;
// uniform float elapsed_time;
// uniform float2 uv_size; // size of the texture in pixels
// uniform float2 uv_pixel_interval; // (1.0/uv_size.x, 1.0/uv_size.y)
// uniform float rand_f; // a random float between 0.0 and 1.0, changes per frame
//
// VertData is implicitly defined as:
// struct VertData {
//     float4 pos : POSITION; // Clip space position
//     float2 uv : TEXCOORD0;  // Texture coordinates
// };

uniform float particle_density <
    string label = "Particle Density";
    string widget_type = "slider";
    float minimum = 1.0;
    float maximum = 100.0;
    float step = 1.0;
> = 25.0;

uniform float particle_base_size <
    string label = "Base Particle Size";
    string widget_type = "slider";
    float minimum = 0.001;
    float maximum = 0.2; 
    float step = 0.001;
> = 0.05;

uniform float particle_size_variation <
    string label = "Size Variation";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 1.0;
    float step = 0.01;
> = 0.5;

uniform float animation_speed <
    string label = "Animation Speed";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 5.0;
    float step = 0.01;
> = 0.3;

uniform float4 particle_color_start <
    string label = "Particle Color (Start of Life)";
    string widget_type = "color";
> = {0.8, 0.8, 1.0, 0.8};

uniform float4 particle_color_end <
    string label = "Particle Color (End of Life)";
    string widget_type = "color";
> = {0.2, 0.2, 0.5, 0.0};

uniform bool enable_source_brightness_affect <
    string label = "Enable Source Brightness Affect";
    string description = "Particle visibility is affected by underlying source brightness.";
    string widget_type = "checkbox";
    string group = "Source Interaction";
> = false;

uniform float source_brightness_strength <
    string label = "Source Brightness Strength";
    string description = "0: no effect. 1: particle alpha strongly follows source brightness.";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 1.0; // For this logic, 0-1 is a good range.
    float step = 0.01;
    string group = "Source Interaction";
> = 0.75;

uniform float source_brightness_threshold <
    string label = "Source Brightness Threshold";
    string description = "Source brightness below this reduces particle visibility.";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 1.0;
    float step = 0.01;
    string group = "Source Interaction";
> = 0.2;

uniform float focus_point_x <
    string label = "Focus Point X";
    string widget_type = "slider";
    float minimum = -0.5;
    float maximum = 1.5;
    float step = 0.01;
    string group = "Blur/Focus Settings";
> = 0.5;

uniform float focus_point_y <
    string label = "Focus Point Y";
    string widget_type = "slider";
    float minimum = -0.5;
    float maximum = 1.5;
    float step = 0.01;
    string group = "Blur/Focus Settings";
> = 0.5;

uniform float focus_strength <
    string label = "Focus Strength";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 10.0;
    float step = 0.1;
    string group = "Blur/Focus Settings";
> = 2.0;

uniform float motion_blur_amount <
    string label = "Motion Blur Amount (subtle)";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 0.95;
    float step = 0.01;
    string group = "Blur/Focus Settings";
> = 0.1;

uniform float bokeh_edge_softness <
    string label = "Bokeh Edge Softness";
    string widget_type = "slider";
    float minimum = 0.0; 
    float maximum = 1.0; 
    string group = "Blur/Focus Settings";
    float step = 0.01;
> = 0.5; 

// --- Shape Customization Uniforms ---
uniform bool use_polygons <
    string label = "Use Polygonal Bokeh";
    string widget_type = "checkbox";
    string group = "Polygon Settings"; 
> = false; 

uniform int poly_sides <
    string label = "Bokeh Sides";
    string widget_type = "slider";
    int minimum = 3; 
    int maximum = 10; 
    int step = 1;
    string group = "Polygon Settings"; 
> = 6; 

uniform float poly_rotation <
    string label = "Bokeh Rotation (Degrees)";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 360.0;
    float step = 1.0;
    string group = "Polygon Settings";
> = 0.0;

uniform float poly_rotation_speed <
    string label = "Bokeh Rotation Speed (Deg/sec)";
    string widget_type = "slider";
    float minimum = -360.0; // Degrees per second, negative for counter-clockwise
    float maximum = 360.0;
    float step = 1.0;
    string group = "Polygon Settings";
> = 0.0; // Default to no animation

uniform bool enable_chromatic_aberration <
    string label = "Enable Chromatic Aberration";
    string widget_type = "checkbox";
    string group = "Artifact Settings";
> = false; // Default to off

uniform float ca_strength <
    string label = "CA Strength";
    string description = "Amount of color separation. Higher values = more fringe.";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 10.0; // Roughly in pixels of separation at edges
    float step = 0.1;
    string group = "Artifact Settings";
> = 2.0; // A subtle default

uniform bool enable_onion_rings <
    string label = "Enable Onion Rings";
    string widget_type = "checkbox";
    string group = "Artifact Settings"; // You can create a new group for these artifacts
> = false; // Default to off

uniform float onion_ring_frequency <
    string label = "Onion Ring Frequency";
    string description = "How many rings appear from center to edge.";
    string widget_type = "slider";
    float minimum = 1.0;
    float maximum = 25.0; 
    float step = 0.5;
    string group = "Artifact Settings";
> = 5.0;

uniform float onion_ring_strength <
    string label = "Onion Ring Strength";
    string description = "How much the rings affect the brightness/alpha. 0=none, 1=strong.";
    string widget_type = "slider";
    float minimum = 0.0; 
    float maximum = 1.0; 
    float step = 0.01;
    string group = "Artifact Settings";
> = 0.4; // Default to a noticeable but not overpowering effect

uniform float onion_ring_animation_speed <
    string label = "Onion Ring Animation Speed";
    string description = "Speed of rings moving inward/outward. 0=static.";
    string widget_type = "slider";
    float minimum = -5.0; 
    float maximum = 5.0;  
    float step = 0.1;
    string group = "Artifact Settings";
> = 0.0;


// --- Helper functions ---

float2 rand2(float2 p) {
    return frac(sin(float2(dot(p, float2(127.1, 311.7)), dot(p, float2(269.5, 183.3)))) * 43758.5453);
}

// Original circle function (still used by get_shape_mask as a fallback or if use_polygons is false)
float circle(float2 uv, float2 pos, float radius, float softness) {
    // Ensure softness doesn't make radius completely disappear or become negative
    softness = min(softness, radius * 0.999); 
    softness = max(0.0, softness); // Ensure non-negative softness
    float d = length(uv - pos);
    return smoothstep(radius, radius - softness, d);
}

// Helper to rotate a 2D vector
float2 rotate(float2 v, float angle_rad) {
    float s = sin(angle_rad);
    float c = cos(angle_rad);
    return float2(v.x * c - v.y * s, v.x * s + v.y * c);
}

// Calculates a value representing distance from polygon edge (negative inside)
float nGonDist(float2 p_local, float N_float, float r_circum) {
    if (N_float < 2.5) return length(p_local) - r_circum; // Fallback for safety

    float ang = atan2(p_local.x, p_local.y); 
    float segment_angle = 2.0 * PI / N_float; 
    float t = floor(0.5 + ang / segment_angle) * segment_angle;
    float dist_to_segment_bisector_projection = cos(t - ang) * length(p_local);
    float apothem = r_circum * cos(PI / N_float);
    return dist_to_segment_bisector_projection - apothem; 
}

// Unified function to generate the shape mask (circle or polygon)
// Includes onion ring effect if enabled.
float get_shape_mask(float2 uv_pixel_centered_aspect, float2 particle_center_aspect, 
                     bool use_polygons_flag, int N_sides_int, float rotation_rad, 
                     float particle_radius_circum, float desired_edge_blur_width)
{
    // Calculate p_local_unrotated: coordinates of the current pixel relative to the particle center,
    // without the polygon's specific orientation rotation. This is used for distance from center (for rings).
    float2 p_local_unrotated = uv_pixel_centered_aspect - particle_center_aspect;
    float dist_from_center_for_rings = length(p_local_unrotated);

    float base_shape_alpha; // This will be the mask from the primary shape (circle/polygon)

    if (!use_polygons_flag || N_sides_int < 3) { 
        // --- Circle Drawing Logic ---
        float actual_circle_softness = min(desired_edge_blur_width, particle_radius_circum * 0.999);
        actual_circle_softness = max(0.0, actual_circle_softness); 

        // Use dist_from_center_for_rings (which is 'd' in the original circle function)
        base_shape_alpha = smoothstep(particle_radius_circum, particle_radius_circum - actual_circle_softness, dist_from_center_for_rings);
    } else {
        // --- Polygon Drawing Logic (N_sides >= 3) ---
        // For the polygon SDF, we need p_local rotated by the particle's orientation
        float2 p_local_rotated_for_sdf = rotate(p_local_unrotated, -rotation_rad); 

        float N_float = float(N_sides_int); 
        float dist_from_sdf_edge = nGonDist(p_local_rotated_for_sdf, N_float, particle_radius_circum); // dist is negative inside

        float half_actual_blur_width = max(desired_edge_blur_width, uv_pixel_interval.y) * 0.5; 
        base_shape_alpha = smoothstep(half_actual_blur_width, -half_actual_blur_width, dist_from_sdf_edge);
    }

    // --- Apply Onion Ring Effect (modulates base_shape_alpha) ---
    if (enable_onion_rings && base_shape_alpha > 0.001) { // Only apply if pixel is significantly part of the bokeh
        // Normalized distance from center (0 at center, ~1 at edge of particle_radius_circum)
        float normalized_dist = saturate(dist_from_center_for_rings / particle_radius_circum);
        
        float ring_phase_offset = elapsed_time * onion_ring_animation_speed;

        // Sinusoidal wave based on normalized distance and frequency, animated by phase
        // (Multiplying by 2.0*PI makes one full cycle of sin per unit of frequency)
        float ring_sine_wave = sin(normalized_dist * onion_ring_frequency * 2.0 * PI - ring_phase_offset);
        
        // Remap sine wave from [-1, 1] to [0, 1] range
        float ring_modulation_factor = (ring_sine_wave * 0.5 + 0.5); 
        
        // Apply strength: lerp from 1.0 (no effect) towards the ring_modulation_factor
        ring_modulation_factor = lerp(1.0, ring_modulation_factor, onion_ring_strength);
        
        base_shape_alpha *= ring_modulation_factor; // Modulate the existing alpha
    }
    
    return saturate(base_shape_alpha); // Ensure final mask is clamped
}


// --- Pixel Shader ---
float4 mainImage(VertData v_in) : TARGET
{
    float2 texcoord = v_in.uv; 
    float4 original_color = image.Sample(textureSampler, texcoord); 
    
    float4 final_particle_color = float4(0.0, 0.0, 0.0, 0.0); 

    float aspect_ratio = uv_size.x / uv_size.y;
    float2 centered_aspect_uv = float2((texcoord.x - 0.5) * aspect_ratio, texcoord.y - 0.5);

    float time = elapsed_time * animation_speed;

    float cells_raw = particle_density / 5.0;
    float cells = clamp(cells_raw, 3.0, 10.0); 

    float2 cell_uv = texcoord * cells; 
    float2 cell_id = floor(cell_uv); 

    for (int iy = -2; iy <= 2; ++iy) { 
        for (int ix = -2; ix <= 2; ++ix) {
            float2 neighbor_cell_id = cell_id + float2(ix, iy);
            
            float2 particle_rand_seed = neighbor_cell_id; 
            float2 particle_rand = rand2(particle_rand_seed);

            float particle_life = particle_rand.x * 5.0 + 2.0; 
            float particle_phase = frac(time / particle_life + particle_rand.y); 

            float2 particle_pos_in_cell = rand2(particle_rand_seed + particle_rand.y); 
            particle_pos_in_cell.y += particle_phase * 2.0 - 1.0; 
            particle_pos_in_cell.x += sin(particle_phase * PI * 2.0 + particle_rand.x * PI) * 0.5; 
            
            float2 particle_center_uv_raw = (neighbor_cell_id + particle_pos_in_cell) / cells;

            float2 focus_uv_raw = float2(focus_point_x, focus_point_y);
            float dist_to_focus = length(particle_center_uv_raw - focus_uv_raw);
            float focus_factor = 1.0 - saturate(dist_to_focus * focus_strength); 

            float current_particle_size = particle_base_size * (1.0 - particle_size_variation * particle_rand.x);
            current_particle_size *= (1.0 + focus_factor);
            current_particle_size = max(0.0001, current_particle_size);

            float life_alpha = sin(particle_phase * PI); 
            float4 current_particle_color_sample = lerp(particle_color_start, particle_color_end, particle_phase);
            current_particle_color_sample.a *= life_alpha;

// --- Source Brightness Interaction ---
            if (enable_source_brightness_affect) {
                // Sample the original source color at the particle's conceptual center.
                // particle_center_uv_raw is in [0,1] UV space.
                float2 source_sample_uv = saturate(particle_center_uv_raw); // Clamp to avoid edge issues
                float4 source_color_at_particle = image.Sample(textureSampler, source_sample_uv);
                
                // Calculate luminance (brightness) of the sampled source color
                float source_luminance = dot(source_color_at_particle.rgb, float3(0.299, 0.587, 0.114));
                
                // Determine modulation factor based on luminance, threshold, and strength
                float luma_modulation_factor = 1.0; // Default: no change to alpha

                if (source_luminance < source_brightness_threshold) {
                    // If source is darker than threshold, the modulation aims towards 0 (transparent)
                    // How quickly it goes to 0 depends on how far below threshold it is.
                    // For simplicity, let's make it a sharper falloff if below threshold:
                    luma_modulation_factor = source_luminance / source_brightness_threshold; // Scales 0-1 below threshold
                    luma_modulation_factor = saturate(luma_modulation_factor); // Ensure it's 0-1
                } else {
                    // If source is brighter than or at threshold, scale factor from a base (e.g., 0 if at threshold) to 1 (at max brightness)
                    // This makes particles "pop" more over bright areas if not already at full alpha.
                    // For this implementation, let's make sure particles are fully visible if source is bright enough.
                    // (The previous logic was scaling between 0 and 1, let's simplify)
                    luma_modulation_factor = 1.0; // Fully visible if above threshold (can be refined later if needed)
                }
                
                // The 'source_brightness_strength' (0 to 1) blends between original alpha and luma-modulated alpha.
                // If strength = 0, final_factor = 1 (no change from original life_alpha).
                // If strength = 1, final_factor = luma_modulation_factor.
                float final_luma_effect_on_alpha = lerp(1.0, luma_modulation_factor, source_brightness_strength); 
                current_particle_color_sample.a *= final_luma_effect_on_alpha;
            }
            // --- End of Source Brightness Interaction ---

            float2 particle_pos_aspect = float2((particle_center_uv_raw.x - 0.5) * aspect_ratio, particle_center_uv_raw.y - 0.5);
            
// Calculate the desired width of the blurred edge based on the new uniform
            float calculated_blur_width = current_particle_size * bokeh_edge_softness;

            // Calculate animated rotation (used by get_shape_mask)
            float static_rotation_rad = poly_rotation * (PI / 180.0);
            float animated_rotation_rad = elapsed_time * poly_rotation_speed * (PI / 180.0);
            float final_rotation_radians = static_rotation_rad + animated_rotation_rad;

            float4 particle_contribution_this_iteration = float4(0.0, 0.0, 0.0, 0.0);

            if (enable_chromatic_aberration && ca_strength > 0.01) {
                // Calculate radial offset direction (from particle center to current pixel)
                // This makes the aberration spread out from the particle's center.
                float2 vector_to_pixel = centered_aspect_uv - particle_pos_aspect;
                float dist_to_pixel_from_center = length(vector_to_pixel);
                
                // Avoid division by zero if pixel is exactly at particle center
                float2 radial_offset_dir = (dist_to_pixel_from_center > 0.0001) ? normalize(vector_to_pixel) : float2(0.0, 0.0);
                
                // Calculate actual offset distance in UV space.
                // ca_strength acts as a multiplier, uv_pixel_interval.y gives a "pixel size" reference.
                float2 offset_uv_amount = radial_offset_dir * ca_strength * uv_pixel_interval.y;

                // Get mask for Red channel: evaluate shape at UV shifted by +offset_uv_amount
                float mask_r = get_shape_mask(centered_aspect_uv + offset_uv_amount, particle_pos_aspect, 
                                              use_polygons, poly_sides, final_rotation_radians, 
                                              current_particle_size, calculated_blur_width);
                
                // Get mask for Green channel: evaluate shape at normal UV (no offset)
                float mask_g = get_shape_mask(centered_aspect_uv, particle_pos_aspect, 
                                              use_polygons, poly_sides, final_rotation_radians, 
                                              current_particle_size, calculated_blur_width);
                
                // Get mask for Blue channel: evaluate shape at UV shifted by -offset_uv_amount
                float mask_b = get_shape_mask(centered_aspect_uv - offset_uv_amount, particle_pos_aspect, 
                                              use_polygons, poly_sides, final_rotation_radians, 
                                              current_particle_size, calculated_blur_width);

                particle_contribution_this_iteration.r = current_particle_color_sample.r * mask_r;
                particle_contribution_this_iteration.g = current_particle_color_sample.g * mask_g;
                particle_contribution_this_iteration.b = current_particle_color_sample.b * mask_b;
                // Alpha is usually best taken from the non-offsetted (green) channel's mask
                // to maintain the particle's overall intended opacity and shape.
                particle_contribution_this_iteration.a = current_particle_color_sample.a * mask_g;

            } else { // No chromatic aberration, or strength is effectively zero
                float particle_mask_no_ca = get_shape_mask(centered_aspect_uv, particle_pos_aspect, 
                                                           use_polygons, poly_sides, final_rotation_radians, 
                                                           current_particle_size, calculated_blur_width);
                particle_contribution_this_iteration = particle_mask_no_ca * current_particle_color_sample;
            }
            
            final_particle_color += particle_contribution_this_iteration;
        }
    }
    
    float4 blended_output_color;
    blended_output_color.rgb = final_particle_color.rgb + original_color.rgb * (1.0 - final_particle_color.a);
    blended_output_color.a = saturate(final_particle_color.a + original_color.a * (1.0 - final_particle_color.a));

    if (motion_blur_amount > 0.0 && motion_blur_amount < 1.0) {
         blended_output_color = lerp(blended_output_color, original_color, motion_blur_amount);
    }

    return saturate(blended_output_color);
}