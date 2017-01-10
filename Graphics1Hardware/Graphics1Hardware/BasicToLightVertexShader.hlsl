cbuffer WorldBuffer : register(b0)
{
	matrix worldMatrix;
};

cbuffer ViewProjectionBuffer : register(b1)
{
	matrix view;
	matrix projection;
	float4 cameraPosition;
}

struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = input.pos;
	pos.w = 1.0f;
	output.surfacePos = pos = mul(pos, worldMatrix);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
	
	pos = input.norm;
	pos.w = 1.0f;
	pos = mul(pos, worldMatrix);
	output.norm = pos;

	pos.x = cameraPosition.x;
	pos.y = cameraPosition.y;
	pos.z = cameraPosition.z;
	output.cameraPos = pos;
	
	output.uvw = input.uvw;

	return output;
}