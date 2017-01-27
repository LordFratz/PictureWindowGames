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
using namespace DirectX;
#include "../FBXExporter/IncludeMe.h"
#include "DeviceResources.h"
#include "Renderer.h"
#include "DDSTextureLoader.h"
#define BACKBUFFER_WIDTH	800
#define BACKBUFFER_HEIGHT	600

//define 1 for bear, 0 for box, 2 for Mage
#define LOADED_BEAR 0


//You shouldn't need to touch this at all to do testing
//define 0 for no shadows, 1 for shadows
#define SHADOWS 1

struct ViewProj
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4 cameraPos;
};

struct AnimInstances
{
	XMFLOAT4X4 instances[3];
};

struct DirectionalLight
{
	XMFLOAT4 dLightPos;
	XMFLOAT4 dLightColor;
	XMFLOAT4 dLightDir;
};

struct VertexPositionColor
{
	XMFLOAT4 pos;
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

struct TangentSkinnedVert
{
	XMFLOAT4 pos;
	XMFLOAT4 UVW;
	XMFLOAT4 Norm;
	XMFLOAT4 Weights;
	XMINT4   Indices;
	XMFLOAT4 Tangents;
};

struct BoxSkinnedConstBuff
{
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 boneOffsets[38];
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
		float cameraSpeed = 15.0f * delta; // * a delta time when time is added
		float cameraRotateSpeed = 3.0f * cameraSpeed;
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
static Microsoft::WRL::ComPtr<ID3D11Buffer> InstanceBuff;
static Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterWireState;
static Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterState;

static Microsoft::WRL::ComPtr<ID3D11Texture2D> theShadowMap;
static Microsoft::WRL::ComPtr<ID3D11Texture2D> theEmptyMap;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> shadowRTV;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
static Microsoft::WRL::ComPtr<ID3D11Buffer> streamedOutput;
static Microsoft::WRL::ComPtr<ID3D11PixelShader> depthPS;
static Microsoft::WRL::ComPtr<ID3D11GeometryShader> nullSOGS;
static Microsoft::WRL::ComPtr<ID3D11Buffer> lightViewBuff;
static Microsoft::WRL::ComPtr<ID3D11SamplerState> clampSamp;
static Microsoft::WRL::ComPtr<ID3D11VertexShader> passVS;
static Microsoft::WRL::ComPtr<ID3D11PixelShader> passPS;
static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> emptySRV;
static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> emptyRTV;
static ID3D11InputLayout *nextLayout;



static ViewProj lightViewPoint;

static XMMATRIX SingleInstanceWorld;
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

	RenderShape* SphereShapething = nullptr;
	RenderMesh* SphereMeshthing = nullptr;

