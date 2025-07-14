// --- User-defined parameters (uniforms) ---

uniform float4 leakColor <
    string label = "Leak Color (Primary / Start)";
    string widget_type = "color";
> = {1.0, 0.5, 0.2, 0.3};

uniform float leakIntensity <
    string label = "Overall Leak Intensity";
    string description = "Master multiplier for the leak effect's visibility.";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 3.0;
    float step = 0.05;
> = 0.8;

uniform float leakSpeed <
    string label = "Leak Animation Speed";
    string description = "Speed of the underlying noise pattern movement.";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 5.0;
    float step = 0.1;
> = 0.5;

uniform float leakScale <
    string label = "Leak Scale";
    string description = "Size of the noise patterns forming the leaks.";
    string widget_type = "slider";
    float minimum = 0.1;
    float maximum = 10.0;
    float step = 0.1;
> = 2.0;

uniform float edgeFalloff <
    string label = "Edge Falloff Power";
    string description = "Controls how sharply leak influence drops from edges. Higher values = leaks hug edges tighter.";
    string widget_type = "slider";
    float minimum = 0.5;
    float maximum = 10.0;
    float step = 0.1;
> = 3.0;

uniform float noiseComplexity <
    string label = "Noise Complexity";
    string widget_type = "slider";
    float minimum = 1.0;
    float maximum = 8.0;
    float step = 1.0;
> = 3.0;

// Edge Biasing
uniform float topBias <
    string label = "Top Edge Bias";
    string group = "Edge Biasing";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 2.0; float step = 0.05;
> = 0.25;

uniform float bottomBias <
    string label = "Bottom Edge Bias";
    string group = "Edge Biasing";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 2.0; float step = 0.05;
> = 0.25;

uniform float leftBias <
    string label = "Left Edge Bias";
    string group = "Edge Biasing";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 2.0; float step = 0.05;
> = 0.25;

uniform float rightBias <
    string label = "Right Edge Bias";
    string group = "Edge Biasing";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 2.0; float step = 0.05;
> = 0.25;

// Leak Shape
uniform float streakiness <
    string label = "Streakiness (X/Y Ratio)";
    string group = "Leak Shape";
    string description = "<1 stretches vertically, >1 stretches horizontally.";
    string widget_type = "slider";
    float minimum = 0.1;
    float maximum = 10.0;
    float step = 0.05;
> = 1.0;

uniform float leakShapeContrast <
    string label = "Leak Shape Contrast";
    string group = "Leak Shape";
    string description = "Higher values make leak shapes sharper and more defined.";
    string widget_type = "slider";
    float minimum = 0.5;
    float maximum = 5.0;
    float step = 0.05;
> = 1.5;

// Dynamic Behavior
uniform bool enablePulsing <
    string label = "Enable Intensity Pulsing";
    string group = "Dynamic Behavior";
    string description = "Makes the leak intensity/alpha throb over time.";
> = true;

uniform float pulseSpeed <
    string label = "Pulse Speed";
    string group = "Dynamic Behavior";
    string widget_type = "slider";
    float minimum = 0.1; float maximum = 5.0; float step = 0.05;
> = 0.5;

uniform float pulseMinAlpha <
    string label = "Pulse Minimum Alpha";
    string group = "Dynamic Behavior";
    string description = "The minimum target alpha for the leaks during pulsing.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 1.0; float step = 0.01;
> = 0.05;

uniform float pulseMaxAlpha <
    string label = "Pulse Maximum Alpha";
    string group = "Dynamic Behavior";
    string description = "The maximum target alpha for the leaks during pulsing.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 1.0; float step = 0.01;
> = 0.3;

uniform bool enableColorShift <
    string label = "Enable Color Shift";
    string group = "Dynamic Behavior";
    string description = "Makes the leak color transition between two colors.";
> = false;

uniform float4 secondLeakColor <
    string label = "Second Leak Color (for shifting)";
    string group = "Dynamic Behavior";
    string widget_type = "color";
> = {1.0, 0.2, 0.1, 0.35};

uniform float colorShiftSpeed <
    string label = "Color Shift Speed";
    string group = "Dynamic Behavior";
    string widget_type = "slider";
    float minimum = 0.05; float maximum = 2.0; float step = 0.05;
> = 0.2;

