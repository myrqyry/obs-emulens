uniform float4x4 ViewProj;
uniform texture2D image;
uniform float2 uv_size; // width and height of the texture

// Canny edge detection parameters
uniform float threshold1 = 100.0 / 255.0; // Lower threshold (normalized)
uniform float threshold2 = 200.0 / 255.0; // Upper threshold (normalized)

struct VertData {
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct VertToFrag {
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

VertToFrag main_vertex(VertData v_in)
{
	VertToFrag v_out;
	v_out.pos = mul(v_in.pos, ViewProj);
	v_out.uv = v_in.uv;
	return v_out;
}

// Helper to convert to grayscale
float grayscale(float4 color)
{
	return dot(color.rgb, float3(0.299, 0.587, 0.114));
}

// Sobel edge detection
float sobel(texture2D tex, float2 uv, float2 pixel_size)
{
	// Get grayscale values of 3x3 neighborhood
	float tl = grayscale(tex2D(tex, uv + float2(-pixel_size.x, -pixel_size.y)));
	float t  = grayscale(tex2D(tex, uv + float2(0.0, -pixel_size.y)));
	float tr = grayscale(tex2D(tex, uv + float2(pixel_size.x, -pixel_size.y)));
	float l  = grayscale(tex2D(tex, uv + float2(-pixel_size.x, 0.0)));
	float r  = grayscale(tex2D(tex, uv + float2(pixel_size.x, 0.0)));
	float bl = grayscale(tex2D(tex, uv + float2(-pixel_size.x, pixel_size.y)));
	float b  = grayscale(tex2D(tex, uv + float2(0.0, pixel_size.y)));
	float br = grayscale(tex2D(tex, uv + float2(pixel_size.x, pixel_size.y)));

	// Sobel operators
	float Gx = -tl - 2.0 * l - bl + tr + 2.0 * r + br;
	float Gy = -tl - 2.0 * t - tr + bl + 2.0 * b + br;

	// Gradient magnitude
	return length(float2(Gx, Gy));
}


float4 main_fragment(VertToFrag v_in) : TARGET
{
	float2 pixel_size = 1.0 / uv_size;
	float magnitude = sobel(image, v_in.uv, pixel_size);

	// Simple thresholding - this is a simplified Canny
	if (magnitude > threshold1) {
		return float4(1.0, 1.0, 1.0, 1.0); // Edge is white
	} else {
		return float4(0.0, 0.0, 0.0, 1.0); // No edge is black
	}
}

technique Draw
{
	pass
	{
		vertex_shader = main_vertex;
		pixel_shader  = main_fragment;
	}
}