	RenderContext* SphereContext = nullptr;
	//RenderShape * SphereShapeOld = nullptr;
	RenderMesh* SphereMesh = nullptr;

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
	AnimInstances animInstances;
	//added for camera
	bool wire = false;
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
	void ModelGeoInstancedShadowContext(RenderNode& rNode)
	{
		auto Node = &(RenderContext&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();

		context->IASetInputLayout(Node->m_inputLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetShader(Node->m_vertexShader.Get(), nullptr, 0);
		context->GSSetShader(nullSOGS.Get(), nullptr, 0);
		context->PSSetShader(passPS.Get(), nullptr, 0);
		auto ContextSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->ContextData[0];
		context->UpdateSubresource(ContextSubresource1->Get(), 0, NULL, &CurrCamera->cameraData, 0, 0);
		context->VSSetConstantBuffers(1, 1, ContextSubresource1->GetAddressOf());




		//context->GSSetShader(Node->m_geometryShader.Get(), nullptr, 0);

		//context->GSSetConstantBuffers(1, 1, ContextSubresource1->GetAddressOf());
		//context->GSSetConstantBuffers(5, 1, InstanceBuff.GetAddressOf());


		//yet to be used

	}

	void SkinnedGeoInstancedShadowShape(RenderNode &rNode)
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
		context->SOSetTargets(1, streamedOutput.GetAddressOf(), &offset);
		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
		ID3D11Buffer* noVB[] = { NULL };
		UINT noOffset[] = { 0 };
		UINT noStride[] = { 0 };
		context->SOSetTargets(1, noVB, noOffset);
		
		context->ClearDepthStencilView(Node->m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1, 1);

		//Pass 0 complete

		context->IASetInputLayout(nextLayout);
		context->VSSetShader(passVS.Get(), nullptr, 0);
		context->GSSetShader(Node->Context.m_geometryShader.Get(), nullptr, 0);
		context->PSSetShader(depthPS.Get(), nullptr, 0);
		stride = sizeof(VertexPositionUVWNorm);
		context->IASetVertexBuffers(0, 1, streamedOutput.GetAddressOf(), &stride, &offset);
		context->OMSetRenderTargets(1, shadowRTV.GetAddressOf(), Node->m_deviceResources->GetDepthStencilView());
		auto ContextSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Context.ContextData[0];
		context->GSSetConstantBuffers(1, 1, ContextSubresource1->GetAddressOf());
		context->GSSetConstantBuffers(5, 1, InstanceBuff.GetAddressOf());
		context->GSSetConstantBuffers(0, 1, ShapeSubresource1->GetAddressOf());
		context->GSSetConstantBuffers(7, 1, lightViewBuff.GetAddressOf());
		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
		context->ClearDepthStencilView(Node->m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1, 1);


		 //Pass 1 complete

		auto targetView = Node->m_deviceResources->GetBackBufferRenderTargetView();
		context->OMSetRenderTargets(1, &targetView, Node->m_deviceResources->GetDepthStencilView());
		context->PSSetShader(Node->Context.m_pixelShader.Get(), nullptr, 0);
		context->PSSetConstantBuffers(2, 1, LightBuff.GetAddressOf());
		auto Sampler = (Microsoft::WRL::ComPtr<ID3D11SamplerState>*)Node->Mesh.MeshData[2];
		context->PSSetSamplers(0, 1, Sampler->GetAddressOf());
		context->PSSetSamplers(1, 1, clampSamp.GetAddressOf());
		context->PSSetShaderResources(5, 1, shadowSRV.GetAddressOf());


#if LOADED_BEAR == 2
		ID3D11ShaderResourceView* ShaderTextures[2] = { ((Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[3])->Get() , ((Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[4])->Get() };
		context->PSSetShaderResources(0, 2, ShaderTextures);
#else
		auto Texture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[3];
		context->PSSetShaderResources(0, 1, Texture->GetAddressOf());
#endif
		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
		context->PSSetShaderResources(5, 1, emptySRV.GetAddressOf());
		context->IASetVertexBuffers(0, 1, noVB, noStride, noOffset);
	}

	void ModelGeoInstancedContext(RenderNode& rNode)
	{
		auto Node = &(RenderContext&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();
		context->IASetInputLayout(Node->m_inputLayout.Get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->VSSetShader(Node->m_vertexShader.Get(), nullptr, 0);
		context->GSSetShader(Node->m_geometryShader.Get(), nullptr, 0);
		context->PSSetShader(Node->m_pixelShader.Get(), nullptr, 0);
		auto ContextSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->ContextData[0];
		context->UpdateSubresource(ContextSubresource1->Get(), 0, NULL, &CurrCamera->cameraData, 0, 0);
		context->VSSetConstantBuffers(1, 1, ContextSubresource1->GetAddressOf());
		context->PSSetConstantBuffers(2, 1, LightBuff.GetAddressOf());
		context->GSSetConstantBuffers(1, 1, ContextSubresource1->GetAddressOf());
		context->GSSetConstantBuffers(5, 1, InstanceBuff.GetAddressOf());
	}

	void SkinnedGeoInstancedShape(RenderNode &rNode)
	{
		auto Node = &(RenderShape&)rNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();

		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[0];
		context->UpdateSubresource(ShapeSubresource1->Get(), 0, NULL, (BoxSkinnedConstBuff*)Node->ShapeData[0], 0, 0);
		context->VSSetConstantBuffers(0, 1, ShapeSubresource1->GetAddressOf());

		context->GSSetConstantBuffers(0, 1, ShapeSubresource1->GetAddressOf());

		auto vertexBuffer = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[1];
#if LOADED_BEAR == 2
		UINT stride = sizeof(TangentSkinnedVert);
#else
		UINT stride = sizeof(SkinnedVert);
#endif
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer->GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(Node->Mesh.m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		auto Sampler = (Microsoft::WRL::ComPtr<ID3D11SamplerState>*)Node->Mesh.MeshData[2];
		context->PSSetSamplers(0, 1, Sampler->GetAddressOf());
#if LOADED_BEAR == 2
		ID3D11ShaderResourceView* ShaderTextures[3] = { ((Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[3])->Get() , ((Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[4])->Get() , ((Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[5])->Get() };
		context->PSSetShaderResources(0, 3, ShaderTextures);
#else
		auto Texture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)Node->Mesh.MeshData[3];
		context->PSSetShaderResources(0, 1, Texture->GetAddressOf());
#endif
		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
	}

	void CleanupTexturedNormSpecShape(std::vector<void*> toClean)
	{
		auto ShapeSubresource1 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[0];
		ShapeSubresource1->Reset();
		auto ShapeSubresource2 = (Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[1];
		ShapeSubresource2->Reset();
		auto Sampler = (Microsoft::WRL::ComPtr<ID3D11SamplerState>*)toClean[2];
		Sampler->Reset();
		auto Texture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)toClean[3];
		Texture->Reset();
		auto NormalTexture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)toClean[4];
		NormalTexture->Reset();
		auto SpecTexture = (Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>*)toClean[5];
		SpecTexture->Reset();
	}

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
		context->GSSetShader(nullptr, nullptr, 0);
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
		context->PSSetSamplers(1, 1, clampSamp.GetAddressOf());
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

	static std::vector<XMFLOAT4X4> Whatchamacallit = std::vector<XMFLOAT4X4>();

	void ProperSkinnedUpdate(RenderShape &Node, float delta)
	{
		auto bufferData = (BoxSkinnedConstBuff*)Node.ShapeData[0];
		auto interpolator = (Interpolator*)Node.ShapeData[1];
		auto skeleton = (Skeleton*)Node.ShapeData[2];
		auto anim = (Animation*)Node.ShapeData[3];

		interpolator->Update(delta);
		auto data = skeleton->getBoneOffsets(interpolator->CurrFrame, XMLoadFloat4x4(&Node.WorldMat));
		bufferData->worldMatrix = Node.WorldMat;
		for(int i = 0; i < interpolator->animation->bones.size(); i++)
		{
			bufferData->boneOffsets[i] = data[i];
			//XMMATRIX temp = XMLoadFloat4x4(&data[i]);

			if (LOADED_BEAR == 1) {
				Whatchamacallit.push_back(XMFLOAT4X4());
				XMStoreFloat4x4(&Whatchamacallit[i], XMMatrixScaling(2.0f, 2.0f, 2.0f) *skeleton->Bones[i].getLocal() * SingleInstanceWorld);
				//Whatchamacallit[i]._42 = -Whatchamacallit[i]._42;
			}
			else {
				Whatchamacallit.push_back(XMFLOAT4X4());
				XMStoreFloat4x4(&Whatchamacallit[i], XMMatrixScaling(0.1f, 0.1f, 0.1f) * skeleton->Bones[i].getLocal() * SingleInstanceWorld);
				//Whatchamacallit[i]._42 = -Whatchamacallit[i]._42;
			}
		}
		*(BoxSkinnedConstBuff*)Node.ShapeData[0] = *bufferData;
		delete[] data;
	}

	void CleanProperSkinnedUpdate(vector<void*> toClean)
	{
		delete toClean[0];
		delete toClean[1];
		delete toClean[2];
		delete toClean[3];
	}

	void ProperBlendedSkinnedUpdate(RenderShape &Node, float delta)
	{
		auto bufferData = (BoxSkinnedConstBuff*)Node.ShapeData[0];
		auto Blender = (BlenderDataStorage*)Node.ShapeData[1];
		auto skeleton = (Skeleton*)Node.ShapeData[2];
		auto anim = (Animation*)Node.ShapeData[3];

		auto CurrFrame = Blender->Update(delta);
		auto data = skeleton->getBoneOffsets(CurrFrame, XMLoadFloat4x4(&Node.WorldMat));
		bufferData->worldMatrix = Node.WorldMat;
		XMStoreFloat4x4(&bufferData->boneOffsets[0], XMMatrixIdentity());
		for (int i = 0; i < Blender->Animations[0].bones.size(); i++)
		{
			bufferData->boneOffsets[i+1] = data[i];
			//XMMATRIX temp = XMLoadFloat4x4(&data[i]);
			//Whatchamacallit.push_back(XMFLOAT4X4());
			//XMStoreFloat4x4(&Whatchamacallit[i], SingleInstanceWorld * skeleton->Bones[i].getLocal());
			//Whatchamacallit[i]._42 = -Whatchamacallit[i]._42;
			if (LOADED_BEAR == 1) {
				Whatchamacallit.push_back(XMFLOAT4X4());
				XMStoreFloat4x4(&Whatchamacallit[i], XMMatrixScaling(2.0f, 2.0f, 2.0f) *skeleton->Bones[i].getLocal() * SingleInstanceWorld);
				//Whatchamacallit[i]._42 = -Whatchamacallit[i]._42;
			}
			else {
				Whatchamacallit.push_back(XMFLOAT4X4());
				XMStoreFloat4x4(&Whatchamacallit[i], XMMatrixScaling(0.1f, 0.1f, 0.1f) * skeleton->Bones[i].getLocal() * SingleInstanceWorld);
				//Whatchamacallit[i]._42 = -Whatchamacallit[i]._42;
			}
		}
		*(BoxSkinnedConstBuff*)Node.ShapeData[0] = *bufferData;
		delete[] data;
	}

	void CleanProperBlendedSkinnedUpdate(vector<void*> toClean)
	{
		delete toClean[0];
		delete toClean[1];
		delete toClean[2];
		//animations
		delete toClean[3];
		delete toClean[4];
		delete toClean[5];
		delete toClean[6];
	}

	void SphereShape(RenderNode& RNode) {
		auto Node = &(RenderShape&)RNode;
		auto context = Node->m_deviceResources->GetD3DDeviceContext();

		auto VertShader = (Microsoft::WRL::ComPtr<ID3D11VertexShader>*)Node->Mesh.MeshData[0];
		context->VSSetShader(VertShader->Get(), nullptr, 0);
		auto PixelShader = (Microsoft::WRL::ComPtr<ID3D11PixelShader>*)Node->Mesh.MeshData[1];
		context->PSSetShader(PixelShader->Get(), nullptr, 0);
		auto InputLayout = (Microsoft::WRL::ComPtr<ID3D11InputLayout>*)Node->Mesh.MeshData[2];
		context->IASetInputLayout(InputLayout->Get());

		auto instCount = (int*)Node->Mesh.MeshData[4];

		D3D11_SUBRESOURCE_DATA BufferData = { 0 };
		BufferData.pSysMem = Whatchamacallit.data();
		BufferData.SysMemPitch = 0;
		BufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC constBuffDesc = CD3D11_BUFFER_DESC(sizeof(XMFLOAT4X4) * *instCount, D3D11_BIND_VERTEX_BUFFER);
		auto Buffer3 = Microsoft::WRL::ComPtr<ID3D11Buffer>();
		Node->m_deviceResources->GetD3DDevice()->CreateBuffer(&constBuffDesc, &BufferData, Buffer3.GetAddressOf());

		ID3D11Buffer* buffers[2] = { ((Microsoft::WRL::ComPtr<ID3D11Buffer>*)Node->Mesh.MeshData[3])->Get(), Buffer3.Get() };//
		UINT stride[2] = { sizeof(VertexPositionUVWNorm), sizeof(XMFLOAT4X4) };
		UINT offset[2] = { 0, 0 };
		context->IASetVertexBuffers(0, 2, buffers, stride, offset); //breaking here

		context->IASetIndexBuffer(Node->Mesh.m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		Buffer3.Reset();


		context->DrawIndexedInstanced(Node->Mesh.m_indexCount, *instCount, 0, 0, 0);
	}

	void CleanSphereShape(std::vector<void*> toClean) {
		((Microsoft::WRL::ComPtr<ID3D11VertexShader>*)toClean[0])->Reset();
		((Microsoft::WRL::ComPtr<ID3D11PixelShader>*)toClean[1])->Reset();
		((Microsoft::WRL::ComPtr<ID3D11InputLayout>*)toClean[2])->Reset();
		((Microsoft::WRL::ComPtr<ID3D11Buffer>*)toClean[3])->Reset();
		delete toClean[4];
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
	VertexPositionColor* CreateD20Verts() {
		float t = (1.0f + sqrt(5.0f)) / 2.0f;
		VertexPositionColor* Mesh = new VertexPositionColor[12];
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
			XMStoreFloat3(&(Mesh[i].color), XMVectorSet(1.0f, 0.0f, 1.0f, 1.0f));
		}
		return Mesh;
	}

	unsigned short* CreateD20Inds() {
		unsigned short* Ind = new unsigned short[60];
		Ind[0] = 0; Ind[1] = 11; Ind[2] = 5;
		Ind[3] = 0; Ind[4] = 5; Ind[5] = 1;
		Ind[6] = 0; Ind[7] = 1; Ind[8] = 7;
		Ind[9] = 0; Ind[10] = 7; Ind[11] = 10;
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
		return Ind;
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



	std::vector<uint8_t> VSData;
	std::vector<uint8_t> PSData;
	bool thing = ShaderLoader::LoadShader(VSData, "BasicToLightVertexShader.cso");
	thing = ShaderLoader::LoadShader(PSData, "BasicLightPixelShader.cso");
	Device->CreateVertexShader(&VSData[0], VSData.size(), NULL, planeContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&PSData[0], PSData.size(), NULL, planeContext->m_pixelShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UVW" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORM" , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	Device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &VSData[0], VSData.size(), planeContext->m_inputLayout.GetAddressOf());

    nextLayout = planeContext->m_inputLayout.Get();

	//Sphere initializations :: shader initializations/layouts

	//Things known to need change:
	//			-needs a custom context/cleanup functions (PlaneContext & PlaneShape) that correctly handle DrawInstanceIndexed()...
	//          -BasicVertexShader and BasicPixelShader will need to be modified to properly match inputs, as well as code for handling instanced transforms (instancePos X boneOffset X world etc)
	//          -Vertex Layout will need new elements flagged with D3D11_INPUT_PER_INSTANCE_DATA (per instance transforms ie, the positions from model origin of each bone)
	//              -Also make sure to use 1 for the start slot index, as a separate buffer houses per instance data ex. "POSITION", 1, DXGI_FORMAT_....
	//**************************************************************
	//SphereContext = new RenderContext(devResources, PlaneContext, CleanupPlaneContext, false);
	//SphereMesh = new RenderMesh(CleanupPlaneShape);
	//SphereShape = new RenderShape(devResources, *SphereMesh, *SphereContext, mat, sphere(), PlaneShape, NoCleanup);
	//
	//std::vector<uint8_t> VSDataSphere;
	//std::vector<uint8_t> PSDataSphere;
	//thing = ShaderLoader::LoadShader(VSDataSphere, "BasicVertexShader.cso");
	//thing = ShaderLoader::LoadShader(PSDataSphere, "BasicPixelShader.cso");
	//Device->CreateVertexShader(&VSDataSphere[0], VSDataSphere.size(), NULL, SphereContext->m_vertexShader.GetAddressOf());
	//Device->CreatePixelShader(&PSDataSphere[0], PSDataSphere.size(), NULL, SphereContext->m_pixelShader.GetAddressOf());
	//static const D3D11_INPUT_ELEMENT_DESC SphereVertexDesc[] =
	//{
	//{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};
	//Device->CreateInputLayout(SphereVertexDesc, ARRAYSIZE(SphereVertexDesc), &VSDataSphere[0], VSDataSphere.size(), SphereContext->m_inputLayout.GetAddressOf());
	//*************************************************************
	//end Sphere initializations and shader initializations/layouts


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

	//Sphere initializations:: VertexBuffer/IndexBuffer
	// These should be good to go when you are ready
	// However, another vertex buffer will be needed to store the instance data, which can be updated via subresource management in the custom context/update/cleanup functions
	//*******************************************************************************
	//D3D11_SUBRESOURCE_DATA BufferDataSphere = { 0 };
	//VertexPositionColor* tempSphereVertBuffer = GenerateObject::CreateD20Verts();
	//BufferDataSphere.pSysMem = tempSphereVertBuffer;
	//BufferDataSphere.SysMemPitch = 0;
	//BufferDataSphere.SysMemSlicePitch = 0;
	//constBuffDesc = CD3D11_BUFFER_DESC(sizeof(VertexPositionColor) * 12, D3D11_BIND_VERTEX_BUFFER);
	//auto BufferSphere = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	//Device->CreateBuffer(&constBuffDesc, &BufferDataSphere, BufferSphere->GetAddressOf());
	//SphereMesh->MeshData.push_back(BufferSphere);
	//
	//
	//unsigned short* sphereIndices = GenerateObject::CreateD20Inds();
	//SphereMesh->m_indexCount = 60;
	//BufferDataSphere = { 0 };
	//BufferDataSphere.pSysMem = sphereIndices;
	//BufferDataSphere.SysMemPitch = 0;
	//BufferDataSphere.SysMemSlicePitch = 0;
	//constBuffDesc = CD3D11_BUFFER_DESC(sizeof(unsigned short) * 60, D3D11_BIND_INDEX_BUFFER);
	//Device->CreateBuffer(&constBuffDesc, &BufferDataSphere, SphereMesh->m_indexBuffer.GetAddressOf());
	//end Sphere initializations :: VertexBuffer/IndexBuffer

	//Light initializations
	XMStoreFloat4(&dynaLight.dLightColor, XMVectorSet(1.0f, 1.0f, 0.0f, 1.0f));//XMVectorSet(1.0f, 0.98f, 0.804f, 1.0f));
	XMStoreFloat4(&dynaLight.dLightDir, XMVectorSet(-0.707f, -0.5f, -0.21f, 1.0f));
	XMStoreFloat4(&dynaLight.dLightPos, XMVectorSet(-15.0f, 10.0f, 2.0f, 1.0f));

	D3D11_SUBRESOURCE_DATA BufferData2 = { 0 };
	BufferData2.pSysMem = &dynaLight;
	BufferData2.SysMemPitch = 0;
	BufferData2.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(DirectionalLight), D3D11_BIND_CONSTANT_BUFFER);
	//auto Buffer4 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, &BufferData2, LightBuff.GetAddressOf());
	//end light initializations

	//Prepass initializations
	
    //Creating the Shadow Map texture
	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
	shadowMapDesc.Width = BACKBUFFER_WIDTH;
	shadowMapDesc.Height = BACKBUFFER_HEIGHT;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	shadowMapDesc.CPUAccessFlags = 0;
	shadowMapDesc.MiscFlags = 0;
	Device->CreateTexture2D(&shadowMapDesc, nullptr, theShadowMap.GetAddressOf());
	Device->CreateTexture2D(&shadowMapDesc, nullptr, theEmptyMap.GetAddressOf());
	//binding shadow map as a render target
	D3D11_RENDER_TARGET_VIEW_DESC shadowRTVDesc;
	shadowRTVDesc.Format = shadowMapDesc.Format;
	shadowRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	shadowRTVDesc.Texture2D.MipSlice = 0;
	Device->CreateRenderTargetView(theShadowMap.Get(), &shadowRTVDesc, shadowRTV.GetAddressOf());
	Device->CreateRenderTargetView(theEmptyMap.Get(), &shadowRTVDesc, emptyRTV.GetAddressOf());

	//binding shadow map as a shader resource
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc;
	shadowSRVDesc.Format = shadowMapDesc.Format;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	Device->CreateShaderResourceView(theShadowMap.Get(), &shadowSRVDesc, shadowSRV.GetAddressOf());
	Device->CreateShaderResourceView(theEmptyMap.Get(), &shadowSRVDesc, emptySRV.GetAddressOf());
	//build view and projection matricies from the viewpoint of the light source ORTHOGRAPHIC due to directional light
	XMStoreFloat4x4(&lightViewPoint.view, XMMatrixInverse(0, XMMatrixLookToLH(XMLoadFloat4(&dynaLight.dLightPos), XMLoadFloat4(&dynaLight.dLightDir), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	XMStoreFloat4(&lightViewPoint.cameraPos, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
	XMStoreFloat4x4(&lightViewPoint.projection, XMMatrixOrthographicLH(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, 0.01f, 100.0f));

	D3D11_SUBRESOURCE_DATA BufferDataShadow = { 0 };
	BufferDataShadow.pSysMem = &lightViewPoint;
	BufferDataShadow.SysMemPitch = 0;
	BufferDataShadow.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(ViewProj), D3D11_BIND_CONSTANT_BUFFER);
	Device->CreateBuffer(&constBuffDesc, &BufferDataShadow, lightViewBuff.GetAddressOf());



	
    //create new depth render to texture pixel shader
	std::vector<uint8_t> shadowPSData;
	ShaderLoader::LoadShader(shadowPSData, "DepthMapPS.cso");
	Device->CreatePixelShader(&shadowPSData[0], shadowPSData.size(), NULL, depthPS.GetAddressOf());

	//create null Geometry Shader with SO enabled
	std::vector<uint8_t> nullGSData;
	ShaderLoader::LoadShader(nullGSData, "VSforNullGS.cso");

	
	D3D11_SO_DECLARATION_ENTRY SODecl[] =
	{
		{ 0, "SV_POSITION", 0, 0, 4, 0 },   
		{ 0, "UVW", 0, 0, 4, 0 },     
		{ 0, "NORM", 0, 0, 4, 0 },
	};
	Device->CreateGeometryShaderWithStreamOutput(&nullGSData[0], nullGSData.size(), SODecl, ARRAYSIZE(SODecl), NULL, 0, 0, NULL, nullSOGS.GetAddressOf());

	//Clamped sampler for shadow map
	D3D11_SAMPLER_DESC clampSamplerDesc;
	clampSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	clampSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.MipLODBias = 0.0f;
	clampSamplerDesc.MaxAnisotropy = 1;
	clampSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	clampSamplerDesc.BorderColor[0] = 1.0f;
	clampSamplerDesc.BorderColor[1] = 1.0f;
	clampSamplerDesc.BorderColor[2] = 1.0f;
	clampSamplerDesc.BorderColor[3] = 1.0f;
	clampSamplerDesc.MinLOD = -FLT_MAX;
	clampSamplerDesc.MaxLOD = FLT_MAX;
	Device->CreateSamplerState(&clampSamplerDesc, clampSamp.GetAddressOf());


	//pass thru VS
	std::vector<uint8_t> passVSData;
	ShaderLoader::LoadShader(passVSData, "InstancePassVS.cso");
	Device->CreateVertexShader(&passVSData[0], passVSData.size(), NULL, passVS.GetAddressOf());

	//blue ps
	std::vector<uint8_t> passPSData;
	ShaderLoader::LoadShader(passPSData, "Trivial_PS.cso");
	Device->CreatePixelShader(&passPSData[0], passPSData.size(), NULL, passPS.GetAddressOf());
	//end Prepass initializations



	//instance initializations
	if (LOADED_BEAR == 1)
	{
		XMStoreFloat4x4(&(animInstances.instances[0]), XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.02f, 0.02f, 0.02f), XMMatrixTranslation(0.0f, 0.0f, 0.0f))));
		XMStoreFloat4x4(&(animInstances.instances[1]), XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.02f, 0.02f, 0.02f), XMMatrixTranslation(5.0f, 0.0f, 1.0f))));
		XMStoreFloat4x4(&(animInstances.instances[2]), XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.02f, 0.02f, 0.02f), XMMatrixTranslation(-5.0f, 0.0f, -1.0f))));
	}
	else if(LOADED_BEAR == 0)
	{
		XMStoreFloat4x4(&(animInstances.instances[0]), XMMatrixTranspose(XMMatrixTranslation(0.0f, 0.0f, 0.0f)));
		XMStoreFloat4x4(&(animInstances.instances[1]), XMMatrixTranspose(XMMatrixTranslation(5.0f, 0.0f, 1.0f)));
		XMStoreFloat4x4(&(animInstances.instances[2]), XMMatrixTranspose(XMMatrixTranslation(-5.0f, 0.0f, -1.0f)));
	}
	else if (LOADED_BEAR == 2) {
		XMStoreFloat4x4(&(animInstances.instances[0]), XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.7f, 0.7f, 0.7f), XMMatrixTranslation(0.0f, 0.0f, 0.0f))));
		XMStoreFloat4x4(&(animInstances.instances[1]), XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.7f, 0.7f, 0.7f), XMMatrixTranslation(5.0f, 0.0f, 1.0f))));
		XMStoreFloat4x4(&(animInstances.instances[2]), XMMatrixTranspose(XMMatrixMultiply(XMMatrixScaling(0.7f, 0.7f, 0.7f), XMMatrixTranslation(-5.0f, 0.0f, -1.0f))));
	}

	SingleInstanceWorld = XMLoadFloat4x4(&animInstances.instances[0]);

	D3D11_SUBRESOURCE_DATA BufferData3 = { 0 };
	BufferData3.pSysMem = &animInstances;
	BufferData3.SysMemPitch = 0;
	BufferData3.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(AnimInstances), D3D11_BIND_CONSTANT_BUFFER);
	Device->CreateBuffer(&constBuffDesc, &BufferData3, InstanceBuff.GetAddressOf());
	//end instance initializations


	auto SampleState = new Microsoft::WRL::ComPtr<ID3D11SamplerState>();
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
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

	if (LOADED_BEAR == 1)
	{
		whatever::loadFile("../Resources/Teddy_Mesh.pwm", "../Resources/Teddy_Run.fbx");
		whatever::loadFile("../Resources/Teddy_Skeleton.pws", "../Resources/Teddy_Run.fbx");
		whatever::loadFile("../Resources/Teddy_RunAnim.pwa", "../Resources/Teddy_Run.fbx");
	}
	else if(LOADED_BEAR == 0)
	{
		whatever::loadFile("../Resources/Box_Mesh.pwm", "../Resources/Box_Jump.fbx");
		whatever::loadFile("../Resources/Box_Skeleton.pws", "../Resources/Box_Jump.fbx");
		whatever::loadFile("../Resources/Box_WalkAnim.pwa", "../Resources/Box_Walk.fbx");
	}
	else if (LOADED_BEAR == 2) {
		whatever::loadFile("../Resources/Mage_Mesh.pwm", "../Resources/BattleMageWhat.fbx");
		whatever::loadFile("../Resources/Mage_Skeleton.pws", "../Resources/BattleMageWhat.fbx");
		whatever::loadFile("../Resources/Mage_DeathAnim.pwa", "../Resources/BattleMageWhat.fbx");
	}


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

	//initialize stream out buffer
	//*******************************************
	int streamBuffSize = numVerts * sizeof(VertexPositionUVWNorm); // needs to be moved to later after load and exactly numVerts * sizeof(VertexPositionUVWNorm)
	D3D11_BUFFER_DESC streamBuffDesc = { streamBuffSize, D3D11_USAGE_DEFAULT, D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	Device->CreateBuffer(&streamBuffDesc, nullptr, streamedOutput.GetAddressOf());
	//******************************************

	int** BoneIndices = whatever::GetVertToBoneInds();
	float** BoneWeights = whatever::GetVertWeightToBoneInds();
#if LOADED_BEAR != 2
	SkinnedVert* SkinnedVertexBuffer = new SkinnedVert[numVerts];
#else
	TangentSkinnedVert* SkinnedVertexBuffer = new TangentSkinnedVert[numVerts];
	float* Tangents = whatever::GetTangents();
#endif
	for(int i = 0; i < numVerts; i++)
	{
#if LOADED_BEAR != 2
		SkinnedVert Temp;
#else
		TangentSkinnedVert Temp;
		Temp.Tangents = XMFLOAT4(Tangents[i * 4], Tangents[i * 4 + 1], Tangents[i * 4 + 2], Tangents[i * 4 + 3]);
#endif
		Temp.pos = XMFLOAT4(Verts[i * 4], Verts[i * 4 + 1], Verts[i * 4 + 2], Verts[i * 4 + 3]);
		Temp.UVW = XMFLOAT4(UVs[i * 2], UVs[i * 2 + 1], 0, 0);
		Temp.Norm = XMFLOAT4(Norms[i * 4], Norms[i * 4 + 1], Norms[i * 4 + 2], Norms[i * 4 + 3]);
		Temp.Weights = XMFLOAT4(BoneWeights[i][0], BoneWeights[i][1], BoneWeights[i][2], BoneWeights[i][3]);
		Temp.Indices = XMINT4(BoneIndices[i][0] + 1, BoneIndices[i][1] + 1, BoneIndices[i][2] + 1, BoneIndices[i][3] + 1);
		SkinnedVertexBuffer[i] = Temp;
	}
	//ModelContext = new RenderContext(devResources, PlaneContext, CleanupPlaneContext, false);
	ModelContext = new RenderContext(devResources, ModelGeoInstancedShadowContext, CleanupPlaneContext, false);
#if LOADED_BEAR == 2
	ModelMesh = new RenderMesh(CleanupTexturedNormSpecShape);
#else
	ModelMesh = new RenderMesh(CleanupTexturedShape);
#endif
	ModelMesh->m_indexCount = whatever::GetIndCount();
	ModelShape = new RenderShape(devResources, *ModelMesh, *ModelContext, mat, sphere(), SkinnedGeoInstancedShadowShape, CleanProperBlendedSkinnedUpdate, ProperBlendedSkinnedUpdate);

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
#if LOADED_BEAR != 2
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(SkinnedVert) * numVerts, D3D11_BIND_VERTEX_BUFFER);
#else
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(TangentSkinnedVert) * numVerts, D3D11_BIND_VERTEX_BUFFER);
#endif
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

	//Change when bear texture gets added
	if (LOADED_BEAR == 1)
	{
		const size_t size1 = strlen("../Resources/Teddy_D.dds") + 1;
		wText = new wchar_t[size1];
		mbstowcs_s(&empty, wText, size_t(size1), "../Resources/Teddy_D.dds", size_t(size1));
	}
	else if(LOADED_BEAR == 0)
	{
		const size_t size1 = strlen("../Resources/TestCube.dds") + 1;
		wText = new wchar_t[size1];
		mbstowcs_s(&empty, wText, size_t(size1), "../Resources/TestCube.dds", size_t(size1));
	}
	else if (LOADED_BEAR == 2) {
		const size_t size1 = strlen("../Resources/MageTexture.dds") + 1;
		wText = new wchar_t[size1];
		mbstowcs_s(&empty, wText, size_t(size1), "../Resources/MageTexture.dds", size_t(size1));
	}

	CreateDDSTextureFromFile(Device, wText, nullptr, SRV1->GetAddressOf(), 0);
	wText = NULL;
	delete wText;
	ModelMesh->MeshData.push_back(SRV1);