// Visual Complexity
uniform float hotspotIntensity <
    string label = "Hotspot Intensity";
    string group = "Visual Complexity";
    string description = "Brightness boost for the core of the leaks.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 3.0; float step = 0.05;
> = 0.5;

uniform float hotspotExponent <
    string label = "Hotspot Tightness";
    string group = "Visual Complexity";
    string description = "Higher values make the hotspot smaller and sharper.";
    string widget_type = "slider";
    float minimum = 1.0; float maximum = 10.0; float step = 0.1;
> = 3.0;

uniform float4 hotspotColor <
    string label = "Hotspot Color Tint";
    string group = "Visual Complexity";
    string description = "Optional color tint for the hotspot (additive). Alpha ignored.";
    string widget_type = "color";
> = {0.1, 0.05, 0.0, 0.0};

uniform float grainAmount <
    string label = "Leak Grain Amount";
    string group = "Visual Complexity";
    string description = "Adds fine noise texture within the leak areas.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 0.5; float step = 0.01;
> = 0.05;

uniform float grainScale <
    string label = "Leak Grain Scale";
    string group = "Visual Complexity";
    string description = "Size of the grain texture. Higher is smaller grain.";
    string widget_type = "slider";
    float minimum = 10.0; float maximum = 100.0; float step = 1.0;
> = 50.0;

// --- NEW Parameter for Blending ---
uniform int blendMode <
    string label = "Leak Blend Mode";
    string group = "Blending";
    string widget_type = "select";
    int option_0_value = 0; string option_0_label = "Alpha Blend (Default)";
    int option_1_value = 1; string option_1_label = "Additive";
    int option_2_value = 2; string option_2_label = "Screen";
    int option_3_value = 3; string option_3_label = "Overlay";
    int option_4_value = 4; string option_4_label = "Soft Light";
> = 0; // Default to Alpha Blend


// --- Helper functions ---
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

