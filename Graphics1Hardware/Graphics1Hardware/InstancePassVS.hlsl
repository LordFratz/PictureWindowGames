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

PixelShaderInput main( VertexShaderInput input )
{
	PixelShaderInput output = (PixelShaderInput)0;
	output.pos = output.surfacePos = output.cameraPos = input.pos;
	output.uvw = input.uvw;
	output.norm = input.norm;
	return output;
}