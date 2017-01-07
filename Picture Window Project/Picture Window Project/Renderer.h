#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <vector>
#include "Common\DeviceResources.h"
using namespace DirectX;

class Camera
{

};

typedef void(*RenderFunc)(RenderNode &rNode);

class RenderNode
{
	RenderFunc func;
	RenderNode* next;
public:
	inline void renderProcess() { func(*this) };
	inline RenderNode* GetNext() { return next; };
};

class RenderContext : public RenderNode
{
	bool isTransparent;
	std::shared_ptr<DX::DeviceResources> m_deviceResources;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	std::vector<void*> ContextData;
};

class RenderMesh
{
	//TODO: Develop Mesh Storage Here
};

class RenderShape : public RenderNode
{
	RenderMesh& Mesh;
	RenderContext& Context;
	XMFLOAT4X4 WorldMat;
};

class RenderSet
{
	RenderNode* head;
public:
	inline RenderNode* GetHead() { return head; };
	static RenderSet* CreateSet(Camera& camera);
};

namespace Renderer
{
	void Render(RenderSet set)
	{
		RenderNode* curr = set.GetHead();
		while (curr != nullptr)
		{
			curr->renderProcess();
			curr = curr->GetNext;
		}
	}
}

