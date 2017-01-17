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

struct Keyframe
{
public:
	float tweenTime;
	XMVECTOR rotation;
	XMVECTOR position;

	XMMATRIX getMat()
	{
		const XMFLOAT3 one = XMFLOAT3(1, 1, 1);
		return XMMatrixAffineTransformation(XMLoadFloat3(&one), XMVectorZero(), rotation, position);
	}
};

struct Bone
{
	std::vector<Keyframe> frames;
};

struct currFrame
{
	std::vector<Keyframe> thisFrame;
};

struct Animation
{
	std::vector<Bone> bones;
};

struct PerBoneData
{
	float frameTime = 0;
	int prevFrame = 0;
	int nextFrame = 1;
};

class Interpolator
{
	std::vector<PerBoneData> perBoneData = std::vector<PerBoneData>();
	bool initializedData = false;
public:
	Animation* animation;
	bool KeyboardControl = true;
	bool changedLastFrame;
	currFrame CurrFrame;
	void Update(float delta)
	{
		CurrFrame = currFrame();
		if(KeyboardControl)
		{
			int moveDir = 0;
			float minNextKey = INFINITY;
			if(GetAsyncKeyState(0x4f))
			{
				if(!changedLastFrame)
				{
					moveDir = 1;
					for (int i = 0; i < animation->bones.size(); i++)
					{
						if (!initializedData)
						{
							perBoneData.push_back(PerBoneData());
						}
						if (animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime < minNextKey && moveDir == 1)
						{
							minNextKey = animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime;
						}
					}
					initializedData = true;
				}
				changedLastFrame = true;
			}
			else if(GetAsyncKeyState(0x50))
			{
				if(!changedLastFrame)
				{
					moveDir = -1;
					for (int i = 0; i < animation->bones.size(); i++)
					{
						if (!initializedData)
						{
							perBoneData.push_back(PerBoneData());
						}
						int temp = perBoneData[i].prevFrame - 1;
						if (temp < 0)
						{
							temp = (int)animation->bones[i].frames.size() - 1;
						}
						if (animation->bones[i].frames[perBoneData[i].prevFrame - 1 > -1 ? perBoneData[i].prevFrame - 1 : (int)animation->bones[i].frames.size() - 1].tweenTime < minNextKey)
						{
							minNextKey = animation->bones[i].frames[temp].tweenTime != 0 ? animation->bones[i].frames[temp].tweenTime : .01f;
						}
					}
					initializedData = true;
				}
				changedLastFrame = true;
			}
			else
			{
				changedLastFrame = false;
			}

			if (GetAsyncKeyState(0x49)) //i
			{
				KeyboardControl = false;
			}
			if(minNextKey == INFINITY)
			{
				minNextKey = 0;
			}
			for(int i = 0; i < animation->bones.size(); i++)
			{
				if (!initializedData)
				{
					perBoneData.push_back(PerBoneData());
				}
				perBoneData[i].frameTime += moveDir * minNextKey;
				if(perBoneData[i].frameTime < 0)
				{
					perBoneData[i].nextFrame = perBoneData[i].prevFrame--;
					if (perBoneData[i].prevFrame < 0)
					{
						perBoneData[i].prevFrame = (int)animation->bones[i].frames.size() - 1;
					}
					perBoneData[i].frameTime += animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime;
				}
				else if(perBoneData[i].frameTime > animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime)
				{
					perBoneData[i].prevFrame = perBoneData[i].nextFrame++;
					if (perBoneData[i].nextFrame > animation->bones[i].frames.size() - 1)
					{
						perBoneData[i].nextFrame = 0;
					}
					perBoneData[i].frameTime -= animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime;
				}

				float tweenDelta = perBoneData[i].frameTime / animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime;
				CurrFrame.thisFrame.push_back(Interpolate(animation->bones[i].frames[perBoneData[i].prevFrame], animation->bones[i].frames[perBoneData[i].nextFrame], tweenDelta));
			}
			initializedData = true;
		}
		else
		{
			if(GetAsyncKeyState(0x50) || GetAsyncKeyState(0x4f)) // O or P
			{
				KeyboardControl = true;
			}
			for(int i = 0; i < animation->bones.size(); i++)
			{
				if (!initializedData)
				{
					perBoneData.push_back(PerBoneData());
				}
				perBoneData[i].frameTime += delta;
				while(perBoneData[i].frameTime > animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime)
				{
					perBoneData[i].frameTime -= animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime;
					perBoneData[i].prevFrame = perBoneData[i].nextFrame++;
					if(perBoneData[i].nextFrame > animation->bones[i].frames.size() - 1)
					{
						perBoneData[i].nextFrame = 0;
					}
				}
				float tweenDelta = perBoneData[i].frameTime / animation->bones[i].frames[perBoneData[i].prevFrame].tweenTime;
				CurrFrame.thisFrame.push_back(Interpolate(animation->bones[i].frames[perBoneData[i].prevFrame], animation->bones[i].frames[perBoneData[i].nextFrame], tweenDelta));
			}
			initializedData = true;
		}
	}

	Keyframe Interpolate(Keyframe currFrame, Keyframe nextFrame, float ratio)
	{
		Keyframe rv = Keyframe();
		rv.rotation = XMQuaternionSlerp(currFrame.rotation, nextFrame.rotation, ratio);
		rv.position = XMVectorLerp(currFrame.position, nextFrame.position, ratio);
		return rv;
	}
};

class TransformNode
{
	std::vector<TransformNode*> children = std::vector<TransformNode*>();
	XMMATRIX local;
	XMMATRIX world;
	TransformNode* parent = nullptr;
	bool dirty = true;
public:

	XMMATRIX& getLocal()
	{
		return local;
	}

	XMMATRIX& getWorld()
	{
		if(dirty)
		{
			if (parent != nullptr)
			{
				world = XMMatrixInverse(nullptr, parent->getWorld()) * local;
			}
			else
			{
				world = local;
			}
			dirty = false;
		}
		return world;
	}

	void setLocal(XMMATRIX& newLocal)
	{
		local = newLocal;
		makeDirty();
	}

	void addParent(TransformNode* Parent)
	{
		parent = Parent;
		Parent->children.push_back(this);
	}

private:
	void makeDirty()
	{
		dirty = true;
		for (int i = 0; i < children.size(); i++)
		{
			children[i]->makeDirty();
		}
	}
};
//Try multiplying in wrong order
struct Skeleton
{
	std::vector<TransformNode> Bones; //1 to 1 index with bones in animation
	std::vector<XMMATRIX> InverseBindMats;

	XMFLOAT4X4* getBoneOffsets(currFrame frame, XMMATRIX world)
	{
		XMFLOAT4X4* offsets = new XMFLOAT4X4[frame.thisFrame.size() + 1]();
		XMStoreFloat4x4(&offsets[0], XMMatrixIdentity());
		for(int i = 0; i < frame.thisFrame.size(); i++)
		{
			auto val = frame.thisFrame[i].getMat();
			Bones[i].setLocal(val);
		}
		for (int i = 0; i < frame.thisFrame.size(); i++)
		{
			XMStoreFloat4x4(&offsets[i + 1], InverseBindMats[i] * Bones[i].getWorld());
		}
		return offsets;
	}
};

typedef void(*CleanupFunc)(std::vector<void*> toClean);

class RenderNode
{
	void(*func)(RenderNode &rNode);
protected:
	RenderNode* next;
public:

	inline void renderProcess() { func(*this); };
	RenderNode* GetNext() const { return next; };

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
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_geometryShader;
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
		m_geometryShader.Reset();
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