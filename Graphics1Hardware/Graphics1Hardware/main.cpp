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
#include "FBXExporter.h"
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"
#include "BasicVertexShader.csh"
#include "BasicPixelShader.csh"
#include "DeviceResources.h"
#include "Renderer.h"
#define BACKBUFFER_WIDTH	800
#define BACKBUFFER_HEIGHT	600

struct ViewProj
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;;
};

struct VertexPositionColor
{
	XMFLOAT3 pos;
	XMFLOAT3 color;
};

class Camera
{
	XMMATRIX viewMatrix;
	POINT currCursor, prevCursor;;
public:
	ViewProj cameraData;
	void init(int sWidth, int sHeight)
	{
		static const XMVECTORF32 eye = { 0.0f, 0.0f, -1.5f, 0.0f };
		static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
		static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		viewMatrix = XMMatrixInverse(0, XMMatrixLookAtRH(eye, at, up));
		XMStoreFloat4x4(&cameraData.view, XMMatrixTranspose(viewMatrix));

		float aspectRatio = sWidth / sHeight;
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
			viewMatrix = XMMatrixMultiply(XMMatrixTranslation(0.0f, 0.0f, -cameraSpeed), viewMatrix);
		if (GetAsyncKeyState(83))
			viewMatrix = XMMatrixMultiply(XMMatrixTranslation(0.0f, 0.0f, cameraSpeed), viewMatrix);
		if (GetAsyncKeyState(65))
			viewMatrix = XMMatrixMultiply(XMMatrixTranslation(-cameraSpeed, 0.0f, 0.0f), viewMatrix);
		if (GetAsyncKeyState(68))
			viewMatrix = XMMatrixMultiply(XMMatrixTranslation(cameraSpeed, 0.0f, 0.0f), viewMatrix);
		if (GetAsyncKeyState(67))
			viewMatrix = XMMatrixMultiply(XMMatrixTranslation(0.0f, cameraSpeed, 0.0f), viewMatrix);
		if (GetAsyncKeyState(88))
			viewMatrix = XMMatrixMultiply(XMMatrixTranslation(0.0f, -cameraSpeed, 0.0f), viewMatrix);

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
		prevCursor = currCursor;
	}
};

static Camera* CurrCamera;

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
	FBXExporter::FBXExport FBX;

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
		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vertexBuffer->GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(Node->Mesh.m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
		context->DrawIndexed(Node->Mesh.m_indexCount, 0, 0);
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

	//Loads in data from fbx file into FBX
	FBX.FBXConvert("../Resources/Box_Jump.fbx", "whateverfornow");

	devResources = make_shared<DeviceResources>();
	devResources->initialize(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, window);

	auto Device = devResources->GetD3DDevice();

	//Start Plane Init
	planeContext = new RenderContext(devResources, PlaneContext, false);
	planeMesh = new RenderMesh();
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, XMMatrixIdentity());
	planeShape = new RenderShape(devResources, *planeMesh, *planeContext, mat, sphere(), PlaneShape);

	Device->CreateVertexShader(&BasicVertexShader, ARRAYSIZE(BasicVertexShader), NULL, planeContext->m_vertexShader.GetAddressOf());
	Device->CreatePixelShader(&BasicPixelShader, ARRAYSIZE(BasicPixelShader), NULL, planeContext->m_pixelShader.GetAddressOf());
	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	Device->CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc), &BasicVertexShader, ARRAYSIZE(BasicVertexShader), planeContext->m_inputLayout.GetAddressOf());
	CD3D11_BUFFER_DESC constBuffDesc(sizeof(ViewProj), D3D11_BIND_CONSTANT_BUFFER);
	auto Buffer = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, nullptr, Buffer->GetAddressOf());
	planeContext->ContextData.push_back(Buffer);

	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(XMFLOAT4X4), D3D11_BIND_CONSTANT_BUFFER);
	auto Buffer2 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, nullptr, Buffer2->GetAddressOf());
	planeMesh->MeshData.push_back(Buffer2);

	static const VertexPositionColor cubeVertices[] =
	{
		{ XMFLOAT3(-2.5f, -0.5f, -2.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-2.5f, -0.5f,  2.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(2.5f, -0.5f, -2.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(2.5f, -0.5f,  2.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
	};

	D3D11_SUBRESOURCE_DATA BufferData = { 0 };
	BufferData.pSysMem = cubeVertices;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(cubeVertices), D3D11_BIND_VERTEX_BUFFER);
	auto Buffer3 = new Microsoft::WRL::ComPtr<ID3D11Buffer>();
	Device->CreateBuffer(&constBuffDesc, &BufferData, Buffer3->GetAddressOf());
	planeMesh->MeshData.push_back(Buffer3);

	static const unsigned short cubeIndices[] =
	{
		0,2,1, // -x
		1,2,3,
	};

	planeMesh->m_indexCount = ARRAYSIZE(cubeIndices);
	BufferData = { 0 };
	BufferData.pSysMem = cubeIndices;
	BufferData.SysMemPitch = 0;
	BufferData.SysMemSlicePitch = 0;
	constBuffDesc = CD3D11_BUFFER_DESC(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
	Device->CreateBuffer(&constBuffDesc, &BufferData, planeMesh->m_indexBuffer.GetAddressOf());
	//End Plane Init

	planeContext->AddChild(planeShape);

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
	float delta = timer.Delta();

	CurrCamera->update(delta);

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
	delete planeContext;
	delete planeShape;
	delete planeMesh;
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