float fbm(float2 p, int octaves, float persistence) {
    float total = 0.0;
    float frequency = 1.0;
    float amplitude = 1.0;
    float maxValue = 0.0;
    for(int i = 0; i < octaves; i++) {
        total += noise(p * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    if (maxValue == 0.0) return 0.0;
    return total / maxValue;
}

// --- Blending Functions ---
float3 AdditiveBlend(float3 base, float3 blend_rgb, float blend_alpha) {
    return base + blend_rgb * blend_alpha;
}

float3 ScreenBlend(float3 base, float3 blend_rgb, float blend_alpha) {
    float3 screened = 1.0 - (1.0 - base) * (1.0 - blend_rgb);
    return lerp(base, screened, blend_alpha);
}

float OverlayBlendChannel(float b, float l) {
    return (b < 0.5) ? (2.0 * b * l) : (1.0 - 2.0 * (1.0 - b) * (1.0 - l));
}

float3 OverlayBlend(float3 base, float3 blend_rgb, float blend_alpha) {
    float3 blended;
    blended.r = OverlayBlendChannel(base.r, blend_rgb.r);
    blended.g = OverlayBlendChannel(base.g, blend_rgb.g);
    blended.b = OverlayBlendChannel(base.b, blend_rgb.b);
    return lerp(base, blended, blend_alpha);
}

float SoftLightChannel(float b, float l) {
     return (l < 0.5) ? (2.0 * b * l + b * b * (1.0 - 2.0 * l)) : (sqrt(b) * (2.0 * l - 1.0) + 2.0 * b * (1.0 - l));
}

float3 SoftLightBlend(float3 base, float3 blend_rgb, float blend_alpha) {
    float3 blended;
    blended.r = SoftLightChannel(base.r, blend_rgb.r);
    blended.g = SoftLightChannel(base.g, blend_rgb.g);
    blended.b = SoftLightChannel(base.b, blend_rgb.b);
    return lerp(base, blended, blend_alpha);
}

// --- Pixel Shader ---
float4 mainImage(VertData v_in) : TARGET
{
    float2 texcoord = v_in.uv;
    float4 originalColor = image.Sample(textureSampler, texcoord);

    // --- Generate Light Leak Effect ---

    // 1. Calculate biased edge mask
    float falloff_top    = pow(1.0 - texcoord.y, edgeFalloff);
    float falloff_bottom = pow(texcoord.y,       edgeFalloff);
    float falloff_left   = pow(1.0 - texcoord.x, edgeFalloff);
    float falloff_right  = pow(texcoord.x,       edgeFalloff);
    float biased_edge_mask = 0.0;
    biased_edge_mask += falloff_top    * topBias;
    biased_edge_mask += falloff_bottom * bottomBias;
    biased_edge_mask += falloff_left   * leftBias;
    biased_edge_mask += falloff_right  * rightBias;
    biased_edge_mask = saturate(biased_edge_mask);

    // 2. Generate animated & shaped noise
    float2 noise_uv_base = texcoord * leakScale;
    float2 noise_uv = noise_uv_base;
    if (streakiness > 1.01) {
       noise_uv.x *= streakiness;
    } else if (streakiness < 0.99) {
       noise_uv.y /= streakiness;
    }
    noise_uv.x += elapsed_time * leakSpeed * 0.3;
    noise_uv.y -= elapsed_time * leakSpeed * 0.2;
    float noise_val = fbm(noise_uv, int(noiseComplexity), 0.5);
    if (noise_val > 0.0) {
      noise_val = pow(saturate(noise_val), leakShapeContrast);
    } else {
      noise_val = 0.0;
    }

    // 3. Calculate spatial component
    float spatial_leak_component = noise_val * biased_edge_mask;

    // 4. Determine dynamic color and alpha target
    float3 base_leak_rgb = leakColor.rgb;
    float base_leak_alpha = leakColor.a;
    if (enableColorShift) {
        float shiftFactor = (sin(elapsed_time * colorShiftSpeed) + 1.0) * 0.5;
        base_leak_rgb = lerp(leakColor.rgb, secondLeakColor.rgb, shiftFactor);
        base_leak_alpha = lerp(leakColor.a, secondLeakColor.a, shiftFactor);
    }
    float animated_alpha_target;
    if (enablePulsing) {
        float actualPulseMin = min(pulseMinAlpha, pulseMaxAlpha);
        float actualPulseMax = max(pulseMinAlpha, pulseMaxAlpha);
        float pulseFactor = (sin(elapsed_time * pulseSpeed) + 1.0) * 0.5;
        animated_alpha_target = lerp(actualPulseMin, actualPulseMax, pulseFactor);
    } else {
        animated_alpha_target = base_leak_alpha;
    }

    // 5. Apply Visual Complexity
    float3 final_leak_rgb = base_leak_rgb;
    float hotspot_factor = pow(saturate(spatial_leak_component), hotspotExponent) * hotspotIntensity;
    final_leak_rgb += hotspotColor.rgb * hotspot_factor;
    float2 grain_uv = texcoord * grainScale + float2(rand(texcoord + elapsed_time * 0.1), rand(texcoord - elapsed_time * 0.1));
    float grain_noise = rand(grain_uv) * 2.0 - 1.0;
    float grain_modulator = saturate(spatial_leak_component + hotspot_factor * 0.5);
    final_leak_rgb += grain_noise * grainAmount * grain_modulator;

    // 6. Calculate final alpha and construct color to blend
    float final_leak_alpha = animated_alpha_target * spatial_leak_component * leakIntensity;
    float4 actualLeakToBlend;
    actualLeakToBlend.rgb = final_leak_rgb;
    actualLeakToBlend.a = saturate(final_leak_alpha);

    // --- Combine original color with light leak using selected blend mode ---
    float4 finalColor;

    if (blendMode == 1) { // Additive
        finalColor.rgb = AdditiveBlend(originalColor.rgb, actualLeakToBlend.rgb, actualLeakToBlend.a);
    } else if (blendMode == 2) { // Screen
        finalColor.rgb = ScreenBlend(originalColor.rgb, actualLeakToBlend.rgb, actualLeakToBlend.a);
    } else if (blendMode == 3) { // Overlay
        finalColor.rgb = OverlayBlend(originalColor.rgb, actualLeakToBlend.rgb, actualLeakToBlend.a);
    } else if (blendMode == 4) { // Soft Light
        finalColor.rgb = SoftLightBlend(originalColor.rgb, actualLeakToBlend.rgb, actualLeakToBlend.a);
    } else { // Default: Alpha Blend (Lerp)
        finalColor.rgb = lerp(originalColor.rgb, actualLeakToBlend.rgb, actualLeakToBlend.a);
    }

    finalColor.rgb = saturate(finalColor.rgb); // Clamp result
    finalColor.a = originalColor.a; // Preserve original alpha

    return finalColor;
}
