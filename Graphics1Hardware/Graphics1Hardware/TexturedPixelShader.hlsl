Texture2D Texture : register(t0);
sampler Sampler : register(s0);

struct PixelShaderInput
{
	float4 PositionWS : SV_POSITION;
	float4 NormalWS : NORM;
	float4 TexCoord : UV;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
	return Texture.Sample(Sampler, IN.TexCoord);
}