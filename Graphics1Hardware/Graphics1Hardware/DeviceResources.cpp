#include "DeviceResources.h"



DeviceResources::DeviceResources()
{
}


DeviceResources::~DeviceResources()
{
}

void DeviceResources::checkResources()
{
	ID3D11Debug *d3dDebug = nullptr;
	if (SUCCEEDED(m_d3dDevice.Get()->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
		d3dDebug->Release();
	}
}

void DeviceResources::cleanup()
{
	m_d3dContext.Reset();
	m_swapChain.Reset();
	m_d3dRenderTargetView.Reset();
	m_d3dDepthStencilView.Reset();

	ID3D11Debug *d3dDebug = nullptr;
	if (SUCCEEDED(m_d3dDevice.Get()->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
		d3dDebug->Release();
	}

	m_d3dDevice.Reset();
}

void DeviceResources::initialize(int sWidth, int sHeight, HWND window)
{
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Width = sWidth;
	scd.BufferDesc.Height = sHeight;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.Windowed = true;
	scd.OutputWindow = window;
	scd.SampleDesc.Count = 1;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &scd, m_swapChain.GetAddressOf(), m_d3dDevice.GetAddressOf(), NULL, m_d3dContext.GetAddressOf());
	ID3D11Texture2D *pBackBuffer;
	m_swapChain.Get()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	m_d3dDevice.Get()->CreateRenderTargetView(pBackBuffer, NULL, m_d3dRenderTargetView.GetAddressOf());
	pBackBuffer->Release();

	ZeroMemory(&m_screenViewport, sizeof(D3D11_VIEWPORT));
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width =  sWidth;
	m_screenViewport.Height = sHeight;

	m_d3dContext->RSSetViewports(1, &m_screenViewport);

	ID3D11Debug *d3dDebug = nullptr;
	if (SUCCEEDED(m_d3dDevice.Get()->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue *d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter;
			memset(&filter, 0, sizeof(filter));
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();
		}
		d3dDebug->Release();
	}
}
