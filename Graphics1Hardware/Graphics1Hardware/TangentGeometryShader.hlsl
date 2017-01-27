cbuffer WorldBuffer : register(b0)
{
	matrix worldMatrix;
};

cbuffer ViewProjectionBuffer : register(b1)
{
	matrix view;
	matrix projection;
	float4 cameraPosition;
}

//PixelShaderInput
struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
	float4 tan : TAN;
	matrix tbn : TBN;
};

//from Vertex Shader
struct GSInput
{
	float4 pos : SV_POSITION;
	float4 uvw : UVW;
	float4 norm : NORM;
	float4 surfacePos : SURPOS;
	float4 cameraPos : CAMPOS;
	float4 tan : TAN;
	matrix tbn : TBN;
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
		float4 pos = input[i].pos;
		pos.w = 1.0f;
		pos = mul(pos, transforms[index]);
		pos = mul(pos, worldMatrix);
		element.surfacePos = pos;
		pos = mul(pos, view);
		pos = mul(pos, projection);
		element.pos = pos;

		pos.x = cameraPosition.x;
		pos.y = cameraPosition.y;
		pos.z = cameraPosition.z;
		element.cameraPos = pos;

		float Det = 1;
		if (input[i].pos.x < 0) {
			Det = -1;
		}

		float4 T = normalize(mul(worldMatrix, input[i].tan));
		float4 N = normalize(mul(worldMatrix, element.norm));
		//T = normalize(T - dot(T, N) * N);
		float4 B = float4(cross(float3(N.x, N.y, N.z), float3(T.x, T.y, T.z)), 0);
		float4 BottomRow = float4(0, 0, 0, 1);

		if (dot(float4(cross(float3(N.x, N.y, N.z), float3(T.x, T.y, T.z)), 0), B) < 0.0) {
			T = T * -1.0;
		}

		element.tbn = matrix(T, B, N, BottomRow);

		output.Append(element);
	}
}