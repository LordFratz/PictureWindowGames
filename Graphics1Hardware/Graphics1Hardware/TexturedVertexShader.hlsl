cbuffer WorldBuffer : register(b0)
{
	matrix worldMatrix;
};

cbuffer ViewProjectionBuffer : register(b1)
{
	matrix view;
	matrix projection;
	matrix camPos;
};

struct VertexShaderInput
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float3 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 PositionWS   : TEXCOORD1;
	float3 NormalWS     : TEXCOORD2;
	float2 TexCoord     : TEXCOORD0;
	float4 Position     : SV_Position;
};

VertexShaderOutput main( VertexShaderInput IN)
{
	VertexShaderOutput OUT;

	OUT.Position = mul(IN.Position, worldMatrix);
	OUT.Position = mul(OUT.Position, view);
	OUT.Position = mul(OUT.Position, projection);

	OUT.PositionWS = mul(worldMatrix, float4(IN.Position, 1.0f));

	OUT.NormalWS = mul((worldMatrix, IN.Normal);

	OUT.TexCoord = IN.TexCoord;

	return OUT;
}