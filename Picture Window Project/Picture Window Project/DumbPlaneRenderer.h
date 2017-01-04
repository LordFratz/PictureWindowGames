#pragma once
#include <pch.h>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include <DirectXMath.h>
using namespace DirectX;
class DumbPlaneRenderer
{
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct VertexPositionColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 color;
	};

	std::shared_ptr<DX::DeviceResources> m_deviceResources;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

	ModelViewProjectionConstantBuffer	m_constantBufferData;
	uint32	m_indexCount;

	bool	m_loadingComplete;
	XMFLOAT4X4 world, camera, proj;;

public:
	DumbPlaneRenderer();
	DumbPlaneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
	~DumbPlaneRenderer();
	void Render();
	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void ReleaseDeviceDependentResources();
};

