// Basic skinning vert shader works with current vertex and shader topologies, requires a cbuffer of boneweights, offsets, and a world matrix
// Adjust buffers for appropriate indexing and ease of use

//per vertex constant buffer data
cbuffer WorldBuffer : register(b0)
{
	matrix worldMatrix;
	matrix boneOffsets[38]; //CHANGED PER skeleton # of bones + 1, 0 index holds identity matrix
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
	float4 boneWeights : WEIGHTS;
	int4   boneIndices : INDICES;
	float4 tangent : TANGENTS;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
	float4 tan : TAN;
};

PixelShaderInput main(VertexShaderInput input)
{
	/*PixelShaderInput output;
	float4 skinnedPos = float4(0.0f, 0.0f, 0.0f, 1.0);
	input.pos.w = 1.0f;

	skinnedPos += input.boneWeights[0] * mul(input.pos, boneOffsets[input.boneIndices[0]]);
	skinnedPos += input.boneWeights[1] * mul(input.pos, boneOffsets[input.boneIndices[1]]);
	skinnedPos += input.boneWeights[2] * mul(input.pos, boneOffsets[input.boneIndices[2]]);
	skinnedPos += input.boneWeights[3] * mul(input.pos, boneOffsets[input.boneIndices[3]]);
	skinnedPos.w = 1.0f;

	float4 pos = output.surfacePos = mul(skinnedPos, worldMatrix);
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

	return output;*/

	//temp test section pass through shader skinning only

	PixelShaderInput output = (PixelShaderInput)0;
	float4 skinnedPos = float4(0.0f, 0.0f, 0.0f, 1.0);
	input.pos.w = 1.0f;
	skinnedPos += input.boneWeights[0] * mul(input.pos, boneOffsets[input.boneIndices[0]]);
	skinnedPos += input.boneWeights[1] * mul(input.pos, boneOffsets[input.boneIndices[1]]);
	skinnedPos += input.boneWeights[2] * mul(input.pos, boneOffsets[input.boneIndices[2]]);
	skinnedPos += input.boneWeights[3] * mul(input.pos, boneOffsets[input.boneIndices[3]]);
	skinnedPos.w = 1.0f;

	output.pos = skinnedPos;
	output.surfacePos = output.cameraPos = input.pos;
	output.uvw = input.uvw;

	//float4 T = normalize(mul(worldMatrix, input.tangent));
	//float4 B = normalize(mul(worldMatrix, float4(cross(float3(input.tangent.x, input.tangent.y, input.tangent.z), float3(input.norm.x, input.norm.y, input.norm.z)), 0)));
	//float4 N = normalize(mul(worldMatrix, input.norm));
	//float4 BottomRow = float4(0,0,0,1);
	//output.tbn = matrix(T,B,N,BottomRow);
	output.tan = input.tangent;
	return output;
}