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

		struct Vertexint {
			int pos[4];
		};

		struct KeyFrame {
			float timeStamp;
			FbxLongLong FrameNum;
			FbxAMatrix GlobalTransform;
		};

		struct Bone {
			//std::string name;
			FbxAMatrix bindPoseMatrix;
			//std::vector<int> BoneVertInds;
			//std::vector<float> BoneWeights;
			//std::vector<KeyFrame> frames;
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

		std::vector<std::string> boneNames;
		std::vector<std::vector<int>> BoneVertInds;
		std::vector<std::vector<float>> BoneWeights;
		std::vector<std::vector<KeyFrame>> frames;

		std::vector<Vertexint> BoneVerts;
		std::vector<Vertex> WeightVerts;
		float startTime;
		float endTime;

		int SocketIndex = 0;

		//int** BoneVerts = nullptr;
		//float** WeightVerts = nullptr;

		bool NotLoadingMeshData = false;

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
		void ExportToBin(FileInfo::ExporterHeader* Header, const char* filename, const char* Fbxfilename);
		void ReadInBin(FileInfo::ExporterHeader* Header, FILE* file, const char* filename);
	};
}