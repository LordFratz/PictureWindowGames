struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
};

struct VertexShaderInput
{
	float4 pos : POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 boneWeights : WEIGHTS;
	int4   boneIndices : INDICES;
};

PixelShaderInput main( VertexShaderInput pos )
{
	PixelShaderInput temp = (PixelShaderInput)0;
	return temp;
}