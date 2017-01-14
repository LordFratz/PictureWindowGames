// Basic skinning vert shader works with current vertex and shader topologies, requires a cbuffer of boneweights, offsets, and a world matrix
// Adjust buffers for appropriate indexing and ease of use

//per vertex constant buffer data
cbuffer WorldBuffer : register(b0)
{
	matrix worldMatrix;
	float4 boneWeights[4];
	matrix boneOffsets[4];
};
//boneWeight[3] can be eliminated using boneWeight[3] = 1 - boneWeight[0] - boneWeight[1] - boneWeight[2] :: since weights must add up to 1

//global constant buffer data
cbuffer ViewProjectionBuffer : register(b1)
{
	matrix view;
	matrix projection;
	float4 cameraPosition;
}

//each vertex
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

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 skinnedPos = float4(0.0f, 0.0f, 0.0f, 1.0);
	input.pos.w = 1.0f;

	skinnedPos += boneWeights[0] * mul(input.pos, boneOffsets[0]);
	skinnedPos += boneWeights[1] * mul(input.pos, boneOffsets[1]);
	skinnedPos += boneWeights[2] * mul(input.pos, boneOffsets[2]);
	skinnedPos += boneWeights[3] * mul(input.pos, boneOffsets[3]);
	skinnedPos.w = 1.0f;

	float pos = output.surfacePos = mul(skinnedPos, worldMatrix);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	pos = input.norm;
	pos.w = 1.0f;
	pos = mul(pos, worldMatrix);
	output.norm = pos;

	pos.x = cameraPosition.x;
	pos.y = cameraPosition.y;
	pos.z = cameraPosition.z;
	output.cameraPos = pos;

	output.uvw = input.uvw;

	return output;
}