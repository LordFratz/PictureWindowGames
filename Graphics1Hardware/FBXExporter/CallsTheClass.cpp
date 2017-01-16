#include "IncludeMe.h"
#include "FBXExporter.h"

#define DoNotInclude

FBXExporter::FBXExport MyExporter;

//Loads a file with either a .pwm / .pws / .pwa
//IF either of these file types needs to be updated / created -
//all previous values are discarded and populated with the fbxfile data that is inputted (takes quite awhile so be careful)
void whatever::loadFile(const char* filename, const char* Fbxfilename)
{
	MyExporter.FBXConvert(filename, Fbxfilename);
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
	int** VertInds = new int*[MyExporter.BoneVertInds.size()];
	for (int i = 0; i < MyExporter.BoneVertInds.size(); i++) {
		int* temp = new int[MyExporter.BoneVertInds[i].size()];
		for (int e = 0; e < MyExporter.BoneVertInds[i].size(); e++) {
			temp[e] = MyExporter.BoneVertInds[i][e];
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
	float** BoneWeights = new float*[MyExporter.BoneWeights.size()];
	for (int i = 0; i < MyExporter.BoneWeights.size(); i++) {
		float* temp = new float[MyExporter.BoneWeights[i].size()];
		for (int e = 0; e < MyExporter.BoneWeights[i].size(); e++) {
			temp[e] = MyExporter.BoneWeights[i][e];
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
	int** BV = new int*[MyExporter.BoneVerts.size()];
	for (int i = 0; i < MyExporter.BoneVerts.size(); i++) {
		int* BVTemp = new int[4];
		BVTemp[0] = MyExporter.BoneVerts[i].pos[0];
		BVTemp[1] = MyExporter.BoneVerts[i].pos[1];
		BVTemp[2] = MyExporter.BoneVerts[i].pos[2];
		BVTemp[3] = MyExporter.BoneVerts[i].pos[3];
		BV[i] = BVTemp;
	}
	return BV;
}

//Gets the Weights of each vert that correspond to each bone
//Layout:
//Literally the same things as GetVertToBoneInds() but with floats
//and instead of bones being stored in each sub array, it is the weights in the same
//spots as the bones for a easy side by side read (if -1 the bones don't exist so the weights also don't)
float ** whatever::GetVertWeightToBoneInds()
{
	float** BV = new float*[MyExporter.BoneVerts.size()];
	for (int i = 0; i < MyExporter.WeightVerts.size(); i++) {
		float* BVTemp = new float[4];
		BVTemp[0] = MyExporter.WeightVerts[i].pos[0];
		BVTemp[1] = MyExporter.WeightVerts[i].pos[1];
		BVTemp[2] = MyExporter.WeightVerts[i].pos[2];
		BVTemp[3] = MyExporter.WeightVerts[i].pos[3];
		BV[i] = BVTemp;
	}
	return BV;
}

//Returns Count of Animation frames Loaded in
//(use these to read into animation Keyframes for each bone)
//DEPRICATED: will be removed once all refs are gone from code
int whatever::GetKeyFrameCount()
{
	return MyExporter.AnimLength + 1; //Might be wrong
}

//Returns sizeof of storage arrays for GetBoneAnimationKeyFrames() and GetAnimationKeyframeTweens()
int whatever::GetAnimationStorageKeyCount()
{
	return (int)MyExporter.frames.size();
}

//Gets the Count in an array of size GetAnimationStorageKeyCount() of each KeyFrame of each
int whatever::GetKeyFrameAmount(int AnimIndex)
{
	return (int)MyExporter.frames[AnimIndex].size();
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
	float** AnimThang = new float*[MyExporter.frames.size()];
	for (int i = 0; i < MyExporter.frames.size(); i++) {
		float* temp = new float[MyExporter.frames[i].size() * 16];
		for (int e = 0; e < MyExporter.frames[i].size(); e++) {
			int spot = 0;
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					temp[e * 16 + spot] = (float)MyExporter.frames[i][e].GlobalTransform.mData[j].mData[k];
					spot++;
				}
			}
		}
		AnimThang[i] = temp;
	}
	return AnimThang;
}

float ** whatever::GetAnimationKeyframeTweens()
{
	float** Tweens = new float*[MyExporter.frames.size()];
	for (int i = 0; i < MyExporter.frames.size(); i++) {
		float* temp = new float[MyExporter.frames[i].size()];
		for (int e = 0; e < MyExporter.frames[i].size(); e++) {
			if (e + 1 == MyExporter.frames[i].size()) {
				temp[e] = MyExporter.endTime - MyExporter.frames[i][e].timeStamp;
			}
			else temp[e] = MyExporter.frames[i][e - 1].timeStamp - MyExporter.frames[i][e].timeStamp;
		}
		Tweens[i] = temp;
	}
	return Tweens;
}
