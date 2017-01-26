struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	//float4 surfacePos : SURPOS;
	//float4 cameraPos : CAMPOS;
	//float4 depthPosition : TEXTURE0;
	//float4 lightViewPosition : TEXCOORD1;
};

float4 main( PixelShaderInput input ) : SV_TARGET
{
	return float4(0.0f, 0.2f, 0.4f, 1.0f);
}