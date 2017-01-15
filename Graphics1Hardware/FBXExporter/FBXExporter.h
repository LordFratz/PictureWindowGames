#pragma once

#include <fbxsdk.h>
#include <vector>
#include "ExporterHeader.h"

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
		FbxManager* SdkManager = nullptr;
		FbxScene* Scene = nullptr;
		std::vector<Vertex> Verts;
		std::vector<Vertex> Normals;
		std::vector<Vertex> UVs;
		std::vector<int> Indecies;
		std::vector<int> CompInds;
		std::vector<Bone> Skeleton;
		std::string CurrentAnimName;
		unsigned int AnimLength;

		int** BoneVerts = nullptr;
		float** WeightVerts = nullptr;

		FBXExport() {};
		~FBXExport();
		void FBXConvert(const char* filename, const char* Fbxfilename);
	private:
		FbxAMatrix ConvertToDirectX(FbxAMatrix mat);
		void ExportFBX(FbxNode* NodeThing);
		FbxAMatrix GetGeometryTransformation(FbxNode* inNode);
		void ClearInfo();
		void ProcessSkeleton(FbxNode* RootNode);
		void ProcessSkeletonRecur(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex);
		void SetVertToBoneInds();
		void SetWeightToBoneInds();
		void ExportToBin(const char* filename);
		void ReadInBin(FileInfo::ExporterHeader* Header, const char* filename);
	};
}