#include "IncludeMe.h"
#include "FBXExporter.h"

#define DoNotInclude

FBXExporter::FBXExport MyExporter;

void whatever::loadFile(const char * filename)
{
	MyExporter.FBXConvert(filename, "asdasd");
}

float* whatever::GetVerts()
{
	float* V = new float[MyExporter.Verts.size() * 4];
	for (int i = 0; i < MyExporter.Verts.size(); i++) {
		V[i * 4 + 0] = MyExporter.Verts[i].pos[0];
		V[i * 4 + 1] = MyExporter.Verts[i].pos[1];
		V[i * 4 + 2] = MyExporter.Verts[i].pos[2];
		V[i * 4 + 3] = MyExporter.Verts[i].pos[3];
	}
	return V;
}

float* whatever::GetNormals()
{
	float* N = new float[MyExporter.Normals.size()];
	for (int i = 0; i < MyExporter.Normals.size(); i++) {
		N[i * 4 + 0] = MyExporter.Normals[i].pos[0];
		N[i * 4 + 1] = MyExporter.Normals[i].pos[1];
		N[i * 4 + 2] = MyExporter.Normals[i].pos[2];
		N[i * 4 + 3] = MyExporter.Normals[i].pos[3];
	}
	return N;
}

float* whatever::GetUVs()
{
	float* UV = new float[MyExporter.UVs.size() * 2];
	for (int i = 0; i < MyExporter.Normals.size(); i++) {
		UV[i * 2 + 0] = MyExporter.UVs[i].pos[0];
		UV[i * 2 + 1] = MyExporter.UVs[i].pos[1];
	}
	return UV;
}

int* whatever::GetInd()
{
	int* Ind = new int[MyExporter.Indecies.size()];
	for (int i = 0; i < MyExporter.Indecies.size(); i++) {
		Ind[i] = MyExporter.Indecies[i];
	}
	return Ind;
}

float ** whatever::GetBoneBindMat()
{
	float** BoneMats = new float*[MyExporter.Skeleton.size()];
	for (int i = 0; i < MyExporter.Skeleton.size(); i++) {
		int spot = 0;
		float* temp = new float[16];
		for (int e = 0; e < 4; e++) {
			for (int j = 0; j < 4; j++) {
				temp[spot] = (float)MyExporter.Skeleton[i].bindPoseMatrix[e].mData[j];
				spot++;
			}
		}
		BoneMats[i] = temp;
	}
	return BoneMats;
}

int ** whatever::GetBoneVertInds()
{
	int** VertInds = new int*[MyExporter.Skeleton.size()];
	for (int i = 0; i < MyExporter.Skeleton.size(); i++) {
		int* temp = new int[MyExporter.Skeleton[i].BoneVertInds.size()];
		for (int e = 0; e < MyExporter.Skeleton[i].BoneVertInds.size(); e++) {
			temp[e] = MyExporter.Skeleton[i].BoneVertInds[e];
		}
		VertInds[i] = temp;
	}
	return VertInds;
}

float ** whatever::GetBoneWeights()
{
	float** BoneWeights = new float*[MyExporter.Skeleton.size()];
	for (int i = 0; i < MyExporter.Skeleton.size(); i++) {
		float* temp = new float[MyExporter.Skeleton[i].BoneWeights.size()];
		for (int e = 0; e < MyExporter.Skeleton[i].BoneWeights.size(); e++) {
			temp[e] = MyExporter.Skeleton[i].BoneWeights[e];
		}
		BoneWeights[i] = temp;
	}
	return BoneWeights;
}

int * whatever::GetParentInds()
{
	int* PInds = new int[MyExporter.Skeleton.size()];
	for (int i = 0; i < MyExporter.Skeleton.size(); i++) {
		PInds[i] = MyExporter.Skeleton[i].parentIndex;
	}
	return PInds;
}
