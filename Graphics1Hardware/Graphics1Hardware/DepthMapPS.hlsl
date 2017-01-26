struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
	float4 depthPosition : TEXTURE0;
	float4 lightPers : TEXTURE1;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float depthValue = input.lightPers.z / input.lightPers.w;
	float4 color = float4(depthValue, depthValue, depthValue, 1.0f);
	return color;
}