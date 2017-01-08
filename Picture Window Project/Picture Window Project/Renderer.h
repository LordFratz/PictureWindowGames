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

//<temp>
class KdTree
{
	//Function that takes in Camera and returns vector of RenderShapes
	//Fast enough for all dynamic RenderShapes? or just static
	//Better idea?
};
//</temp>
typedef void (*CleanupFunc)();

//typedef void (*RenderFunc)(RenderNode &rNode);

class RenderNode
{
	void(*func)(RenderNode &rNode);
protected:
	RenderNode* next;
public:

	inline void renderProcess() { func(*this); };
	inline RenderNode* GetNext() { return next; };;

	RenderNode(void (*Func)(RenderNode &rNode))
	{
		next = nullptr;
		func = Func;
	}
	~RenderNode()
	{
		delete next;
	}
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

public:
	RenderContext(std::shared_ptr<DX::DeviceResources> deviceResources , void(*Func)(RenderNode &rNode)) : RenderNode(Func)
	{
		m_deviceResources = deviceResources;
	}
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
	RenderShape(RenderMesh& mesh, RenderContext& context, XMFLOAT4X4 worldMat, sphere boundingSphere, void(*Func)(RenderNode* rNode)) : Mesh(mesh), Context(context), RenderNode(Func)
	{
		WorldMat = worldMat;
		BoundingSphere = boundingSphere;
	}
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
};

namespace Renderer
{
	void Render(RenderSet* set)
	{
		RenderNode* curr = set->GetHead();
		while (curr != nullptr)
		{
			curr->renderProcess();
			curr = curr->GetNext();
		}
	}
}

