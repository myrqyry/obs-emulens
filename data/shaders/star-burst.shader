// --- User-defined parameters (uniforms) ---
uniform string Description<
    string widget_type = "info";
> = "✨ Dramatic Starburst Filter ✨\nCreates striking star-shaped rays from bright light sources. Enhanced!";

// Main parameters
uniform float Threshold<
    string label = "✨ Brightness Threshold";
    string widget_type = "slider";
    float minimum = 0.33;
    float maximum = 2.00;
    float step = 0.01;
> = 0.7;

uniform float Intensity<
    string label = "🔆 Ray Intensity";
    string widget_type = "slider";
    float minimum = 1.0;
    float maximum = 10.0;
    float step = 0.5;
> = 3.0;

uniform int StarPoints<
    string label = "🌟 Star Points";
    string widget_type = "slider";
    int minimum = 4;
    int maximum = 16;
    int step = 2; // Ensure only even numbers for symmetrical stars, or allow odd for different styles
> = 8;

uniform float RayLength<
    string label = "📏 Ray Length";
    string widget_type = "slider";
    float minimum = 0.05;
    float maximum = 0.5;
    float step = 0.05;
> = 0.2;

uniform float RayThickness<
    string label = "🔍 Ray Thickness";
    string widget_type = "slider";
    float minimum = 0.5;
    float maximum = 5.0;
    float step = 0.5;
> = 2.0;

uniform float RaySmoothness<
    string label = "🌊 Ray Smoothness (Length Falloff)";
    string widget_type = "slider";
    float minimum = 1.0;
    float maximum = 10.0;
    float step = 0.5;
> = 3.0;

uniform float Rotation<
    string label = "🔄 Rotation";
    string widget_type = "slider";
    float minimum = 0.0;
    float maximum = 6.283185; // Approx 2*PI
    float step = 0.1; // Radians
> = 0.0;

// Color options
uniform bool ColorizeRays<
    string label = "🎨 Colorize Rays";
    string group = "Color Options";
> = false;

uniform float4 RayColor<
    string label = "🌈 Ray Color";
    string group = "Color Options";
    string widget_type = "color"; // Added widget_type for consistency
> = { 1.0, 0.8, 0.4, 1.0 }; // Golden yellow default

// Animation
uniform bool EnableRotation<
    string label = "🔄 Animate Rotation";
    string group = "Animation";
> = false;

uniform float RotationSpeed<
    string label = "⚙️ Rotation Speed";
    string group = "Animation";
    string widget_type = "slider";
    float minimum = -2.0;
    float maximum = 2.0;
    float step = 0.1;
> = 0.5;

// Advanced
uniform bool ExtendRays<
    string label = "🔍 Extend Beyond Bright Areas";
    string group = "Advanced";
> = true;

uniform bool AnamorphicRays<
    string label = "🎬 Anamorphic Rays";
    string group = "Advanced";
> = false;

#define MAX_RAY_SAMPLES 12
#define DEFAULT_RAY_SAMPLES 8
#define MIN_RAY_SAMPLES 4

// Add user parameter
uniform int ray_sample_count <
    string label = "Ray Sample Quality";
    string widget_type = "slider";
    int minimum = MIN_RAY_SAMPLES;
    int maximum = MAX_RAY_SAMPLES;
    int step = 1;
> = DEFAULT_RAY_SAMPLES;

// --- NEW Parameters for Ray Appearance & Core Glow ---
uniform float CoreGlowIntensity <
    string label = "✨ Core Glow Intensity";
    string group = "Ray Appearance";
    string description = "Adds a bloom/glow to the bright source pixels themselves.";
    string widget_type = "slider";
    float minimum = 0.0; float maximum = 2.0; float step = 0.05;
> = 0.3;

uniform bool CoreGlowUsesRayColor <
    string label = "✨ Core Glow Uses Ray Color";
    string group = "Ray Appearance";
    string description = "If true, core glow is tinted with Ray Color, else it brightens the source color or uses white.";
> = false;

uniform float RayEdgeSoftness <
    string label = "🔪 Ray Edge Softness";
    string group = "Ray Appearance";
    string description = "Controls the softness of the ray's side edges. Higher = softer.";
    string widget_type = "slider";
    float minimum = 0.5; float maximum = 5.0; float step = 0.1;
> = 1.5;


