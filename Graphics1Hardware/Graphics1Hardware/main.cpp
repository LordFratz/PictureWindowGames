// CGS HW Project A "Line Land".
// Author: L.Norri CD CGS, FullSail University

// Introduction:
// Welcome to the hardware project of the Computer Graphics Systems class.
// This assignment is fully guided but still requires significant effort on your part.
// Future classes will demand the habits & foundation you develop right now!
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this project and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include "XTime.h"
#include <memory>
#pragma comment (lib, "d3d11.lib")

using namespace std;

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
using namespace DirectX;
#include "../FBXExporter/IncludeMe.h"
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"
#include "BasicVertexShader.csh"
#include "BasicPixelShader.csh"
#include "BasicLitSkinningVertShader.csh"
#include "BasicToLightVertexShader.csh"
#include "BasicLightPixelShader.csh"
#include "DeviceResources.h"
#include "Renderer.h"
#include "DDSTextureLoader.h"
#define BACKBUFFER_WIDTH	800
#define BACKBUFFER_HEIGHT	600

struct ViewProj
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4 cameraPos;
};

struct DirectionalLight
{
	XMFLOAT4 dLightPos;
	XMFLOAT4 dLightColor;
	XMFLOAT4 dLightDir;
};

struct VertexPositionColor
{
	XMFLOAT3 pos;
	XMFLOAT3 color;
};

struct VertexPositionUVWNorm
{
	XMFLOAT4 pos;
	XMFLOAT4 UVW;
	XMFLOAT4 Norm;
};

struct SkinnedVert
{
	XMFLOAT4 pos;
	XMFLOAT4 UVW;
	XMFLOAT4 Norm;
	XMFLOAT4 Weights;
	XMINT4   Indices;
};

struct BoxSkinnedConstBuff
{
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 boneOffsets[5];
};

class Camera
{
	XMMATRIX viewMatrix;
	POINT currCursor, prevCursor;;
public:
	ViewProj cameraData;
	void init(int sWidth, int sHeight)
	{
		static const XMVECTORF32 eye = { 0.0f, 0.0f, -10.0f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		viewMatrix = XMMatrixInverse(0, XMMatrixLookAtLH(eye, at, up));
		XMStoreFloat4x4(&cameraData.view, XMMatrixTranspose(viewMatrix));
		XMStoreFloat4(&cameraData.cameraPos, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
		float aspectRatio = float(sWidth) / float(sHeight);

		float fovAngleY = 60.0f * XM_PI / 180.0f;

		if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}

		XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
			fovAngleY,
			aspectRatio,
			0.01f,
			100.0f
		);

		XMStoreFloat4x4(&cameraData.projection, XMMatrixTranspose(perspectiveMatrix));
	}

	void update(float delta)
	{
		float cameraSpeed = 0.5f * 0.001f; // * a delta time when time is added
		float cameraRotateSpeed = 5.0f * cameraSpeed;
		GetCursorPos(&currCursor);
		if (GetAsyncKeyState(87))
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixTranslation(0.0f, 0.0f, cameraSpeed));
		if (GetAsyncKeyState(83))
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixTranslation(0.0f, 0.0f, -cameraSpeed));
		if (GetAsyncKeyState(65))
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixTranslation(cameraSpeed, 0.0f, 0.0f));
		if (GetAsyncKeyState(68))
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixTranslation(-cameraSpeed, 0.0f, 0.0f));
		if (GetAsyncKeyState(67))
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixTranslation(0.0f, cameraSpeed, 0.0f));
		if (GetAsyncKeyState(88))
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixTranslation(0.0f, -cameraSpeed, 0.0f));

		if (GetAsyncKeyState(2))
		{
			float deltaX = (float)(currCursor.x - prevCursor.x);
			float deltaY = (float)(currCursor.y - prevCursor.y);
			XMMATRIX tempMatrix = viewMatrix;
			XMVectorSetX(viewMatrix.r[3], 0.0f);
			XMVectorSetY(viewMatrix.r[3], 0.0f);
			XMVectorSetZ(viewMatrix.r[3], 0.0f);
			viewMatrix = XMMatrixMultiply(XMMatrixRotationX(deltaY * cameraRotateSpeed), viewMatrix);
			viewMatrix = XMMatrixMultiply(viewMatrix, XMMatrixRotationY(deltaX * cameraRotateSpeed));
			XMVectorSetX(viewMatrix.r[3], XMVectorGetX(tempMatrix.r[3]));
			XMVectorSetY(viewMatrix.r[3], XMVectorGetY(tempMatrix.r[3]));
			XMVectorSetZ(viewMatrix.r[3], XMVectorGetZ(tempMatrix.r[3]));
		}

		XMStoreFloat4x4(&cameraData.view, XMMatrixTranspose(viewMatrix));
		XMVECTOR tempVec = viewMatrix.r[3];
		XMVectorSetW(tempVec, 1.0f);
		XMStoreFloat4(&(cameraData.cameraPos), tempVec);
		prevCursor = currCursor;
	}
};

