// --- Parameters ---
uniform int preset <
    string label = "Preset";
    string group = "Presets";
    string widget_type = "select";
    int option_0_value = 0; string option_0_label = "1: Stable (Minimal)";
    int option_1_value = 1; string option_1_label = "2: Breathing";
    int option_2_value = 2; string option_2_label = "3: Handheld";
    int option_3_value = 3; string option_3_label = "4: Shaky";
    int option_4_value = 4; string option_4_label = "5: Earthquake";
    int option_5_value = 99; string option_5_label = "Custom";
> = 2;

uniform float masterIntensity <
    string label = "Master Intensity";
    string group = "Custom Settings";
    string description = "Overall strength multiplier for custom settings.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 2.0; float step = 0.05;
> = 1.0;

uniform float positionAmount <
    string label = "Position Shake Amount";
    string group = "Custom Settings";
    string description = "Max screen offset (0.01 = 1% of screen dimension)";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 0.1; float step = 0.001;
> = 0.005;

uniform float rotationAmount <
    string label = "Rotation Amount (Degrees)";
    string group = "Custom Settings";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 10.0; float step = 0.1;
> = 0.5;

uniform float zoomAmount <
    string label = "Zoom Fluctuation Amount";
    string group = "Custom Settings";
    string description = "Amount to zoom in/out (0=none, 0.1= +/-10% zoom)";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 0.2; float step = 0.002;
> = 0.01;

uniform float positionSpeed <
    string label = "Position Shake Speed";
    string group = "Custom Settings";
    string widget_type = "slider";
    float minimum = 0.1; float maximum = 10.0; float step = 0.1;
> = 1.5;

uniform float rotationSpeed <
    string label = "Rotation Speed";
    string group = "Custom Settings";
    string widget_type = "slider";
    float minimum = 0.1; float maximum = 10.0; float step = 0.1;
> = 1.0;

uniform float zoomSpeed <
    string label = "Zoom Fluctuation Speed";
    string group = "Custom Settings";
    string widget_type = "slider";
    float minimum = 0.1; float maximum = 10.0; float step = 0.1;
> = 0.8;

// --- NEW Parameters for Simulated Focus/Blur ---
uniform bool enableDynamicBlur <
    string label = "Enable Dynamic Blur (Focus Effect)";
    string group = "Focus & Blur";
> = true;

uniform float blurAmount <
    string label = "Max Blur Amount";
    string group = "Focus & Blur";
    string description = "Maximum blur radius (larger = more blur, more performance cost).";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 5.0; float step = 0.1; // Max pixel offset for blur
> = 1.0;

uniform float blurSpeed <
    string label = "Blur Fluctuation Speed";
    string group = "Focus & Blur";
    string widget_type = "slider";
    float minimum = 0.1; float maximum = 10.0; float step = 0.1;
> = 1.5;

// We can also add a base static blur if desired, or tie blur intensity to presets
uniform float staticBlurAmount <
    string label = "Static Base Blur";
    string group = "Focus & Blur";
    string description = "A constant blur amount, added to dynamic blur.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 3.0; float step = 0.05;
> = 0.0;

uniform float edgeFeatherAmount < // NEW
    string label = "Edge Feather Amount";
    string group = "Edge Handling";
    string description = "Softens source edges to hide hard lines during movement (0=none, 0.1=10% feather from each edge).";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 0.25; float step = 0.005;
> = 0.05;

// --- Helper Functions ---
float rand(float2 co){
    return frac(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453);
}

float noise(float2 p) {
    float2 i = floor(p);
    float2 f = frac(p);
    f = f*f*(3.0-2.0*f);
    float res = lerp(lerp(rand(i + float2(0.0,0.0)), rand(i + float2(1.0,0.0)),f.x),
                     lerp(rand(i + float2(0.0,1.0)), rand(i + float2(1.0,1.0)),f.x),f.y);
    return res;
}

