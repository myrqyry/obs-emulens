uniform float4x4 ViewProj;
uniform texture2D image;
uniform texture2D condition_image; // The canny-edge output

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

// Pass-through fragment shader
float4 main_fragment(VertToFrag v_in) : TARGET
{
	// For now, just pass through the original image
	return tex2D(image, v_in.uv);
}

technique Draw
{
	pass
	{
		vertex_shader = main_vertex;
		pixel_shader  = main_fragment;
	}
}