static Camera* CurrCamera;
static Microsoft::WRL::ComPtr<ID3D11Buffer> LightBuff;
//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	shared_ptr<DeviceResources> devResources;
	RenderContext* planeContext = nullptr;
	RenderShape* planeShape = nullptr;
	RenderMesh* planeMesh = nullptr;

	RenderContext* ModelContext = nullptr;
	RenderShape* ModelShape = nullptr;
	RenderMesh* ModelMesh = nullptr;

	RenderSet Set;
	ID3D11Buffer *VertBuffer;
	const unsigned int vertCount = 361;
	ID3D11Buffer *ConstBuffer;
	ID3D11InputLayout *pLayout;
	XTime timer;
	XMFLOAT2 offsetVel;
	ID3D11Buffer *BackgroundBuffer;
#define NumVertsBackground 1200
	ID3D11VertexShader *pVS;
	ID3D11PixelShader *pPS;
	struct SEND_TO_VRAM
	{
		XMVECTORF32 constantColor;
		XMFLOAT2 constantOffset;
		XMFLOAT2 padding;
	};
	SEND_TO_VRAM toShader;
	SEND_TO_VRAM toShader2;

	//FBXLoaded data (temp till milestone 2)


	//added for dynamic light
	DirectionalLight dynaLight;
	//added for camera