#if LOADED_BEAR == 2

	//Loads Normal / Specular Texture maps when the mage is being loaded in for the mage

	auto SRV2 = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();
	auto SRV3 = new Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>();

	const size_t size1 = strlen("../Resources/MageNormal.dds") + 1;
	wText = new wchar_t[size1];
	mbstowcs_s(&empty, wText, size_t(size1), "../Resources/MageNormal.dds", size_t(size1));

	CreateDDSTextureFromFile(Device, wText, nullptr, SRV2->GetAddressOf(), 0);

	ModelMesh->MeshData.push_back(SRV2);

	const size_t size3 = strlen("../Resources/MageSpecular.dds") + 1;
	wText = new wchar_t[size3];
	mbstowcs_s(&empty, wText, size_t(size3), "../Resources/MageSpecular.dds", size_t(size3));

	CreateDDSTextureFromFile(Device, wText, nullptr, SRV3->GetAddressOf(), 0);

	ModelMesh->MeshData.push_back(SRV3);

	wText = NULL;
	delete wText;
#endif

#if 0
	auto ShapeData1 = new BoxSkinnedConstBuff;
	ShapeData1->worldMatrix = ModelShape->WorldMat;
	XMStoreFloat4x4(&ShapeData1->boneOffsets[0], XMMatrixIdentity());
	int numBones = whatever::GetBoneCount();
	int numKeyframes = whatever::GetKeyFrameCount();
	float** boneMats = whatever::GetBoneBindMat();
	float** keyFrames = whatever::GetBoneAnimationKeyFrames();
	XMFLOAT4X4** allBoneMats = new XMFLOAT4X4*[numBones];
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
#else
	auto ShapeData1 = new BoxSkinnedConstBuff;
	ShapeData1->worldMatrix = ModelShape->WorldMat;
	XMStoreFloat4x4(&ShapeData1->boneOffsets[0], XMMatrixIdentity());
	int numBones = whatever::GetBoneCount();
	float** boneMats = whatever::GetBoneBindMat();
	auto parentInd = whatever::GetParentInds();
	auto animKeyframes = whatever::GetBoneAnimationKeyFrames();
	auto animtweens = whatever::GetAnimationKeyframeTweens();
	Animation* anim1 = new Animation();
	Skeleton* skele1 = new Skeleton();
	skele1->Bones.reserve(numBones);
	auto identity = XMMatrixIdentity();
	XMVECTOR blah;
	for(int i= 0; i < numBones; i++)
	{
		auto currBind = XMFLOAT4X4(boneMats[i][0], boneMats[i][1], boneMats[i][2], boneMats[i][3],
								   boneMats[i][4], boneMats[i][5], boneMats[i][6], boneMats[i][7],
								   boneMats[i][8], boneMats[i][9], boneMats[i][10], boneMats[i][11],
								   boneMats[i][12], boneMats[i][13], boneMats[i][14], boneMats[i][15]);

		XMStoreFloat4x4(&ShapeData1->boneOffsets[i + 1], XMMatrixInverse(nullptr, XMLoadFloat4x4(&currBind)));
		//ShapeData1->boneOffsets[i + 1] = currBind;

		skele1->InverseBindMats.push_back(XMLoadFloat4x4(&ShapeData1->boneOffsets[i + 1]));
		skele1->Bones.push_back(TransformNode());
		if(parentInd[i] != -1)
		{
			skele1->Bones[i].addParent(&skele1->Bones[parentInd[i]]);
		}
		skele1->Bones[i].setLocal(identity);
		anim1->bones.push_back(Bone());
		auto keyFrameCount = whatever::GetKeyFrameAmount(i);
		if(keyFrameCount > 2)
		{
			for(int j = 0; j < keyFrameCount; j++)
			{
				anim1->bones[i].frames.push_back(Keyframe());
				anim1->bones[i].frames[j].tweenTime = animtweens[i][j];
				auto tempMat = XMFLOAT4X4(animKeyframes[i][j * 16 + 0], animKeyframes[i][j * 16 + 1], animKeyframes[i][j * 16 + 2], animKeyframes[i][j * 16 + 3],
										  animKeyframes[i][j * 16 + 4], animKeyframes[i][j * 16 + 5], animKeyframes[i][j * 16 + 6], animKeyframes[i][j * 16 + 7],
										  animKeyframes[i][j * 16 + 8], animKeyframes[i][j * 16 + 9], animKeyframes[i][j * 16 + 10], animKeyframes[i][j * 16 + 11],
										  animKeyframes[i][j * 16 + 12], animKeyframes[i][j * 16 + 13], animKeyframes[i][j * 16 + 14], animKeyframes[i][j * 16 + 15]);
				XMMatrixDecompose(&blah, &anim1->bones[i].frames[j].rotation, &anim1->bones[i].frames[j].position, XMLoadFloat4x4(&tempMat));
			}
		}
		else if(keyFrameCount == 0)
		{
			anim1->bones[i].frames.push_back(Keyframe());
			anim1->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim1->bones[i].frames[0].rotation, &anim1->bones[i].frames[0].position, identity);
			anim1->bones[i].frames.push_back(Keyframe());
			anim1->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim1->bones[i].frames[1].rotation, &anim1->bones[i].frames[1].position, identity);
		}
		else
		{
			//HOW THE HECK DO YOU ONLY HAVE 1 ANIMATION FRAME?!?
			auto tempMat = XMFLOAT4X4(animKeyframes[i][0],  animKeyframes[i][1],  animKeyframes[i][2],  animKeyframes[i][3],
									  animKeyframes[i][4],  animKeyframes[i][5],  animKeyframes[i][6],  animKeyframes[i][7],
									  animKeyframes[i][8],  animKeyframes[i][9],  animKeyframes[i][10], animKeyframes[i][11],
									  animKeyframes[i][12], animKeyframes[i][13], animKeyframes[i][14], animKeyframes[i][15]);
			anim1->bones[i].frames.push_back(Keyframe());
			anim1->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim1->bones[i].frames[0].rotation, &anim1->bones[i].frames[0].position, XMLoadFloat4x4(&tempMat));
			anim1->bones[i].frames.push_back(Keyframe());
			anim1->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim1->bones[i].frames[1].rotation, &anim1->bones[i].frames[1].position, XMLoadFloat4x4(&tempMat));
		}
	}

