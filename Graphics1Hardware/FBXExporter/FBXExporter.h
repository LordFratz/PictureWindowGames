#pragma once

#include <fbxsdk.h>
#include <vector>

namespace FBXExporter
{
	class FBXExport
	{
	private:

		struct Vertex {
			float pos[4];
		};

		struct KeyFrame {
			FbxLongLong FrameNum;
			FbxAMatrix GlobalTransform;
		};

		struct Bone {
			std::string name;
			FbxAMatrix bindPoseMatrix;
			std::vector<int> BoneVertInds;
			std::vector<float> BoneWeights;
			std::vector<KeyFrame> frames;
			int parentIndex;
		};
	public:
		FbxManager* SdkManager;
		FbxScene* Scene;
		std::string InputFilePath;
		char* OutputFilePath;
		std::vector<Vertex> Verts;
		std::vector<Vertex> Normals;
		std::vector<Vertex> UVs;
		std::vector<int> Indecies;
		std::vector<Bone> Skeleton;
		std::string CurrentAnimName;
		unsigned int AnimLength;

		FBXExport() {};
		~FBXExport();
		void FBXConvert(const char* filename, char* OutputName);
	private:
		FbxAMatrix ConvertToDirectX(FbxAMatrix mat);
		void ExportFBX(FbxNode* NodeThing, int ParentIndex = -1);
		FbxAMatrix GetGeometryTransformation(FbxNode* inNode);
		void ClearInfo();
	};
}