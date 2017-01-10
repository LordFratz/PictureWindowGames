#pragma once

#define EXPORT __declspec(dllexport)
#include <vector>

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
	}
};