#endif

#if LOADED_BEAR == 0
	whatever::loadFile("../Resources/Box_JumpAnim.pwa", "../Resources/Box_Jump.fbx");
#elif LOADED_BEAR == 1
	whatever::loadFile("../Resources/Teddy_IdleAnim.pwa", "../Resources/Teddy_Idle.fbx");
#elif LOADED_BEAR == 2
	whatever::loadFile("../Resources/Mage_WalkAnim.pwa", "../Resources/BattleMage_Walk.fbx");
#endif

	numBones = whatever::GetBoneCount();
	animKeyframes = whatever::GetBoneAnimationKeyFrames();
	animtweens = whatever::GetAnimationKeyframeTweens();
	Animation* anim2 = new Animation();

#if 1
	for (int i = 0; i < numBones; i++)
	{
		anim2->bones.push_back(Bone());
		auto keyFrameCount = whatever::GetKeyFrameAmount(i);
		if (keyFrameCount > 2)
		{
			for (int j = 0; j < keyFrameCount; j++)
			{
				anim2->bones[i].frames.push_back(Keyframe());
				anim2->bones[i].frames[j].tweenTime = animtweens[i][j];
				auto tempMat = XMFLOAT4X4(animKeyframes[i][j * 16 + 0], animKeyframes[i][j * 16 + 1], animKeyframes[i][j * 16 + 2], animKeyframes[i][j * 16 + 3],
					animKeyframes[i][j * 16 + 4], animKeyframes[i][j * 16 + 5], animKeyframes[i][j * 16 + 6], animKeyframes[i][j * 16 + 7],
					animKeyframes[i][j * 16 + 8], animKeyframes[i][j * 16 + 9], animKeyframes[i][j * 16 + 10], animKeyframes[i][j * 16 + 11],
					animKeyframes[i][j * 16 + 12], animKeyframes[i][j * 16 + 13], animKeyframes[i][j * 16 + 14], animKeyframes[i][j * 16 + 15]);
				XMMatrixDecompose(&blah, &anim2->bones[i].frames[j].rotation, &anim2->bones[i].frames[j].position, XMLoadFloat4x4(&tempMat));
			}
		}
		else if (keyFrameCount == 0)
		{
			anim2->bones[i].frames.push_back(Keyframe());
			anim2->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim2->bones[i].frames[0].rotation, &anim2->bones[i].frames[0].position, identity);
			anim2->bones[i].frames.push_back(Keyframe());
			anim2->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim2->bones[i].frames[1].rotation, &anim2->bones[i].frames[1].position, identity);
		}
		else
		{
			//HOW THE HECK DO YOU ONLY HAVE 1 ANIMATION FRAME?!?
			auto tempMat = XMFLOAT4X4(animKeyframes[i][0], animKeyframes[i][1], animKeyframes[i][2], animKeyframes[i][3],
				animKeyframes[i][4], animKeyframes[i][5], animKeyframes[i][6], animKeyframes[i][7],
				animKeyframes[i][8], animKeyframes[i][9], animKeyframes[i][10], animKeyframes[i][11],
				animKeyframes[i][12], animKeyframes[i][13], animKeyframes[i][14], animKeyframes[i][15]);
			anim2->bones[i].frames.push_back(Keyframe());
			anim2->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim2->bones[i].frames[0].rotation, &anim2->bones[i].frames[0].position, XMLoadFloat4x4(&tempMat));
			anim2->bones[i].frames.push_back(Keyframe());
			anim2->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim2->bones[i].frames[1].rotation, &anim2->bones[i].frames[1].position, XMLoadFloat4x4(&tempMat));
		}
	}
