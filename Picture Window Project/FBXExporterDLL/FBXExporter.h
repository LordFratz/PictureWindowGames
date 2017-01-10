#pragma once

#ifdef FBXEXPORTERDLL_EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#include <fbxsdk.h>
#include <unordered_map>

namespace FBXExporter
{
 class EXPORT FBXExport
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
		char* InputFilePath;
		char* OutputFilePath;
		bool HasAnim = true;
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
		/*static EXPORT*/ void FBXConvert(char* filename, char* OutputName);
		void ExportFBX(FbxNode* NodeThing, int ParentIndex = -1);
	};
}