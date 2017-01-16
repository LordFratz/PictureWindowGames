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

struct Bone
{
public:
	XMVECTOR rotation;
	XMVECTOR position;
};

struct Keyframe
{
public:
	float tweenTime;
	std::vector<Bone> Bones;
};

struct Animation
{
public:
	std::vector<Keyframe> Frames;
};

class Interpolator
{
	float frameTime = 0;
	int prevFrame = 0;
	int nextFrame = 1;
public:
	Animation* animation;
	bool KeyboardControl;
	bool changedLastFrame;
	Keyframe currFrame;
	void Update(float delta)
	{
		if(KeyboardControl)
		{
			if(GetAsyncKeyState(0x4f)) //o
			{
				if(!changedLastFrame)
				{
					prevFrame = nextFrame++;
					if(nextFrame > animation->Frames.size())
					{
						nextFrame = 0;
					}
				}
				changedLastFrame = true;
			}
			else if(GetAsyncKeyState(0x50)) //p
			{
				if(!changedLastFrame)
				{
					prevFrame = nextFrame--;
					if (nextFrame < 0)
					{
						nextFrame = animation->Frames.size() - 1;
					}
				}
				changedLastFrame = true;
			}
			else
			{
				changedLastFrame = false;
			}

			currFrame = animation->Frames[prevFrame];

			if(GetAsyncKeyState(0x49)) //i
			{
				KeyboardControl = false;
			}
		}
		else
		{
			if(GetAsyncKeyState(0x50) || GetAsyncKeyState(0x4f)) // O or P
			{
				KeyboardControl = true;
			}
			frameTime += delta;

			while (frameTime > animation->Frames[prevFrame].tweenTime)
			{
				frameTime -= animation->Frames[prevFrame].tweenTime;
				prevFrame = nextFrame++;
				if (nextFrame > animation->Frames.size())
				{
					nextFrame = 0;
				}
			}
			float tweenDelta = frameTime / animation->Frames[prevFrame].tweenTime;
			currFrame = Interpolate(animation->Frames[prevFrame], animation->Frames[nextFrame], tweenDelta);
		}
	}

	Keyframe Interpolate(Keyframe currFrame, Keyframe nextFrame, float ratio)
	{
		Keyframe rv = Keyframe();
		for (int i = 0; i < currFrame.Bones.size(); i++)
		{
			rv.Bones[i].rotation = XMQuaternionSlerp(currFrame.Bones[i].rotation, nextFrame.Bones[i].rotation, ratio);
			rv.Bones[i].position = XMVectorLerp(currFrame.Bones[i].position, nextFrame.Bones[i].position, ratio);
		}
		return rv;
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
typedef void(*CleanupFunc)(std::vector<void*> toClean);

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
	CleanupFunc cFunc;

	RenderContext(std::shared_ptr<DeviceResources> deviceResources, void(*Func)(RenderNode &rNode), CleanupFunc CFunc, bool IsTransparent) : RenderNode(Func)
	{
		cFunc = CFunc;
		next = nullptr;
		isTransparent = IsTransparent;
		m_deviceResources = deviceResources;
	}
	~RenderContext()
	{
		m_vertexShader.Reset();
		m_pixelShader.Reset();
		m_inputLayout.Reset();
		cFunc(ContextData);
	}
};

class RenderMesh
{
public:
	Microsoft::WRL::ComPtr<ID3D11Buffer>	m_indexBuffer;
	unsigned int m_indexCount;
	std::vector<void*> MeshData;
	//TODO: Remember to pre-invert the bind matrices
	//Holds bind pose/bone data
	CleanupFunc cFunc;
	RenderMesh(CleanupFunc CFunc)
	{
		cFunc = CFunc;
	}
	~RenderMesh()
	{
		m_indexBuffer.Reset();
		cFunc(MeshData);
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
	std::vector<void*> ShapeData;
	CleanupFunc cFunc;
	void(*update)(RenderShape& node, float delta);

	RenderShape(std::shared_ptr<DeviceResources> deviceResources, RenderMesh& mesh, RenderContext& context, XMFLOAT4X4 worldMat, sphere boundingSphere, void(*Func)(RenderNode &rNode), CleanupFunc CFunc, void(*Update)(RenderShape& node, float delta) = nullptr) : RenderNode(Func), Mesh(mesh), Context(context)
	{
		cFunc = CFunc;
		update = Update;
		next = nullptr;
		m_deviceResources = deviceResources;
		WorldMat = worldMat;
		BoundingSphere = boundingSphere;
	}
	~RenderShape()
	{
		cFunc(ShapeData);
	}

	void Update(float delta)
	{
		update(*this, delta);
	}
};

class RenderSet
{
	RenderNode* head = nullptr;
public:
	~RenderSet()
	{
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