#endif

#if LOADED_BEAR == 0
	whatever::loadFile("../Resources/Box_IdleAnim.pwa", "../Resources/Box_Idle.fbx");
#elif LOADED_BEAR == 1
	whatever::loadFile("../Resources/Teddy_Attack1Anim.pwa", "../Resources/Teddy_Attack1.fbx");
#elif LOADED_BEAR == 2
	whatever::loadFile("../Resources/Mage_IdleAnim.pwa", "../Resources/BattleMage_Idle.fbx");
#endif

	numBones = whatever::GetBoneCount();
	animKeyframes = whatever::GetBoneAnimationKeyFrames();
	animtweens = whatever::GetAnimationKeyframeTweens();
	Animation* anim3 = new Animation();

#if 1
	for (int i = 0; i < numBones; i++)
	{
		anim3->bones.push_back(Bone());
		auto keyFrameCount = whatever::GetKeyFrameAmount(i);
		if (keyFrameCount > 2)
		{
			for (int j = 0; j < keyFrameCount; j++)
			{
				anim3->bones[i].frames.push_back(Keyframe());
				anim3->bones[i].frames[j].tweenTime = animtweens[i][j];
				auto tempMat = XMFLOAT4X4(animKeyframes[i][j * 16 + 0], animKeyframes[i][j * 16 + 1], animKeyframes[i][j * 16 + 2], animKeyframes[i][j * 16 + 3],
					animKeyframes[i][j * 16 + 4], animKeyframes[i][j * 16 + 5], animKeyframes[i][j * 16 + 6], animKeyframes[i][j * 16 + 7],
					animKeyframes[i][j * 16 + 8], animKeyframes[i][j * 16 + 9], animKeyframes[i][j * 16 + 10], animKeyframes[i][j * 16 + 11],
					animKeyframes[i][j * 16 + 12], animKeyframes[i][j * 16 + 13], animKeyframes[i][j * 16 + 14], animKeyframes[i][j * 16 + 15]);
				XMMatrixDecompose(&blah, &anim3->bones[i].frames[j].rotation, &anim3->bones[i].frames[j].position, XMLoadFloat4x4(&tempMat));
			}
		}
		else if (keyFrameCount == 0)
		{
			anim3->bones[i].frames.push_back(Keyframe());
			anim3->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim3->bones[i].frames[0].rotation, &anim3->bones[i].frames[0].position, identity);
			anim3->bones[i].frames.push_back(Keyframe());
			anim3->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim3->bones[i].frames[1].rotation, &anim3->bones[i].frames[1].position, identity);
		}
		else
		{
			//HOW THE HECK DO YOU ONLY HAVE 1 ANIMATION FRAME?!?
			auto tempMat = XMFLOAT4X4(animKeyframes[i][0], animKeyframes[i][1], animKeyframes[i][2], animKeyframes[i][3],
				animKeyframes[i][4], animKeyframes[i][5], animKeyframes[i][6], animKeyframes[i][7],
				animKeyframes[i][8], animKeyframes[i][9], animKeyframes[i][10], animKeyframes[i][11],
				animKeyframes[i][12], animKeyframes[i][13], animKeyframes[i][14], animKeyframes[i][15]);
			anim3->bones[i].frames.push_back(Keyframe());
			anim3->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim3->bones[i].frames[0].rotation, &anim3->bones[i].frames[0].position, XMLoadFloat4x4(&tempMat));
			anim3->bones[i].frames.push_back(Keyframe());
			anim3->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim3->bones[i].frames[1].rotation, &anim3->bones[i].frames[1].position, XMLoadFloat4x4(&tempMat));
		}
	}
