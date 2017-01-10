texture2D baseTexture : register(t0); // first texture

SamplerState filter : register(s0);

cbuffer DirectionalLight : register(b2)
{
	float4 dLightPos;
	float4 dLightColor;
	float4 dLightDir;
}

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filter, input.uvw.xy);
	float3 viewDir = normalize(input.cameraPos.xyz - input.surfacePos.xyz);
	float specPower = 128.0f;
	float constantSpecIntensity = 1.0f;
	float4 whiteLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 lightResult = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float lightRatio = saturate(dot(-normalize(dLightDir.xyz), normalize(input.norm.xyz)));
	lightResult += (lightRatio * dLightColor);
	float3 halfVec = normalize(-normalize(dLightDir.xyz) + viewDir);
	float specIntensity = max(saturate(pow(saturate(dot(input.norm.xyz, halfVec)), specPower)), 0);
	lightResult += (specIntensity * constantSpecIntensity * whiteLight);

	return saturate(lightResult * baseColor);
}
