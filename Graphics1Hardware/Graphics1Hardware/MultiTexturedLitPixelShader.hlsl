texture2D baseTexture : register(t0); // first texture

texture2D NormalTexture : register(t1); //Normal texture

texture2D SpecTexture : register(t2); //Spec texture

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
	float4 tan : TAN;
	matrix tbn : TBN;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filter, input.uvw.xy);
	float4 SpecuPow = SpecTexture.Sample(filter, input.uvw.xy);
	float specuLum = (.299 * SpecuPow.x + .587 * SpecuPow.y + .114 * SpecuPow.z);
	float4 normmap = NormalTexture.Sample(filter, input.uvw.xy);
	normmap = normalize(mul(normmap, 2.0f) - 1.0f);
	normmap = normalize(mul(normmap, input.tbn));
	float3 viewDir = normalize(input.cameraPos.xyz - input.surfacePos.xyz);
	float specPower = 128.0f;
	float4 whiteLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 lightResult = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float lightRatio = saturate(dot(-normalize(dLightDir.xyz), normalize(normmap.xyz)));
	lightResult += (lightRatio * dLightColor);
	float3 halfVec = normalize(-normalize(dLightDir.xyz) + viewDir);
	float specIntensity = max(saturate(pow(saturate(dot(normmap.xyz, halfVec)), specuLum)), 0);
	lightResult += (specIntensity * whiteLight);
	lightResult += float4(.2f, .2f, .2f, .2f);
	//return baseColor;
	return saturate(lightResult * baseColor);
}