#endif

#if LOADED_BEAR == 0
	whatever::loadFile("../Resources/Box_AttackAnim.pwa", "../Resources/Box_Attack.fbx");
#elif LOADED_BEAR == 1
	whatever::loadFile("../Resources/Teddy_Attack2Anim.pwa", "../Resources/Teddy_Attack2.fbx");
#elif LOADED_BEAR == 2
	whatever::loadFile("../Resources/Mage_RunAnim.pwa", "../Resources/BattleMage_Run.fbx");
#endif

	numBones = whatever::GetBoneCount();
	animKeyframes = whatever::GetBoneAnimationKeyFrames();
	animtweens = whatever::GetAnimationKeyframeTweens();
	Animation* anim4 = new Animation();

#if 1
	for (int i = 0; i < numBones; i++)
	{
		anim4->bones.push_back(Bone());
		auto keyFrameCount = whatever::GetKeyFrameAmount(i);
		if (keyFrameCount > 2)
		{
			for (int j = 0; j < keyFrameCount; j++)
			{
				anim4->bones[i].frames.push_back(Keyframe());
				anim4->bones[i].frames[j].tweenTime = animtweens[i][j];
				auto tempMat = XMFLOAT4X4(animKeyframes[i][j * 16 + 0], animKeyframes[i][j * 16 + 1], animKeyframes[i][j * 16 + 2], animKeyframes[i][j * 16 + 3],
					animKeyframes[i][j * 16 + 4], animKeyframes[i][j * 16 + 5], animKeyframes[i][j * 16 + 6], animKeyframes[i][j * 16 + 7],
					animKeyframes[i][j * 16 + 8], animKeyframes[i][j * 16 + 9], animKeyframes[i][j * 16 + 10], animKeyframes[i][j * 16 + 11],
					animKeyframes[i][j * 16 + 12], animKeyframes[i][j * 16 + 13], animKeyframes[i][j * 16 + 14], animKeyframes[i][j * 16 + 15]);
				XMMatrixDecompose(&blah, &anim4->bones[i].frames[j].rotation, &anim4->bones[i].frames[j].position, XMLoadFloat4x4(&tempMat));
			}
		}
		else if (keyFrameCount == 0)
		{
			anim4->bones[i].frames.push_back(Keyframe());
			anim4->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim4->bones[i].frames[0].rotation, &anim4->bones[i].frames[0].position, identity);
			anim4->bones[i].frames.push_back(Keyframe());
			anim4->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim4->bones[i].frames[1].rotation, &anim4->bones[i].frames[1].position, identity);
		}
		else
		{
			//HOW THE HECK DO YOU ONLY HAVE 1 ANIMATION FRAME?!?
			auto tempMat = XMFLOAT4X4(animKeyframes[i][0], animKeyframes[i][1], animKeyframes[i][2], animKeyframes[i][3],
				animKeyframes[i][4], animKeyframes[i][5], animKeyframes[i][6], animKeyframes[i][7],
				animKeyframes[i][8], animKeyframes[i][9], animKeyframes[i][10], animKeyframes[i][11],
				animKeyframes[i][12], animKeyframes[i][13], animKeyframes[i][14], animKeyframes[i][15]);
			anim4->bones[i].frames.push_back(Keyframe());
			anim4->bones[i].frames[0].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim4->bones[i].frames[0].rotation, &anim4->bones[i].frames[0].position, XMLoadFloat4x4(&tempMat));
			anim4->bones[i].frames.push_back(Keyframe());
			anim4->bones[i].frames[1].tweenTime = 1000;
			XMMatrixDecompose(&blah, &anim4->bones[i].frames[1].rotation, &anim4->bones[i].frames[1].position, XMLoadFloat4x4(&tempMat));
		}
	}
