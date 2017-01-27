texture2D baseTexture : register(t0); // first texture
Texture2D depthMapTexture : register(t5);


SamplerState filter : register(s0);
SamplerState clampFilter : register(s1);

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
	float4 depthPosition : TEXTURE0;
	float4 lightViewPosition : TEXTURE1;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float2 projectTexCoord;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;
	float4 baseColor = baseTexture.Sample(filter, input.uvw.xy);
	float3 viewDir = normalize(input.cameraPos.xyz - input.surfacePos.xyz);
	float specPower = 128.0f;
	float constantSpecIntensity = 1.0f;
	float4 whiteLight = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 lightResult = float4(0.0f, 0.0f, 0.0f, 0.0f);


	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
	{
		float depthValue = depthMapTexture.Sample(clampFilter, projectTexCoord).r;
		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		lightDepthValue = lightDepthValue - 0.001f;

		if (lightDepthValue < depthValue)
		{
				float lightRatio = saturate(dot(-normalize(dLightDir.xyz), normalize(input.norm.xyz)));
				lightResult += (lightRatio * dLightColor);
				float3 halfVec = normalize(-normalize(dLightDir.xyz) + viewDir);
				float specIntensity = max(saturate(pow(saturate(dot(input.norm.xyz, halfVec)), specPower)), 0);
				lightResult += (specIntensity * constantSpecIntensity * whiteLight);
		}
	}
	else
	{
		float lightRatio = saturate(dot(-normalize(dLightDir.xyz), normalize(input.norm.xyz)));
		lightResult += (lightRatio * dLightColor);
		float3 halfVec = normalize(-normalize(dLightDir.xyz) + viewDir);
		float specIntensity = max(saturate(pow(saturate(dot(input.norm.xyz, halfVec)), specPower)), 0);
		lightResult += (specIntensity * constantSpecIntensity * whiteLight);
	}
	
	
	//return baseColor;
	return saturate(lightResult * baseColor);
}