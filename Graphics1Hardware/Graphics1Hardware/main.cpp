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
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"
#include "DeviceResources.h"
#define BACKBUFFER_WIDTH	800
#define BACKBUFFER_HEIGHT	600

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	shared_ptr<DeviceResources> devResources;
	//ID3D11Device *Device;
	//ID3D11DeviceContext *dContext;
	//ID3D11RenderTargetView *targetView;
	//IDXGISwapChain *swapChain;
	//CD3D11_VIEWPORT viewport;
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

public:
	struct SIMPLE_VERTEX
	{
		XMFLOAT2 position;
	};

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************//
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
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
	//********************* END WARNING ************************//
	devResources = make_shared<DeviceResources>();
	devResources->initialize(BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT, window);
	DXGI_SWAP_CHAIN_DESC scd;
	auto Device = devResources->GetD3DDevice();
	auto dContext = devResources->GetD3DDeviceContext();
	SIMPLE_VERTEX circle[361];
	for (unsigned int i = 0; i < vertCount - 1; i++)
	{
		circle[i].position.x = sin(XMConvertToRadians((float)i)) * .2f;
		circle[i].position.y = cos(XMConvertToRadians((float)i)) * .2f;
	}
	circle[360] = circle[0];

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = circle;
	data.SysMemPitch = NULL;
	data.SysMemSlicePitch = NULL;

	D3D11_BUFFER_DESC buffDesc;
	ZeroMemory(&buffDesc, sizeof(D3D11_BUFFER_DESC));
	buffDesc.Usage = D3D11_USAGE_IMMUTABLE;
	buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc.CPUAccessFlags = NULL;
	buffDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * vertCount;
	Device->CreateBuffer(&buffDesc, &data, &VertBuffer);

	SIMPLE_VERTEX background[NumVertsBackground];
	unsigned int vertPos = 0;
	for(unsigned int y = 0; y < 20; y++)
	{
		for(unsigned int x = 0; x < 20; x+= 2)
		{
			if (y % 2 == 1)
			{
				float vertPosX = 1 - .1 * x;
				float vertPosY = -1 * (1 - .1 * y);
				background[vertPos].position.x = vertPosX;
				background[vertPos].position.y = vertPosY;
				vertPos++;
				background[vertPos].position.x = vertPosX - .1f;
				background[vertPos].position.y = vertPosY;
				vertPos++;
				background[vertPos].position.x = vertPosX;
				background[vertPos].position.y = vertPosY + .1f;
				vertPos++;
				background[vertPos].position.x = vertPosX;
				background[vertPos].position.y = vertPosY + .1f;
				vertPos++;
				background[vertPos].position.x = vertPosX - .1f;
				background[vertPos].position.y = vertPosY;
				vertPos++;
				background[vertPos].position.x = vertPosX - .1f;
				background[vertPos].position.y = vertPosY + .1f;
				vertPos++;
			}
			else
			{
				float vertPosX = 1 - .1 * (x + 1);
				float vertPosY = -1 * (1 - .1 * y);
				background[vertPos].position.x = vertPosX;
				background[vertPos].position.y = vertPosY;
				vertPos++;
				background[vertPos].position.x = vertPosX - .1f;
				background[vertPos].position.y = vertPosY;
				vertPos++;
				background[vertPos].position.x = vertPosX;
				background[vertPos].position.y = vertPosY + .1f;
				vertPos++;
				background[vertPos].position.x = vertPosX;
				background[vertPos].position.y = vertPosY + .1f;
				vertPos++;
				background[vertPos].position.x = vertPosX - .1f;
				background[vertPos].position.y = vertPosY;
				vertPos++;
				background[vertPos].position.x = vertPosX - .1f;
				background[vertPos].position.y = vertPosY + .1f;
				vertPos++;
			}
		}
	}
	// TODO: PART 5 STEP 3
	D3D11_SUBRESOURCE_DATA data3;
	ZeroMemory(&data3, sizeof(D3D11_SUBRESOURCE_DATA));
	data3.pSysMem = background;
	data3.SysMemPitch = NULL;
	data3.SysMemSlicePitch = NULL;

	D3D11_BUFFER_DESC buffDesc2;
	ZeroMemory(&buffDesc2, sizeof(D3D11_BUFFER_DESC));
	buffDesc2.Usage = D3D11_USAGE_IMMUTABLE;
	buffDesc2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffDesc2.CPUAccessFlags = NULL;
	buffDesc2.ByteWidth = sizeof(SIMPLE_VERTEX) * NumVertsBackground;
	HRESULT blah = Device->CreateBuffer(&buffDesc2, &data3, &BackgroundBuffer);

	Device->CreateVertexShader(&Trivial_VS, ARRAYSIZE(Trivial_VS), NULL, &pVS);
	Device->CreatePixelShader(&Trivial_PS, ARRAYSIZE(Trivial_PS), NULL, &pPS);
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	Device->CreateInputLayout(vLayout, ARRAYSIZE(vLayout), &Trivial_VS, ARRAYSIZE(Trivial_VS), &pLayout);
	dContext->IASetInputLayout(pLayout);

	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(D3D11_BUFFER_DESC));
	constDesc.ByteWidth = sizeof(SEND_TO_VRAM);
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constDesc.Usage = D3D11_USAGE_DYNAMIC;

	toShader.constantColor = Colors::Yellow;
	toShader.constantOffset.x = 0;
	toShader.constantOffset.y = 0;
	toShader2.constantColor = Colors::Black;
	toShader2.constantOffset.x = 0;
	toShader2.constantOffset.y = 0;

	D3D11_SUBRESOURCE_DATA data2;
	ZeroMemory(&data2, sizeof(D3D11_SUBRESOURCE_DATA));
	data2.pSysMem = &toShader;
	data2.SysMemPitch = 0;
	data2.SysMemSlicePitch = 0;
	Device->CreateBuffer(&constDesc, &data2, &ConstBuffer);
	offsetVel.x = 1.0f;
	offsetVel.y = .5f;;
	timer.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	auto dContext = devResources->GetD3DDeviceContext();
	auto targetView = devResources->GetBackBufferRenderTargetView();
	auto viewport = devResources->GetScreenViewport();
	auto swapChain = devResources->GetSwapChain();
	timer.Signal();
	float delta = timer.Delta();
	toShader.constantOffset.x += offsetVel.x * delta;
	toShader.constantOffset.y += offsetVel.y * delta;
	if(toShader.constantOffset.x + .18 > 1.0f)
	{
		offsetVel.x = -1.0f;
	}
	else if(toShader.constantOffset.x - .18 < -1.0f)
	{
		offsetVel.x = 1.0f;
	}
	if(toShader.constantOffset.y + .18 > 1.0f)
	{
		offsetVel.y = -.5f;
	}
	if(toShader.constantOffset.y - .18 < -1.0f)
	{
		offsetVel.y = .5f;
	}
	dContext->OMSetRenderTargets(1, &targetView, NULL);
	dContext->RSSetViewports(1, &viewport);
	FLOAT color[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	dContext->ClearRenderTargetView(targetView, color);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	dContext->Map(ConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &toShader2, sizeof(SEND_TO_VRAM));
	dContext->Unmap(ConstBuffer, 0);
	dContext->VSSetConstantBuffers(0, 1, &ConstBuffer);

	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;
	dContext->IASetVertexBuffers(0, 1, &BackgroundBuffer, &stride, &offset);
	dContext->VSSetShader(pVS, NULL, 0);
	dContext->PSSetShader(pPS, NULL, 0);
	dContext->IASetInputLayout(pLayout);
	dContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dContext->Draw(NumVertsBackground, 0);
	ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
	dContext->Map(ConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &toShader, sizeof(SEND_TO_VRAM));
	dContext->Unmap(ConstBuffer, 0);
	dContext->VSSetConstantBuffers(0, 1, &ConstBuffer);
	dContext->IASetVertexBuffers(0, 1, &VertBuffer, &stride, &offset);
	dContext->VSSetShader(pVS, NULL, 0);
	dContext->PSSetShader(pPS, NULL, 0);
	dContext->IASetInputLayout(pLayout);
	dContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);
	dContext->Draw(vertCount, 0);
	swapChain->Present(0, 0);
	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{

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