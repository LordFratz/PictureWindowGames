cbuffer ViewProjectionBuffer : register(b1)
{
	matrix view;
	matrix projection; //Look into cameraData in main (may need to update subresource or whatever)
	float4 cameraPosition;
}

struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 color : COLOR;
	//Per-instance data
	matrix Matrix : WORLDMATRIX;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = input.pos;
	pos.w = 1.0f;
	pos = mul(input.Matrix, pos);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	output.color = float3(input.color.x, input.color.y, input.color.z);

	return output;
}