#pragma once

#define EXPORT __declspec(dllexport)

extern "C" {
	namespace whatever {
		EXPORT void loadFile(const char* filename);
		EXPORT float* GetVerts();
		EXPORT float* GetNormals();
		EXPORT float* GetUVs();
		EXPORT int* GetInd();
		EXPORT float** GetBoneBindMat();
		EXPORT int** GetBoneVertInds();
		EXPORT float** GetBoneWeights();
		EXPORT int* GetParentInds();
		EXPORT int GetVertCount();
		EXPORT int GetNormalCount();
		EXPORT int GetUVCount();
		EXPORT int GetIndCount();
		EXPORT int* GetBoneCounts();
	}
};