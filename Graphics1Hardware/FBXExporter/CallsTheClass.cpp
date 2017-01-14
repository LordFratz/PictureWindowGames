#include "IncludeMe.h"
#include "FBXExporter.h"

#define DoNotInclude

FBXExporter::FBXExport MyExporter;

//Loads a file, discards all previous values upon a new call
void whatever::loadFile(const char * filename)
{
	MyExporter.FBXConvert(filename);
}

//Every 4 floats refer to a single vertex postion
//Layout:
//{ X }{ Y }{ Z }{ W }{...}
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

//Every 4 floats refer to a single normal
//Layout:
//{ X }{ Y }{ Z }{ W }{...}
float* whatever::GetNormals()
{
	float* N = new float[MyExporter.Normals.size() * 4];
	for (int i = 0; i < MyExporter.Normals.size(); i++) {
		N[i * 4 + 0] = MyExporter.Normals[i].pos[0];
		N[i * 4 + 1] = MyExporter.Normals[i].pos[1];
		N[i * 4 + 2] = MyExporter.Normals[i].pos[2];
		N[i * 4 + 3] = MyExporter.Normals[i].pos[3];
	}
	return N;
}

//Every 2 floats is a UV
//Layout:
//{ U }{ V }{...}
float* whatever::GetUVs()
{
	float* UV = new float[MyExporter.UVs.size() * 2];
	for (int i = 0; i < MyExporter.Normals.size(); i++) {
		UV[i * 2 + 0] = MyExporter.UVs[i].pos[0];
		UV[i * 2 + 1] = MyExporter.UVs[i].pos[1];
	}
	return UV;
}

//Every Short is a index for the respective vertex in the current Loaded file
//Layout:
//{ Ind 1 }{ Ind 2 }{...}
short* whatever::GetShortInd()
{
	short* Ind = new short[MyExporter.Indecies.size()];
	for (int i = 0; i < MyExporter.Indecies.size(); i++) {
		Ind[i] = MyExporter.Indecies[i];
	}
	return Ind;
}

//Every int is a index for the respective vertex in the current Loaded file
//Layout:
//{ Ind 1 }{ Ind 2 }{...}
int* whatever::GetInd()
{
	int* Ind = new int[MyExporter.Indecies.size()];
	for (int i = 0; i < MyExporter.Indecies.size(); i++) {
		Ind[i] = MyExporter.Indecies[i];
	}
	return Ind;
}

//Gets the Index list that represents which verticies share the same space as which
//Layout:
//Basically what the Index list would be if the vertes were indexed to get rid of duplicates
//{int 1}{int 2}{...}
int * whatever::GetCompInd()
{
	int* Ind = new int[MyExporter.CompInds.size()];
	for (int i = 0; i < MyExporter.CompInds.size(); i++) {
		Ind[i] = MyExporter.CompInds[i];
	}
	return Ind;
}

//Gets Bind Pose Matrixies for EVERY bone (joint) in the model
//Layout:
//array of float arrays that represent each bind pose matrix in each bone in order that they are stored
//(should be accessed in the same order as the indicies / bone wieghts / parent indices / animations)
//[First Bone] [...]
//{Value 1 R1} {...}
//{Value 2 R1}
//{Value 3 R1}
//{Value 4 R1}
//{Value 5 R2}
//{...}
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

//Gets Vertex Indicies the each bone influences
//Layout:
//array of int arrays that represents all the vertecies for each bone
//(should be indexed at the same time as the bones)
//[First Bone Verts][...]
//{Ind 1}           {...}
//{Ind 2}
//{...}
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

//Gets Bone Weights for each Vertex index for each bone
//Layout:
//array of float arrays that represent all the wieght indexes for each vertex for each bone
//(should be read in at the same time as the bones)
//[First Bone Weights][...]
//{Weight 1}          {...}
//{Weight 2}
//{...}
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

//Gets parents Indicies to determine who is the parent / child of who
//Use this to setup your skeleton system with the joints (if -1 it has no parent)
//Layout:
//{Ind 1}{Ind 2}{...}
int * whatever::GetParentInds()
{
	int* PInds = new int[MyExporter.Skeleton.size()];
	for (int i = 0; i < MyExporter.Skeleton.size(); i++) {
		PInds[i] = MyExporter.Skeleton[i].parentIndex;
	}
	return PInds;
}