// --- Pixel Shader ---
float4 mainImage(VertData v_in) : TARGET
{
    float2 original_texcoord = v_in.uv;
    float2 center = float2(0.5, 0.5);

    // --- Determine settings based on preset ---
    float current_pos_amount;
    float current_rot_amount_deg;
    float current_zoom_amount;
    float current_pos_speed;
    float current_rot_speed;
    float current_zoom_speed;
    // Preset-specific blur settings
    float preset_blur_amount_factor = 1.0; // Multiplier for blurAmount
    float preset_blur_speed_factor = 1.0;  // Multiplier for blurSpeed

    if (preset == 99) { // Custom
        current_pos_amount = positionAmount * masterIntensity;
        current_rot_amount_deg = rotationAmount * masterIntensity;
        current_zoom_amount = zoomAmount * masterIntensity;
        current_pos_speed = positionSpeed;
        current_rot_speed = rotationSpeed;
        current_zoom_speed = zoomSpeed;
        // For custom, blurAmount and blurSpeed are used directly
    } else {
        // Define preset values explicitly
        if (preset == 0) { // Stable
            current_pos_amount = 0.0005; current_rot_amount_deg = 0.05; current_zoom_amount = 0.000;
            current_pos_speed = 0.2; current_rot_speed = 0.15; current_zoom_speed = 0.1;
            preset_blur_amount_factor = 0.1; preset_blur_speed_factor = 0.5; // Less blur for stable
        } else if (preset == 1) { // Breathing
            current_pos_amount = 0.0015; current_rot_amount_deg = 0.15; current_zoom_amount = 0.002;
            current_pos_speed = 0.4; current_rot_speed = 0.3; current_zoom_speed = 0.25;
            preset_blur_amount_factor = 0.3; preset_blur_speed_factor = 0.8;
        } else if (preset == 2) { // Handheld (Default)
            current_pos_amount = 0.005; current_rot_amount_deg = 0.5; current_zoom_amount = 0.01;
            current_pos_speed = 1.5; current_rot_speed = 1.0; current_zoom_speed = 0.8;
            preset_blur_amount_factor = 1.0; preset_blur_speed_factor = 1.0;
        } else if (preset == 3) { // Shaky
            current_pos_amount = 0.015; current_rot_amount_deg = 1.5; current_zoom_amount = 0.02;
            current_pos_speed = 5.0; current_rot_speed = 4.0; current_zoom_speed = 3.0;
            preset_blur_amount_factor = 1.2; preset_blur_speed_factor = 1.2; // More blur for shaky
        } else if (preset == 4) { // Earthquake
            current_pos_amount = 0.05; current_rot_amount_deg = 5.0; current_zoom_amount = 0.05;
            current_pos_speed = 10.0; current_rot_speed = 8.0; current_zoom_speed = 6.0;
            preset_blur_amount_factor = 1.5; preset_blur_speed_factor = 1.5; // Even more blur
        } else { // Fallback
            current_pos_amount = 0.005; current_rot_amount_deg = 0.5; current_zoom_amount = 0.01;
            current_pos_speed = 1.5; current_rot_speed = 1.0; current_zoom_speed = 0.8;
            preset_blur_amount_factor = 1.0; preset_blur_speed_factor = 1.0;
        }
    }
    float current_rot_amount_rad = radians(current_rot_amount_deg);

    // Calculate dynamic transformations (Position, Rotation, Zoom - same as before)
    // ... (copy paste the existing logic for pos_offset, rot_angle_rad, zoom_factor)
    float time_pos1 = elapsed_time * current_pos_speed * 0.7;
    float time_pos2 = elapsed_time * current_pos_speed * 1.5;
    float offset_x_val = (noise(float2(time_pos1, 5.3)) - 0.5) * 2.0;
    offset_x_val += (noise(float2(time_pos2 + 10.0, 2.1)) - 0.5) * 1.0;
    float offset_y_val = (noise(float2(8.1, time_pos1 + 3.7)) - 0.5) * 2.0;
    offset_y_val += (noise(float2(1.9, time_pos2 + 15.0)) - 0.5) * 1.0;
    float2 pos_offset = float2(offset_x_val * 0.333, offset_y_val * 0.333) * current_pos_amount;

    float time_rot1 = elapsed_time * current_rot_speed * 0.6;
    float time_rot2 = elapsed_time * current_rot_speed * 2.1;
    float rot_val = (noise(float2(time_rot1 + 22.5, 9.8)) - 0.5) * 2.0;
    rot_val += (noise(float2(time_rot2 + 30.0, time_rot2)) - 0.5) * 1.0;
    float rot_angle_rad = (rot_val * 0.333) * current_rot_amount_rad;

    float time_zoom1 = elapsed_time * current_zoom_speed * 0.5;
    float time_zoom2 = elapsed_time * current_zoom_speed * 1.8;
    float zoom_val = (noise(float2(17.2, time_zoom1 + 40.0)) - 0.5) * 2.0;
    zoom_val += (noise(float2(time_zoom2 + 55.0, 11.6)) - 0.5) * 1.0;
    float zoom_factor = 1.0 + (zoom_val * 0.333) * current_zoom_amount;


    // --- Apply transformations to UV coordinates ---
    float2 transformed_uv = original_texcoord;
    transformed_uv -= center;
    transformed_uv /= zoom_factor;
    float s = sin(rot_angle_rad);
    float c = cos(rot_angle_rad);
    mat2 rotationMatrix = mat2(c, -s, s, c);
    transformed_uv = mul(rotationMatrix, transformed_uv);
    transformed_uv += center;
    transformed_uv += pos_offset;

    // --- Sample the image with transformed UVs ---
    float4 color_from_source = image.Sample(textureSampler, transformed_uv);

    // --- Apply Dynamic Blur ---
    if (enableDynamicBlur && (blurAmount > 0.001 || staticBlurAmount > 0.001)) {
        float dynamic_blur_intensity = (noise(float2(elapsed_time * blurSpeed * preset_blur_speed_factor, 77.7)) * 2.0 - 1.0); // -1 to 1
        // Make it pulse more:
        dynamic_blur_intensity = sin(elapsed_time * blurSpeed * preset_blur_speed_factor * 0.5) * 0.5 + 0.5; // 0 to 1 sine wave for pulsing
        
        float current_blur_radius_px = staticBlurAmount + (blurAmount * preset_blur_amount_factor * dynamic_blur_intensity);
        
        if (current_blur_radius_px > 0.01) { // Only blur if radius is significant
            float2 pixel_size = uv_pixel_interval; // From ShaderFilter's standard uniforms
            float4 blurred_color = float4(0.0, 0.0, 0.0, 0.0);
            float total_weight = 0.0;

            // Simple 3x3 Box Blur (9 samples)
            // We use current_blur_radius_px to scale the pixel_size for sampling offsets
            float blur_offset = current_blur_radius_px;

            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    float2 sample_uv = transformed_uv + float2(x, y) * pixel_size * blur_offset;
                    blurred_color += image.Sample(textureSampler, sample_uv);
                    total_weight += 1.0;
                }
            }
            if (total_weight > 0.0) {
                color_from_source = blurred_color / total_weight;
            }
        }
    }

    // --- Edge Feathering based on ORIGINAL texcoord ---
    float edge_feather_multiplier = 1.0;
    if (edgeFeatherAmount > 0.0001) {
        float feather_l = smoothstep(0.0, edgeFeatherAmount, original_texcoord.x);
        float feather_r = smoothstep(0.0, edgeFeatherAmount, 1.0 - original_texcoord.x);
        float feather_t = smoothstep(0.0, edgeFeatherAmount, original_texcoord.y);
        float feather_b = smoothstep(0.0, edgeFeatherAmount, 1.0 - original_texcoord.y);
        edge_feather_multiplier = saturate(feather_l * feather_r * feather_t * feather_b);
    }

    float3 feathered_rgb = color_from_source.rgb * edge_feather_multiplier;
    float feathered_alpha = color_from_source.a * edge_feather_multiplier;
    
    float4 finalColor = float4(feathered_rgb, feathered_alpha);

    return finalColor;
}
