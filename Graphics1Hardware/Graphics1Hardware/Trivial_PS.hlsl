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

void main( PixelShaderInput input )
{
	return;
}