//Gets the Count of Verticies
int whatever::GetVertCount()
{
	return (int)MyExporter.Verts.size();
}

//Gets the Count of Normals
int whatever::GetNormalCount()
{
	return (int)MyExporter.Normals.size();
}

//Gets the Count of UVs
int whatever::GetUVCount()
{
	return (int)MyExporter.UVs.size();
}

//Gets the count of Indicies
int whatever::GetIndCount()
{
	return (int)MyExporter.Indecies.size();
}

//Gets the Bone Counts and the Counts of both the Vertex Indecies and Bone Weights for each bone
//Layout:
// {Amount of Bones}{Vert Indicies Count 1}{Bone Wieght Count 1}{...}
int whatever::GetBoneCount()
{
	return (int)MyExporter.Skeleton.size();
}

//Gets Every Bone that affects every vert in order
//Layout:
//(always has 4 ints per each Vertex, if a bone is -1 the rest of the bones after it including that one do not exist)
//{Vert 1}{Vert 2}{...}->
//    V       V     v
//{Bone 1}  {...} {...}
//{Bone 2}
//{Bone 3}
//{Bone 4}
int ** whatever::GetVertToBoneInds()
{
	int** Verts = new int*[MyExporter.Verts.size()];
	for (int i = 0; i < MyExporter.Verts.size(); i++) {
		int* Bones = new int[4];
		int spot = 0;
		for (int e = 0; e < MyExporter.Skeleton.size(); e++) {
			if (spot >= 4) {
				break;
			}
			for (int j = 0; j < MyExporter.Skeleton[e].BoneVertInds.size(); j++) {
				if (MyExporter.Skeleton[e].BoneVertInds[j] == MyExporter.CompInds[i]) {
					Bones[spot] = e;
					spot++;
					break;
				}
			}
		}
		if (spot < 4) {
			for (int e = spot; e < 4; e++) {
				Bones[e] = -1;
			}
		}
		Verts[i] = Bones;
	}
	return Verts;
}

//Gets the Weights of each vert that correspond to each bone
//Layout:
//Literally the same things as GetVertToBoneInds() but with floats
//and instead of bones being stored in each sub array, it is the weights in the same
//spots as the bones for a easy side by side read (if -1 the bones don't exist so the weights also don't)
float ** whatever::GetVertWeightToBoneInds()
{
	float** Verts = new float*[MyExporter.Verts.size()];
	for (int i = 0; i < MyExporter.Verts.size(); i++) {
		float* Bones = new float[4];
		int spot = 0;
		for (int e = 0; e < MyExporter.Skeleton.size(); e++) {
			if (spot >= 4) {
				break;
			}
			for (int j = 0; j < MyExporter.Skeleton[e].BoneVertInds.size(); j++) {
				if (MyExporter.Skeleton[e].BoneVertInds[j] == MyExporter.CompInds[i]) {
					Bones[spot] = MyExporter.Skeleton[e].BoneWeights[j];
					spot++;
					break;
				}
			}
		}
		if (spot < 4) {
			for (int e = spot; e < 4; e++) {
				Bones[e] = 0;
			}
		}
		Verts[i] = Bones;
	}
	return Verts;
}

//Returns Count of Animation frames Loaded in
//(use these to read into animation Keyframes for each bone)
int whatever::GetKeyFrameCount()
{
	return MyExporter.AnimLength + 1; //Might be wrong
}

//Returns Animation Keyframes
//Layout:
//array of float arrays for each animation in each bone
//(should be read in at the same time as bones)
//[First Bone Animation]                  [...]
//{1st KeyFrame(1st 16 floats Matrix)}    {...}
//{2nd KeyFrame(2nd 16 floats Matrix)}
//{...}
float ** whatever::GetBoneAnimationKeyFrames()
{
	float** AnimThang = new float*[MyExporter.Skeleton.size()];
	for (int i = 0; i < MyExporter.Skeleton.size(); i++) {
		float* temp = new float[MyExporter.Skeleton[i].frames.size() * 16];
		for (int e = 0; e < MyExporter.Skeleton[i].frames.size(); e++) {
			int spot = 0;
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					temp[e * 16 + spot] = (float)MyExporter.Skeleton[i].frames[e].GlobalTransform.mData[j].mData[k];
					spot++;
				}
			}
		}
		AnimThang[i] = temp;
	}
	return AnimThang;
}