#endif

	BlenderDataStorage* Blender = new BlenderDataStorage();
	Blender->Animations.push_back(*anim1);
	Blender->Animations.push_back(*anim2);
	Blender->Animations.push_back(*anim3);
	Blender->Animations.push_back(*anim4);
	Blender->From.animation = anim1;
	ModelShape->ShapeData.push_back(ShapeData1);
	ModelShape->ShapeData.push_back(Blender);
	ModelShape->ShapeData.push_back(skele1);
	ModelShape->ShapeData.push_back(anim1);
	ModelShape->ShapeData.push_back(anim2);
	ModelShape->ShapeData.push_back(anim3);
	ModelShape->ShapeData.push_back(anim4);


	std::vector<uint8_t> VSData2;
	std::vector<uint8_t> PSData2;
	std::vector<uint8_t> GSData;
#if LOADED_BEAR == 2
	thing = ShaderLoader::LoadShader(VSData2, "ToMultitextuedPixelShader.cso"); //actually is a vertex shader, just weirdly named
	thing = ShaderLoader::LoadShader(PSData2, "MultiTexturedLitPixelShader.cso");
	thing = ShaderLoader::LoadShader(GSData, "TangentGeometryShader.cso");
	Device->CreateVertexShader(&VSData2[0], VSData2.size(), NULL, ModelContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&PSData2[0], PSData2.size(), NULL, ModelContext->m_pixelShader.GetAddressOf());
	Device->CreateGeometryShader(&GSData[0], GSData.size(), NULL, ModelContext->m_geometryShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UVW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
#else
	thing = ShaderLoader::LoadShader(VSData2, "BasicLitSkinningVertShader.cso");
	thing = ShaderLoader::LoadShader(PSData2, "BasicLightPixelShader.cso");
	thing = ShaderLoader::LoadShader(GSData, "BasicGeometryShader.cso");
	Device->CreateVertexShader(&VSData2[0], VSData2.size(), NULL, ModelContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&PSData2[0], PSData2.size(), NULL, ModelContext->m_pixelShader.GetAddressOf());
	Device->CreateGeometryShader(&GSData[0], GSData.size(), NULL, ModelContext->m_geometryShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UVW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
#endif

	Device->CreateInputLayout(vertexDesc2, ARRAYSIZE(vertexDesc2), &VSData2[0], VSData2.size(), ModelContext->m_inputLayout.GetAddressOf());


	SphereMeshthing = new RenderMesh(CleanSphereShape);
	SphereShapething = new RenderShape(devResources, *SphereMeshthing, *planeContext, mat, sphere(), SphereShape, NoCleanup);
	std::vector<uint8_t> VSData3;
	std::vector<uint8_t> PSData3;
	thing = ShaderLoader::LoadShader(VSData3, "InstancedVertexShader.cso");
	//assert(thing);
	thing = ShaderLoader::LoadShader(PSData3, "BasicPixelShader.cso");
	auto VertShad = new Microsoft::WRL::ComPtr<ID3D11VertexShader>();
	auto PixShad = new Microsoft::WRL::ComPtr<ID3D11PixelShader>();
	Device->CreateVertexShader(&VSData3[0], VSData3.size(), NULL, VertShad->GetAddressOf());
	Device->CreatePixelShader(&PSData3[0], PSData3.size(), NULL, PixShad->GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc3[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLDMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLDMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLDMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLDMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	auto InputLay = new Microsoft::WRL::ComPtr<ID3D11InputLayout>();
	HRESULT asdalkdd = Device->CreateInputLayout(vertexDesc3, ARRAYSIZE(vertexDesc3), &VSData3[0], VSData3.size(), InputLay->GetAddressOf());

	SphereMeshthing->MeshData.push_back(VertShad);
	SphereMeshthing->MeshData.push_back(PixShad);
	SphereMeshthing->MeshData.push_back(InputLay);

	VertexPositionColor* SphereVertexBuffer = GenerateObject::CreateD20Verts();

	BufferData = { 0 };
	BufferData.pSysMem = SphereVertexBuffer;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(VertexPositionColor) * 12, D3D11_BIND_VERTEX_BUFFER);
	auto Buffer100 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, &BufferData, Buffer100->GetAddressOf());
	SphereMeshthing->MeshData.push_back(Buffer100);

	unsigned short* SphereInds = GenerateObject::CreateD20Inds();
	SphereMeshthing->m_indexCount = 60;

	BufferData = { 0 };
	BufferData.pSysMem = SphereInds;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(short) * SphereMeshthing->m_indexCount, D3D11_BIND_INDEX_BUFFER);
	Device->CreateBuffer(&constBuffDesc, &BufferData, SphereMeshthing->m_indexBuffer.GetAddressOf());

	int* tempBones = new int;
	*tempBones = numBones;
	SphereMeshthing->MeshData.push_back(tempBones);


	D3D11_RASTERIZER_DESC rasterStateDescriptor;
	ZeroMemory(&rasterStateDescriptor, sizeof(rasterStateDescriptor));
	rasterStateDescriptor.FillMode = D3D11_FILL_WIREFRAME;
	rasterStateDescriptor.CullMode = D3D11_CULL_BACK;
	rasterStateDescriptor.DepthClipEnable = true;

	Device->CreateRasterizerState(&rasterStateDescriptor, rasterWireState.GetAddressOf());
	rasterStateDescriptor.FillMode = D3D11_FILL_SOLID;
	Device->CreateRasterizerState(&rasterStateDescriptor, rasterState.GetAddressOf());

	ModelContext->AddChild(ModelShape);
	//ModelContext->AddChild(planeContext);
	//ModelContext->AddChild(planeShape);
	//ModelContext->AddChild(SphereShapething);

	//planeContext->AddChild(planeShape);
	//planeContext->AddChild(ModelContext);
	//planeContext->AddChild(ModelShape);
	Set = RenderSet();
	Set.SetHead(ModelContext);

	CurrCamera = new Camera;
	CurrCamera->init(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT);
	auto dContext = devResources->GetD3DDeviceContext();
	auto targetView = devResources->GetBackBufferRenderTargetView();
	dContext->OMSetRenderTargets(1, &targetView, devResources->GetDepthStencilView());
	dContext->OMSetDepthStencilState(devResources->GetDepthStencilState(), 0);

	timer.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	Whatchamacallit.clear();
	auto swapChain = devResources->GetSwapChain();
	auto dContext = devResources->GetD3DDeviceContext();
	auto targetView = devResources->GetBackBufferRenderTargetView();
	auto viewport = devResources->GetScreenViewport();
	timer.Signal();
	float delta = (float)timer.Delta();

	CurrCamera->update(delta);
	ModelShape->Update(delta);

	dContext->RSSetViewports(1, &viewport);
	FLOAT color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	dContext->ClearRenderTargetView(targetView, color);
	FLOAT black[] = {1.0f, 1.0f, 1.0f, 1.0f};
	dContext->ClearRenderTargetView(shadowRTV.Get(), black);
	dContext->ClearDepthStencilView(devResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1, 1);
	if (GetAsyncKeyState('F') & 0x1)
	{
		wire = !wire;
	}
	if (wire)
		dContext->RSSetState(rasterWireState.Get());
	else
		dContext->RSSetState(rasterState.Get());
	Renderer::Render(&Set);
	swapChain->Present(0, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	emptyRTV.Reset();

	theEmptyMap.Reset();
	emptySRV.Reset();
	passPS.Reset();
	passVS.Reset();
	clampSamp.Reset();
	nullSOGS.Reset();
	lightViewBuff.Reset();
	depthPS.Reset();
	streamedOutput.Reset();
	theShadowMap.Reset();
	shadowRTV.Reset();
	shadowSRV.Reset();
	rasterState.Reset();
	rasterWireState.Reset();
	InstanceBuff.Reset();
	LightBuff.Reset();
	devResources->checkResources();
	delete planeContext;
	delete planeShape;
	delete planeMesh;
	delete ModelContext;
	delete ModelShape;
	delete ModelMesh;
	delete SphereMeshthing;
	delete SphereShapething;
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