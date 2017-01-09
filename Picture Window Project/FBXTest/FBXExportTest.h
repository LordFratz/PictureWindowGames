#pragma once

//#ifdef FBXEXPORTER_EXPORTS
//#define EXPORT __declspec(dllexport)
//#else
//#define EXPORT __declspec(dllimport)
//#endif

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

		FbxManager* SdkManager;
		FbxScene* Scene;
		char* InputFilePath;
		char* OutputFilePath;
		bool HasAnim = true;
		std::vector<Vertex> Verts;
		std::vector<Vertex> Normals;
		std::vector<Vertex> UVs;
		std::vector<int> Indecies;
		
	public:
		FBXExport() {};
		/*static EXPORT*/ void FBXConvert(char* filename, char* OutputName);
		void ExportFBX();
	};
}