float4 mainImage(VertData v_in) : TARGET {
    float4 currentPixelColor = image.Sample(textureSampler, v_in.uv);
    float brightness = dot(currentPixelColor.rgb, float3(0.299, 0.587, 0.114));
    
    // --- Apply Core Glow ---
    if (CoreGlowIntensity > 0.0 && brightness > Threshold) {
        float glowAmountNormalized = pow(saturate((brightness - Threshold) / (1.0 - Threshold + 0.001)), 1.5); // How much brighter than threshold
        float actualGlow = glowAmountNormalized * CoreGlowIntensity;
        
        float3 glowTint;
        if (CoreGlowUsesRayColor) {
            glowTint = RayColor.rgb;
        } else {
            // Brighten original or add white-ish glow
            glowTint = lerp(float3(1.0,1.0,1.0), currentPixelColor.rgb, 0.5); // Mix of white and original color
        }
        currentPixelColor.rgb += glowTint * actualGlow; // Additive glow
        currentPixelColor.rgb = saturate(currentPixelColor.rgb);
    }
    
    float4 baseOutputColor = currentPixelColor; // This color now includes the core glow

    // Handle rotation
    float finalRotation = Rotation;
    if (EnableRotation) {
        finalRotation += elapsed_time * RotationSpeed;
    }
    
    float totalRayStrength = 0.0; // Accumulator for all rays' strength affecting this pixel

    // The main logic for ray generation starts here
    // We iterate over StarPoints to cast rays from *other* locations towards the *current pixel* (v_in.uv)
    // OR, more commonly for starbursts, for the current pixel, we see if it lies on a ray path from ANY bright source.
    // The current implementation iterates angles and samples *outwards* from current pixel.
    // This means if v_in.uv is dark, but is on a path of a ray from a bright spot, it should light up.
    // This is what ExtendRays helps achieve.

    // If ExtendRays is true OR if the current pixel itself is bright enough to cast rays.
    // The brightness check for the current pixel casting rays is implicitly handled if ExtendRays is false,
    // as totalRayStrength would remain 0 if no incoming rays are found.
    // The original code's "contribution" logic was a bit confusing. Let's simplify.
    // totalRayStrength will be the sum of light gathered along each ray direction *for the current pixel v_in.uv*.

    for (int i = 0; i < StarPoints; i++) { // Loop only up to StarPoints
        float angle = (6.2831853 / float(StarPoints)) * float(i) + finalRotation;
        
        float2 dir = float2(cos(angle), sin(angle));
        if (AnamorphicRays) {
            dir.y *= 0.5; // Compress vertical for horizontal streaks
            dir = normalize(dir);
        }
        
        float raySamplesAccumulator = 0.0;
        int sampleCount = ray_sample_count;
        
        // Sample along the ray in ONE direction (from current pixel outwards)
        // To detect if current pixel is on a ray, we should sample *towards* potential sources.
        // OR, if a pixel IS a source, it casts rays. The current logic tries to see if current pixel is on a ray.
        for (int j = 1; j <= sampleCount; j++) {
            float scale = float(j) / float(sampleCount); // 0 to 1 along ray length
            float2 samplePos = v_in.uv - dir * RayLength * scale; // Sample *towards* the potential source of the ray

            // Check bounds for this primary sample
            if (samplePos.x < 0.0 || samplePos.x > 1.0 || samplePos.y < 0.0 || samplePos.y > 1.0) {
                if (!ExtendRays) continue; // If not extending, stop if primary sample is out of bounds
                // If extending, we might still count thickness samples that are in bounds.
            }

            float rayBrightness = 0.0;
            if (!(samplePos.x < 0.0 || samplePos.x > 1.0 || samplePos.y < 0.0 || samplePos.y > 1.0)) {
                 rayBrightness = dot(image.Sample(textureSampler, samplePos).rgb, float3(0.299, 0.587, 0.114));
            }


            float falloff = pow(1.0 - scale, RaySmoothness);
            float currentRayContribution = 0.0;

            if (rayBrightness > Threshold * lerp(0.8, 0.4, scale) ) { // Softer threshold for distant parts of ray
                currentRayContribution = falloff;
            }

            // --- Ray Thickness with new RayEdgeSoftness ---
            if (currentRayContribution > 0.0 && RayThickness > 0.01) {
                float perpDistanceBase = RayThickness * 0.005 * uv_pixel_interval.y / RayLength; // Smaller base for thickness
                float2 perpDir = float2(-dir.y, dir.x);
                
                // Check 2 perpendicular samples on each side (4 total extra points for thickness)
                for (int p_side = -1; p_side <= 1; p_side +=2) { // -1 and 1 (sides)
                    for (int p_step = 1; p_step <= 2; p_step++) { // Two steps outwards for thickness
                        // float thickness_falloff = pow(1.0 - (float(p_step)-1.0) / 1.0, RayEdgeSoftness); // p_step=1 -> 1, p_step=2 -> 0
                        float thickness_falloff_normalized = 1.0 - (float(p_step -1) / 2.0); // 1.0 for p_step=1, 0.5 for p_step=2
                        float actual_thickness_falloff = pow(thickness_falloff_normalized, RayEdgeSoftness);

                        float2 perpSamplePos = v_in.uv - dir * RayLength * scale + // Point on central ray
                                               (perpDir * float(p_side) * perpDistanceBase * float(p_step)); // Offset perpendicularly

                        if (perpSamplePos.x >= 0.0 && perpSamplePos.x <= 1.0 && perpSamplePos.y >= 0.0 && perpSamplePos.y <= 1.0) {
                            float perpBright = dot(image.Sample(textureSampler, perpSamplePos).rgb, float3(0.299, 0.587, 0.114));
                            if (perpBright > Threshold * lerp(0.8, 0.4, scale)) {
                                raySamplesAccumulator += falloff * actual_thickness_falloff * 0.5; // Add less for side samples
                            }
                        }
                    }
                }
                raySamplesAccumulator += currentRayContribution; // Add central sample contribution
            } else if (currentRayContribution > 0.0) {
                 raySamplesAccumulator += currentRayContribution;
            }
        } // End samples along ray (j)
        totalRayStrength += raySamplesAccumulator / float(sampleCount); // Average samples for this ray direction
    } // End star points (i)

    totalRayStrength = saturate(totalRayStrength / (sqrt(float(StarPoints))*0.5 + 0.1) ); // Normalize based on points, sqrt helps for more points
                                                                                      // Needs careful tweaking based on how totalRayStrength accumulates

    float finalRayMixFactor = totalRayStrength * Intensity * 0.5; // Final scaling for intensity of rays
                                                                  // The 0.5 is an adjustment factor, tune as needed

    if (finalRayMixFactor > 0.001) {
        float3 appliedRayColor = ColorizeRays ? RayColor.rgb : float3(1.0, 1.0, 1.0); // White if not colorized
        // Additive blending for rays
        baseOutputColor.rgb += appliedRayColor * finalRayMixFactor;
        baseOutputColor.rgb = saturate(baseOutputColor.rgb);
    }
    
    return baseOutputColor;
}
