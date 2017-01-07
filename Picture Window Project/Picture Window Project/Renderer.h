#pragma once
#include "pch.h"
#include <DirectXMath.h>
#include <vector>
#include "Common\DeviceResources.h"
using namespace DirectX;

struct sphere
{
	float radius;
};

class Camera
{
public:

};

typedef void(*CleanupFunc)(void);
typedef void(*RenderFunc)(RenderNode &rNode);

class RenderNode
{
	RenderFunc func;
public:
	RenderNode* next;
	RenderNode()
	{
		next = nullptr;
	}
	RenderNode(RenderFunc Func)
	{
		next = nullptr;
		func = Func;
	}
	~RenderNode()
	{
		delete next;
	}
	inline void renderProcess() { func(*this) };
	inline RenderNode* GetNext() { return next; };
	inline void AddChild(RenderNode* child)
	{
		if(next == nullptr)
		{
			next = child;
		}
		else
		{
			next->AddChild(child);
		}
	}
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
	Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
	std::vector<void*> MeshData;
public:
	CleanupFunc func;
};

class RenderShape : public RenderNode
{
	RenderMesh& Mesh;
	RenderContext& Context;
	XMFLOAT4X4 WorldMat;
	sphere BoundingSphere;
public:
	~RenderShape()
	{
		Mesh.func();
		delete next;
	}
};

class RenderSet
{
	RenderNode* head;
public:
	~RenderSet()
	{
		delete head;
	}
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

