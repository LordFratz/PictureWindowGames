#pragma once

#include <fbxsdk.h>
#include <unordered_map>

namespace FBXExporter
{
	class FBXExport
	{
	private:

		struct Vertex {
			float pos[4];
		};

		struct Bone {
			FbxAMatrix bindPoseMatrix;
			std::vector<int> BoneVertInds;
			std::vector<float> BoneWeights;
			int parentIndex;
		};

		struct KeyFrame {
			FbxLongLong FrameNum;
			FbxAMatrix GlobalTransform;
			KeyFrame* mNext = nullptr;
		};
	public:
		FbxManager* SdkManager;
		FbxScene* Scene;
		std::string InputFilePath;
		char* OutputFilePath;
		//bool HasAnim = true;
		std::vector<Vertex> Verts;
		std::vector<Vertex> Normals;
		std::vector<Vertex> UVs;
		std::vector<int> Indecies;
		std::vector<Bone> Skeleton;
		//FbxLongLong AnimLength;
		//std::vector<KeyFrame> Animation;
		//FbxTexture* Texture;

		FBXExport() {};
		~FBXExport();
		/*static EXPORT*/ void FBXConvert(const char* filename, char* OutputName);
	private:
		void ConvertToDirectX();
		void ExportFBX(FbxNode* NodeThing, int ParentIndex = -1);
	};
}