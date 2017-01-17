/*
#pragma pack_matrix(row_major)
struct GSOutput
{
float4 pos : SV_POSITION;
float4 colorOut : COLOR;
float3 uvw : UVW;
};
struct GSInput
{
float4 colorOut : COLOR;
float4 projectedCoordinate : SV_POSITION;
};

cbuffer WORLD_MATRICES : register(b1)
{
float4x4 world0;
float4x4 world1;
}

cbuffer WORLD_VIEWPOINT : register(b2)
{
float4x4 viewpoint;
}

cbuffer Instance_Transforms : register(b3)
{
float4x4 transforms[3];
}
[instance(3)]
[maxvertexcount(4)]
void main(
point float4 input[1] : SV_POSITION,
unsigned int index : SV_GSInstanceID,
inout TriangleStream< GSOutput > output
)
{
float4 ndcVector;
GSOutput Quad[4];
GSOutput element = (GSOutput)0;
element.colorOut = float4(1.0f, 0.0f, 1.0f, 0.4f);
element.pos = input[0];

element.pos.x -= 2.5f;
element.pos.y += 2.5f;
Quad[0] = element;
element.pos.x += 5.0f;
element.uvw.x = 1.0f;
Quad[1] = element;
element.pos.x -= 5.0f;
element.pos.y -= 5.0f;
element.uvw.x = 0.0f;
element.uvw.y = 1.0f;
Quad[2] = element;
element.pos.x += 5.0f;
element.uvw.x = 1.0f;
Quad[3] = element;

for (int i = 0; i < 4; i++)
{
ndcVector = Quad[i].pos;
ndcVector.w = 1;
ndcVector = mul(ndcVector, transforms[index]);
ndcVector = mul(ndcVector, world1);
ndcVector = mul(ndcVector, viewpoint);
Quad[i].pos = ndcVector;
output.Append(Quad[i]);
}
}
*/

//PixelShaderInput
struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
};

//from Vertex Shader
struct GSInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
};

//transforms stored in ndc (premultiplied on cpu by world/view/proj)
cbuffer Instance_Transforms : register(b5)
{
	float4x4 transforms[3];
}

[instance(3)]
[maxvertexcount(3)]
void main(
	triangle GSInput input[3] : SV_POSITION, 
	unsigned int index : SV_GSInstanceID,
	inout TriangleStream< GSOutput > output
)
{
	//loop once for each vert of triangle
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element = input[i];
		element.pos = mul(input[i].pos, transforms[index]);
		output.Append(element);
	}
}