public:
	struct SIMPLE_VERTEX
	{
		XMFLOAT2 position;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

namespace
{
	/// <summary>
	/// Generic TEXTURELESS RenderContext Function
	/// </summary>
	/// <param name="rNode">The r node.</param>
	void PlaneContext(RenderNode &rNode)
	{
		auto Node = &(RenderContext&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();
		context->IASetInputLayout(Node->m_inputLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetShader(Node->m_vertexShader.Get(), nullptr, 0);
		context->PSSetShader(Node->m_pixelShader.Get(), nullptr, 0);
		auto ContextSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->ContextData[0];
		context->UpdateSubresource(ContextSubresource1->Get(), 0, NULL, &CurrCamera->cameraData,0, 0);
		context->VSSetConstantBuffers(1, 1, ContextSubresource1->GetAddressOf());
		context->PSSetConstantBuffers(2, 1, LightBuff.GetAddressOf());
	}

	void CleanupPlaneContext(std::vector<void*> toClean)
	{
		auto ContextSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[0];
		ContextSubresource1->Reset();
	}

	/// <summary>
	/// Generic TEXTURELESS RenderShape Function
	/// </summary>
	/// <param name="rNode">The r node.</param>
	void PlaneShape(RenderNode &rNode)
	{
		auto Node = &(RenderShape&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();
		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[0];
		context->UpdateSubresource(ShapeSubresource1->Get(), 0, NULL, &Node->WorldMat, 0, 0);
		context->VSSetConstantBuffers(0, 1, ShapeSubresource1->GetAddressOf());
		auto vertexBuffer = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[1];
		UINT stride = sizeof(VertexPositionUVWNorm);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer->GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(Node->Mesh.m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
	}

	void CleanupPlaneShape(std::vector<void*> toClean)
	{
		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[0];
		ShapeSubresource1->Reset();
		auto ShapeSubresource2 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[1];
		ShapeSubresource2->Reset();
	}

	/// <summary>
	/// Generic TEXTURED RenderContext Function.
	/// </summary>
	/// <param name="rNode">The r node.</param>
	void TexturedContext(RenderNode &rNode)
	{
		auto Node = &(RenderContext&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();

	}

	/// <summary>
	/// Generic TEXTURED RenderShape Function.
	/// </summary>
	/// <param name="rNode">The r node.</param>
	void TexturedShape(RenderNode &rNode)
	{
		auto Node = &(RenderShape&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();

		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[0];
		context->UpdateSubresource(ShapeSubresource1->Get(), 0, NULL, &Node->WorldMat, 0, 0);
		context->VSSetConstantBuffers(0, 1, ShapeSubresource1->GetAddressOf());
		auto vertexBuffer = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[1];
		UINT stride = sizeof(VertexPositionUVWNorm);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer->GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(Node->Mesh.m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		auto Sampler = (Microsoft::WRL::ComPtr<ID3D11SamplerState>*)Node->Mesh.MeshData[2];
		context->PSSetSamplers(0, 1, Sampler->GetAddressOf());
		auto Texture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[3];
		context->PSSetShaderResources(0, 1, Texture->GetAddressOf());

		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
	}

	void CleanupTexturedShape(std::vector<void*> toClean)
	{
		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[0];
		ShapeSubresource1->Reset();
		auto ShapeSubresource2 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[1];
		ShapeSubresource2->Reset();
		auto Sampler = (Microsoft::WRL::ComPtr<ID3D11SamplerState>*)toClean[2];
		Sampler->Reset();
		auto Texture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)toClean[3];
		Texture->Reset();
	}

	void NoCleanup(std::vector<void*> toClean){}

	void SkinnedShape(RenderNode &rNode)
	{
		auto Node = &(RenderShape&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();

		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[0];
		context->UpdateSubresource(ShapeSubresource1->Get(), 0, NULL, (BoxSkinnedConstBuff*)Node->ShapeData[0], 0, 0);
		context->VSSetConstantBuffers(0, 1, ShapeSubresource1->GetAddressOf());
		auto vertexBuffer = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[1];
		UINT stride = sizeof(SkinnedVert);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer->GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(Node->Mesh.m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		auto Sampler = (Microsoft::WRL::ComPtr<ID3D11SamplerState>*)Node->Mesh.MeshData[2];
		context->PSSetSamplers(0, 1, Sampler->GetAddressOf());
		auto Texture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[3];
		context->PSSetShaderResources(0, 1, Texture->GetAddressOf());

		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
	}

	void CleanSkinnedShape(std::vector<void*> toClean)
	{
		delete toClean[0];
	}

	void SkinnedUpdate(RenderShape &node, float delta)
	{
		auto bufferData =   (BoxSkinnedConstBuff*)node.ShapeData[0];
		auto keyframes =    (XMFLOAT4X4**)node.ShapeData[1];
		auto numKeyframes = (int*)node.ShapeData[2];
		auto currKeyframe = (int*)node.ShapeData[3];
		auto lastKeyframe = (int*)node.ShapeData[4];
		auto numBones = (int*)node.ShapeData[5];
		auto ranLastFrame = (bool*)node.ShapeData[6];

		if(GetAsyncKeyState(0x4f) & !*ranLastFrame)
		{
			*currKeyframe = *currKeyframe - 1;
			if(*currKeyframe < 0)
			{
				*currKeyframe = *numKeyframes - 1;
			}
			else if (*currKeyframe > *numKeyframes - 1)
			{
				*currKeyframe = 0;
			}
			*ranLastFrame = true;
		}
		else if(GetAsyncKeyState(0x50) & !*ranLastFrame)
		{
			*currKeyframe = *currKeyframe + 1;
			if(*currKeyframe > *numKeyframes - 1)
			{
				*currKeyframe = 0;
			}
			else if (*currKeyframe < 0)
			{
				*currKeyframe = *numKeyframes - 1;
			}
			*ranLastFrame = true;
		}
		else
		{
			*ranLastFrame = false;
		}

		if(*currKeyframe != *lastKeyframe)
		{
			*lastKeyframe = *currKeyframe;
			for(int i = 0; i < *numBones; i++)
			{
				bufferData->boneOffsets[i + 1] = keyframes[i][*currKeyframe];
			}
		}
		*(BoxSkinnedConstBuff*)node.ShapeData[0] = *bufferData;
		*(int*)node.ShapeData[3] = *currKeyframe;
		*(int*)node.ShapeData[4] = *lastKeyframe;
		*(bool*)node.ShapeData[6] = *ranLastFrame;

	}

	void CleanSkinnedUpdates(std::vector<void*> toClean)
	{
		delete toClean[0];
		delete[] toClean[1];
		delete toClean[2];
		delete toClean[3];
		delete toClean[4];
		delete toClean[5];
		delete toClean[6];
	}
}


//JIT extremely unoptomized, useful for only small numbers of objects
namespace Collisions
{
	void CheckCollisions(std::vector<RenderShape*> *shapes)
	{
		XMVECTOR* tempPositions = new XMVECTOR[shapes->size()];

		for (size_t i = 0; i < shapes->size(); i++)
		{
			tempPositions[i] = XMLoadFloat4x4(&((*shapes)[i]->WorldMat)).r[3];
		}


		for (size_t i = 0; i < shapes->size() - 1; i++)
		{
			float tempRadius = (*shapes)[i]->BoundingSphere.radius;
			if (tempRadius > 0.0f)
			{
				for (size_t j = i; j < shapes->size(); j++)
				{
					float tempRadius2 = (*shapes)[j]->BoundingSphere.radius;
					if (tempRadius2 > 0.0f && XMVector3Dot(tempPositions[i] - tempPositions[j], tempPositions[i] - tempPositions[j]).m128_f32[0] <= (tempRadius + tempRadius2) * (tempRadius + tempRadius2))
					{
						//collision! //add or trigger motion reaction and geometry deformation //will return to this later
					}
				}
			}
			else if (tempRadius == 0.0f)
			{
				//shape is the ground plane, check against all other shapes
				for (size_t j = i; j < shapes->size(); j++)
				{
					float tempRadius2 = (*shapes)[j]->BoundingSphere.radius;
					if (tempRadius2 > 0.0f && tempPositions[j].m128_f32[1] + 0.5f <= tempRadius2)
					{
						//collision add upward motion and geometry squishing

					}

				//	if((XMVector3Dot(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), tempPositions[j]) - XMVector3Dot(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f))).m128_f32[0] >= 0.0f)
				//	{
				//		//point ahead or on plane

				//	}
				//	else
				//	{
				//		//point behind plane //will return later here
				//	}
				}
			}
		}
	}
}

namespace GenerateObject
{
	void CreateD20(VertexPositionUVWNorm* Mesh, int* Ind) {
		//generate Verticies for D20
		float t = (1.0f + sqrt(5.0f)) / 2.0f;
		Mesh = new VertexPositionUVWNorm[12];
		Mesh[0].pos = XMFLOAT4(-1, t, 0, 1);
		Mesh[1].pos = XMFLOAT4(1, t, 0, 1);
		Mesh[2].pos = XMFLOAT4(-1, -t, 0, 1);
		Mesh[3].pos = XMFLOAT4(1, -t, 0, 1);

		Mesh[4].pos = XMFLOAT4(0, -1, t, 1);
		Mesh[5].pos = XMFLOAT4(0, 1, t, 1);
		Mesh[6].pos = XMFLOAT4(0, -1, -t, 1);
		Mesh[7].pos = XMFLOAT4(0, 1, -t, 1);

		Mesh[8].pos = XMFLOAT4(t, 0, -1, 1);
		Mesh[9].pos = XMFLOAT4(t, 0, 1, 1);
		Mesh[10].pos = XMFLOAT4(-t, 0, -1, 1);
		Mesh[11].pos = XMFLOAT4(-t, 0, 1, 1);

		for (int i = 0; i < 12; i++) {
			float length = sqrt((Mesh[i].pos.x * Mesh[i].pos.x) + (Mesh[i].pos.y * Mesh[i].pos.y) + (Mesh[i].pos.z * Mesh[i].pos.z));
			Mesh[i].pos.x = Mesh[i].pos.x / length;
			Mesh[i].pos.y = Mesh[i].pos.y / length;
			Mesh[i].pos.z = Mesh[i].pos.z / length;
		}

		Ind = new int[60];
		Ind[0]  = 0; Ind[1]  = 11; Ind[2]  = 5;
		Ind[3]  = 0; Ind[4]  = 5; Ind[5]  = 1;
		Ind[6]  = 0; Ind[7]  = 1; Ind[8]  = 7;
		Ind[9]  = 0; Ind[10] = 7; Ind[11] = 10;
		Ind[12] = 0; Ind[13] = 10; Ind[14] = 11;

		Ind[15] = 1; Ind[16] = 5; Ind[17] = 9;
		Ind[18] = 5; Ind[19] = 11; Ind[20] = 4;
		Ind[21] = 11; Ind[22] = 10; Ind[23] = 2;
		Ind[24] = 10; Ind[25] = 7; Ind[26] = 6;
		Ind[27] = 7; Ind[28] = 1; Ind[29] = 8;

		Ind[30] = 3; Ind[31] = 9; Ind[32] = 4;
		Ind[33] = 3; Ind[34] = 4; Ind[35] = 2;
		Ind[36] = 3; Ind[37] = 2; Ind[38] = 6;
		Ind[39] = 3; Ind[40] = 6; Ind[41] = 8;
		Ind[42] = 3; Ind[43] = 8; Ind[44] = 9;

		Ind[45] = 4; Ind[46] = 9; Ind[47] = 5;
		Ind[48] = 2; Ind[49] = 4; Ind[50] = 11;
		Ind[51] = 6; Ind[52] = 2; Ind[53] = 10;
		Ind[54] = 8; Ind[55] = 6; Ind[56] = 7;
		Ind[57] = 9; Ind[58] = 8; Ind[59] = 1;
	}
}

namespace ShaderLoader
{
	bool  LoadShader(std::vector<uint8_t> & _data, const char * _fileName) {
		FILE * file = nullptr;
		fopen_s(&file, _fileName, "rb");
		if (!file) return false; //Normally Log that there was a bad file name sent in
		fseek(file, 0, SEEK_END);
		long size = ftell(file);
		fseek(file, 0, SEEK_SET);
		_data.resize(size);
		fread(&_data[0], sizeof(uint8_t), size, file);
		fclose(file);
		return true;
	}
}

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************//
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
#if 1
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );
    wndClass.lpfnWndProc    = appWndProc;
    wndClass.lpszClassName  = L"DirectXApplication";
	wndClass.hInstance      = application;
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME );
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"CGS Hardware Project",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX),
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,
							NULL, NULL,	application, this );

    ShowWindow( window, SW_SHOW );
#endif
	//********************* END WARNING ************************//

	//Loads in data from fbx file into FBX with examples
	//whatever::loadFile("../Resources/Box_Jump.fbx");
	//float** BoneMatTest = whatever::GetBoneBindMat();
	//int** BoneVertTest = whatever::GetBoneVertInds();
	//float** BoneWeightTest = whatever::GetBoneWeights();
	//int* BoneParentTest = whatever::GetParentInds();

	devResources = make_shared<DeviceResources>();
	devResources->initialize(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, window);

	auto Device = devResources->GetD3DDevice();



#if 1
	//Start Plane Init
	planeContext = new RenderContext(devResources, PlaneContext, CleanupPlaneContext, false);
	planeMesh = new RenderMesh(CleanupTexturedShape);
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, XMMatrixIdentity());
	planeShape = new RenderShape(devResources, *planeMesh, *planeContext, mat, sphere(), TexturedShape, NoCleanup);



#if true //use this section once the plane has UVs, normals, and a texture loaded
	//auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	//auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	//std::vector<uint8_t> VSData;
	//std::vector<uint8_t> PSData;
	//bool thing = ShaderLoader::LoadShader(VSData, "BasicToLightVertexShader.cso");
	//thing = ShaderLoader::LoadShader(VSData, "BasicToLightPixelShader.cso");
	//Device->CreateVertexShader(&VSData[0], VSData.size(), NULL, planeContext->m_vertexShader.GetAddressOf());
	Device->CreateVertexShader(&BasicToLightVertexShader, ARRAYSIZE(BasicToLightVertexShader), NULL, planeContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&BasicLightPixelShader, ARRAYSIZE(BasicLightPixelShader), NULL, planeContext->m_pixelShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UVW" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORM" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	Device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &BasicToLightVertexShader, ARRAYSIZE(BasicToLightVertexShader), planeContext->m_inputLayout.GetAddressOf());
#else //current plane
	Device->CreateVertexShader(&BasicVertexShader, ARRAYSIZE(BasicVertexShader), NULL, planeContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&BasicPixelShader, ARRAYSIZE(BasicPixelShader), NULL, planeContext->m_pixelShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	Device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &BasicVertexShader, ARRAYSIZE(BasicVertexShader), planeContext->m_inputLayout.GetAddressOf());
#endif



	CD3D11_BUFFER_DESC constBuffDesc(sizeof(ViewProj), D3D11_BIND_CONSTANT_BUFFER);
	auto Buffer = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, nullptr, Buffer->GetAddressOf());
	planeContext->ContextData.push_back(Buffer);

	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
	auto Buffer2 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, nullptr, Buffer2->GetAddressOf());
	planeMesh->MeshData.push_back(Buffer2);

	static const VertexPositionUVWNorm planeVerts[] =
	{
		{ XMFLOAT4(-2.5f, -0.5f, -2.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
		{ XMFLOAT4(-2.5f, -0.5f,  2.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
		{ XMFLOAT4(2.5f, -0.5f, -2.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) },
		{ XMFLOAT4(2.5f, -0.5f,  2.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f) }
	};

	D3D11_SUBRESOURCE_DATA BufferData = { 0 };
	BufferData.pSysMem = planeVerts;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(planeVerts), D3D11_BIND_VERTEX_BUFFER);
	auto Buffer3 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, &BufferData, Buffer3->GetAddressOf());
	planeMesh->MeshData.push_back(Buffer3);

	//Light initializations
	XMStoreFloat4(&dynaLight.dLightColor, XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f));//XMVectorSet(1.0f, 0.98f, 0.804f, 1.0f));
	XMStoreFloat4(&dynaLight.dLightDir, XMVectorSet(0.0f, -1.0f, 0.0f, 1.0f));
	XMStoreFloat4(&dynaLight.dLightPos, XMVectorSet(0.0f, 10.0f, 0.0f, 1.0f));

	D3D11_SUBRESOURCE_DATA BufferData2 = { 0 };
	BufferData2.pSysMem = &dynaLight;
	BufferData2.SysMemPitch = 0;
	BufferData2.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(DirectionalLight), D3D11_BIND_CONSTANT_BUFFER);
	//auto Buffer4 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, &BufferData2, LightBuff.GetAddressOf());
	//end light initializations

	auto SampleState = new Microsoft::WRL::ComPtr<ID3D11SamplerState>();
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	Device->CreateSamplerState(&samplerDesc, SampleState->GetAddressOf());
	planeMesh->MeshData.push_back(SampleState);

	auto SRV = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();
	const size_t size = strlen("../Resources/Plane.dds") + 1;
	size_t empty;
	wchar_t* wText = new wchar_t[size];
	mbstowcs_s(&empty, wText, size_t(size), "../Resources/Plane.dds", size_t(size));
	CreateDDSTextureFromFile(Device, wText, nullptr, SRV->GetAddressOf(), 0);
	wText = NULL;
	delete wText;
	planeMesh->MeshData.push_back(SRV);

	static const unsigned short planeIndices[] =
	{
		0,2,1, // -x
		1,2,3,
	};

	planeMesh->m_indexCount = ARRAYSIZE(planeIndices);
	BufferData = { 0 };
	BufferData.pSysMem = planeIndices;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(planeIndices), D3D11_BIND_INDEX_BUFFER);
	Device->CreateBuffer(&constBuffDesc, &BufferData, planeMesh->m_indexBuffer.GetAddressOf());
	//End Plane Init
#endif

	whatever::loadFile("../Resources/Box_Mesh.pwm", "../Resources/Box_Jump.fbx");
	whatever::loadFile("../Resources/Box_Skeleton.pws", "../Resources/Box_Jump.fbx");
	whatever::loadFile("../Resources/Box_JumpAnim.pwa", "../Resources/Box_Jump.fbx");
	//whatever::loadFile("../Resources/Teddy_Run.fbx");

	int numVerts = whatever::GetVertCount();
	short* IndexBuffer = whatever::GetShortInd();
	float* UVs = whatever::GetUVs();
	float* Norms = whatever::GetNormals();
	float* Verts = whatever::GetVerts();
	int numIndices = whatever::GetIndCount();
	//VertexPositionUVWNorm* VertexBuffer = new VertexPositionUVWNorm[numVerts];
	//for (int i = 0; i < numVerts; i++)
	//{
	//	VertexPositionUVWNorm Temp;
	//	Temp.pos = XMFLOAT4(Verts[i*4], Verts[i*4+1], Verts[i*4+2], Verts[i*4+3]);
	//	Temp.UVW = XMFLOAT4(UVs[i*2], UVs[i*2+1], 0, 0);
	//	Temp.Norm = XMFLOAT4(Norms[i*4], Norms[i*4+1], Norms[i*4+2], Norms[i*4+3]);
	//	VertexBuffer[i] = Temp;
	//}
	int** BoneIndices = whatever::GetVertToBoneInds();
	float** BoneWeights = whatever::GetVertWeightToBoneInds();
	SkinnedVert* SkinnedVertexBuffer = new SkinnedVert[numVerts];
	for(int i = 0; i < numVerts; i++)
	{
		SkinnedVert Temp;
		Temp.pos = XMFLOAT4(Verts[i * 4], Verts[i * 4 + 1], Verts[i * 4 + 2], Verts[i * 4 + 3]);
		Temp.UVW = XMFLOAT4(UVs[i * 2], UVs[i * 2 + 1], 0, 0);
		Temp.Norm = XMFLOAT4(Norms[i * 4], Norms[i * 4 + 1], Norms[i * 4 + 2], Norms[i * 4 + 3]);
		Temp.Weights = XMFLOAT4(BoneWeights[i][0], BoneWeights[i][1], BoneWeights[i][2], BoneWeights[i][3]);
		Temp.Indices = XMINT4(BoneIndices[i][0] + 1, BoneIndices[i][1] + 1, BoneIndices[i][2] + 1, BoneIndices[i][3] + 1);
		SkinnedVertexBuffer[i] = Temp;
	}

	ModelContext = new RenderContext(devResources, PlaneContext, CleanupPlaneContext, false);
	ModelMesh = new RenderMesh(CleanupTexturedShape);
	ModelMesh->m_indexCount = whatever::GetIndCount();
	ModelShape = new RenderShape(devResources, *ModelMesh, *planeContext, mat, sphere(), SkinnedShape, CleanSkinnedUpdates, SkinnedUpdate);

	ModelMesh->m_indexCount = numIndices;

	BufferData = { 0 };
	BufferData.pSysMem = IndexBuffer;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;

	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(short) * ModelMesh->m_indexCount, D3D11_BIND_INDEX_BUFFER);
	Device->CreateBuffer(&constBuffDesc, &BufferData, ModelShape->Mesh.m_indexBuffer.GetAddressOf());;

	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(BoxSkinnedConstBuff), D3D11_BIND_CONSTANT_BUFFER);
	auto Buffer11 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, nullptr, Buffer11->GetAddressOf());
	ModelMesh->MeshData.push_back(Buffer11);

	BufferData = { 0 };
	BufferData.pSysMem = SkinnedVertexBuffer;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(SkinnedVert) * numVerts, D3D11_BIND_VERTEX_BUFFER);
	//constBuffDesc = CD3D11_BUFFER_DESC(sizeof(VertexPositionUVWNorm) * numVerts, D3D11_BIND_VERTEX_BUFFER);
	auto Buffer10 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, &BufferData, Buffer10->GetAddressOf());
	ModelMesh->MeshData.push_back(Buffer10);

	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(ViewProj), D3D11_BIND_CONSTANT_BUFFER);
	auto Buffer12 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, nullptr, Buffer12->GetAddressOf());
	ModelContext->ContextData.push_back(Buffer12);

	auto SampleState1 = new Microsoft::WRL::ComPtr<ID3D11SamplerState>();
	Device->CreateSamplerState(&samplerDesc, SampleState1->GetAddressOf());
	ModelMesh->MeshData.push_back(SampleState1);

	auto SRV1 = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();
	const size_t size1 = strlen("../Resources/TestCube.dds") + 1;
	wText = new wchar_t[size1];
	mbstowcs_s(&empty, wText, size_t(size1), "../Resources/TestCube.dds", size_t(size1));
	CreateDDSTextureFromFile(Device, wText, nullptr, SRV1->GetAddressOf(), 0);
	wText = NULL;
	delete wText;
	ModelMesh->MeshData.push_back(SRV1);

	auto ShapeData1 = new BoxSkinnedConstBuff;
	ShapeData1->worldMatrix = ModelShape->WorldMat;
	XMStoreFloat4x4(&ShapeData1->boneOffsets[0], XMMatrixIdentity());
	int numBones = whatever::GetBoneCount();
	int numKeyframes = whatever::GetKeyFrameCount();
	float** boneMats = whatever::GetBoneBindMat();
	float** keyFrames = whatever::GetBoneAnimationKeyFrames();
	XMFLOAT4X4** allBoneMats = new XMFLOAT4X4*[numBones];
	Animation anim1 = Animation();
	for (int i = 0; i < numBones; i++)
	{
		allBoneMats[i] = new XMFLOAT4X4[numKeyframes + 1];
		ShapeData1->boneOffsets[i + 1] = XMFLOAT4X4(boneMats[i][0],  boneMats[i][1],  boneMats[i][2],  boneMats[i][3],
													boneMats[i][4],  boneMats[i][5],  boneMats[i][6],  boneMats[i][7],
													boneMats[i][8],  boneMats[i][9],  boneMats[i][10], boneMats[i][11],
													boneMats[i][12], boneMats[i][13], boneMats[i][14], boneMats[i][15]);
		allBoneMats[i][0] = XMFLOAT4X4(boneMats[i][0],  boneMats[i][1],  boneMats[i][2],  boneMats[i][3],
									   boneMats[i][4],  boneMats[i][5],  boneMats[i][6],  boneMats[i][7],
									   boneMats[i][8],  boneMats[i][9],  boneMats[i][10], boneMats[i][11],
									   boneMats[i][12], boneMats[i][13], boneMats[i][14], boneMats[i][15]);

		for(int j = 0; j < numKeyframes; j++)
		{
			allBoneMats[i][j + 1] = XMFLOAT4X4(keyFrames[i][j * 16 + 0],  keyFrames[i][j * 16 + 1],  keyFrames[i][j * 16 + 2],  keyFrames[i][j * 16 + 3],
											   keyFrames[i][j * 16 + 4],  keyFrames[i][j * 16 + 5],  keyFrames[i][j * 16 + 6],  keyFrames[i][j * 16 + 7],
											   keyFrames[i][j * 16 + 8],  keyFrames[i][j * 16 + 9],  keyFrames[i][j * 16 + 10], keyFrames[i][j * 16 + 11],
											   keyFrames[i][j * 16 + 12], keyFrames[i][j * 16 + 13], keyFrames[i][j * 16 + 14], keyFrames[i][j * 16 + 15]);
		}
	}
	ModelShape->ShapeData.push_back(ShapeData1);
	ModelShape->ShapeData.push_back(allBoneMats);
	ModelShape->ShapeData.push_back(new int(numKeyframes + 1));
	ModelShape->ShapeData.push_back(new int(0));
	ModelShape->ShapeData.push_back(new int(0));
	ModelShape->ShapeData.push_back(new int(numBones));
	ModelShape->ShapeData.push_back(new bool(false));

	Device->CreateVertexShader(&BasicLitSkinningVertShader, ARRAYSIZE(BasicLitSkinningVertShader), NULL, ModelContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&BasicLightPixelShader, ARRAYSIZE(BasicLightPixelShader), NULL, ModelContext->m_pixelShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UVW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "INDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(vertexDesc2, ARRAYSIZE(vertexDesc2), &BasicLitSkinningVertShader, ARRAYSIZE(BasicLitSkinningVertShader), ModelContext->m_inputLayout.GetAddressOf());

	//ModelContext->AddChild(ModelShape);
	//ModelContext->AddChild(planeContext);
	//ModelContext->AddChild(planeShape);

	planeContext->AddChild(planeShape);
	planeContext->AddChild(ModelContext);
	planeContext->AddChild(ModelShape);
	Set = RenderSet();
	Set.SetHead(planeContext);

	CurrCamera = new Camera;
	CurrCamera->init(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
	timer.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	auto swapChain = devResources->GetSwapChain();
	auto dContext = devResources->GetD3DDeviceContext();
	auto targetView = devResources->GetBackBufferRenderTargetView();
	auto viewport = devResources->GetScreenViewport();
	timer.Signal();
	float delta = (float)timer.Delta();

	CurrCamera->update(delta);
	ModelShape->Update(delta);

	dContext->OMSetRenderTargets(1, &targetView, NULL);
	dContext->RSSetViewports(1, &viewport);
	FLOAT color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	dContext->ClearRenderTargetView(targetView, color);

	Renderer::Render(&Set);
	swapChain->Present(0, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	LightBuff.Reset();
	devResources->checkResources();
	delete planeContext;
	delete planeShape;
	delete planeMesh;
	delete ModelContext;
	delete ModelShape;
	delete ModelMesh;
	devResources->cleanup();
	UnregisterClass( L"DirectXApplication", application );
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************//
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//