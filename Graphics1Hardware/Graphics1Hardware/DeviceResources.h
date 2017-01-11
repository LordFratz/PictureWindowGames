#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include <wrl/client.h>

class DeviceResources
{
public:
	DeviceResources();
	~DeviceResources();

	ID3D11Device*				GetD3DDevice() const { return m_d3dDevice.Get(); }
	ID3D11DeviceContext*		GetD3DDeviceContext() const { return m_d3dContext.Get(); }
	IDXGISwapChain*			GetSwapChain() const { return m_swapChain.Get(); }
	ID3D11RenderTargetView*	GetBackBufferRenderTargetView() const { return m_d3dRenderTargetView.Get(); }
	ID3D11DepthStencilView*		GetDepthStencilView() const { return m_d3dDepthStencilView.Get(); }
	D3D11_VIEWPORT				GetScreenViewport() const { return m_screenViewport; }
	DirectX::XMFLOAT4X4			GetOrientationTransform3D() const { return m_orientationTransform3D; }

	void checkResources();
	void cleanup();
	void initialize(int sWidth, int sHeight, HWND window);
private:
	Microsoft::WRL::ComPtr<ID3D11Device>			m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_d3dContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>			m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;
	D3D11_VIEWPORT									m_screenViewport;
	DirectX::XMFLOAT4X4	m_orientationTransform3D;
};

