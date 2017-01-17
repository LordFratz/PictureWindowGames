#include "DeviceResources.h"



DeviceResources::DeviceResources()
{
}


DeviceResources::~DeviceResources()
{
}

void DeviceResources::checkResources()
{
	m_d3dContext.Get()->ClearState();
	m_d3dContext.Get()->Flush();
	ID3D11Debug *d3dDebug = nullptr;
	if (SUCCEEDED(m_d3dDevice.Get()->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_SUMMARY);
		d3dDebug->Release();
	}
}

void DeviceResources::cleanup()
{
	m_swapChain.Reset();
	m_d3dRenderTargetView.Reset();
	m_d3dDepthStencilState.Reset();
	m_d3dDepthStencilView.Reset();

	m_d3dContext.Get()->ClearState();
	m_d3dContext.Get()->Flush();
	m_d3dContext.Reset();

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
	m_screenViewport.Width =  (FLOAT)sWidth;
	m_screenViewport.Height = (FLOAT)sHeight;

	m_d3dContext->RSSetViewports(1, &m_screenViewport);

	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		lround(m_screenViewport.Width),
		lround(m_screenViewport.Height),
		1, // This depth stencil view has only one texture.
		1, // Use a single mipmap level.
		D3D11_BIND_DEPTH_STENCIL
	);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencil;
	m_d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		depthStencil.GetAddressOf()
	);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	m_d3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		m_d3dDepthStencilView.GetAddressOf()
	);

	D3D11_DEPTH_STENCIL_DESC StateDesc;
	StateDesc.DepthEnable = true;
	StateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	StateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	StateDesc.StencilEnable = false;
	StateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	StateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	StateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	StateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	StateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	StateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	StateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	StateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	StateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	StateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	m_d3dDevice->CreateDepthStencilState(&StateDesc, m_d3dDepthStencilState.GetAddressOf());

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
