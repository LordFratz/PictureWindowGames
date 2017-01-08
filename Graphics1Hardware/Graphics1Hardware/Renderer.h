#pragma once
#include <DirectXMath.h>
#include <vector>
#include "DeviceResources.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
using namespace DirectX;

struct sphere
{
	float radius;

	sphere()
	{
		radius = 0;
	}
	sphere(float Radius)
	{
		radius = Radius;
	}
};

//<temp>
class KdTree
{
	//Function that takes in Camera and returns vector of RenderShapes
	//Fast enough for all dynamic RenderShapes? or just static
	//Better idea?
};
//</temp>
typedef void(*CleanupFunc)();

//typedef void (*RenderFunc)(RenderNode &rNode);

class RenderNode
{
	void(*func)(RenderNode &rNode);
protected:
	RenderNode* next;
public:

	inline void renderProcess() { func(*this); };
	RenderNode* GetNext() const { return next; };;

	RenderNode(void(*Func)(RenderNode &rNode))
	{
		next = nullptr;
		func = Func;
	}
	~RenderNode()
	{
		delete next;
	}
	void AddChild(RenderNode* child)
	{
		if (next == nullptr)
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
public:

	std::shared_ptr<DeviceResources> m_deviceResources;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
	std::vector<void*> ContextData;

	RenderContext(std::shared_ptr<DeviceResources> deviceResources, void(*Func)(RenderNode &rNode), bool IsTransparent) : RenderNode(Func)
	{
		next = nullptr;
		isTransparent = IsTransparent;
		m_deviceResources = deviceResources;
	}
	~RenderContext()
	{
		for(int i = 0; i < ContextData.size(); i++)
		{
			delete ContextData[i];
		}
	}
};

class RenderMesh
{
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_indexBuffer;
	unsigned int m_indexCount;
	std::vector<void*> MeshData;;
	~RenderMesh()
	{
		for(int i = 0; i < MeshData.size(); i++)
		{
			delete MeshData[i];
		}
	}
};

class RenderShape : public RenderNode
{
public:

	std::shared_ptr<DeviceResources> m_deviceResources;
	RenderMesh& Mesh;
	RenderContext& Context;
	XMFLOAT4X4 WorldMat;
	sphere BoundingSphere;

	RenderShape(std::shared_ptr<DeviceResources> deviceResources, RenderMesh& mesh, RenderContext& context, XMFLOAT4X4 worldMat, sphere boundingSphere, void(*Func)(RenderNode &rNode)) : RenderNode(Func), Mesh(mesh), Context(context)
	{
		next = nullptr;
		m_deviceResources = deviceResources;
		WorldMat = worldMat;
		BoundingSphere = boundingSphere;
	}
	~RenderShape()
	{
		delete next;
	}
};

class RenderSet
{
	RenderNode* head = nullptr;
public:
	~RenderSet()
	{
		delete head;
	}
	RenderNode* GetHead() const { return head; }
	void SetHead(RenderNode* Head) { head = Head; }
};

namespace Renderer
{
	inline void Render(RenderSet* set)
	{
		RenderNode* curr = set->GetHead();
		while (curr != nullptr)
		{
			curr->renderProcess();
			curr = curr->GetNext();
		}
	}
}

