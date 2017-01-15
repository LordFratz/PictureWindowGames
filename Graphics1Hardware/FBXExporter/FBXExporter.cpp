#include "FBXExporter.h"
#include <stdio.h>

FBXExporter::FBXExport::~FBXExport()
{
	ClearInfo();
}

void FBXExporter::FBXExport::FBXConvert(const char* filename, const char* Fbxfilename) {
	std::string name = filename;
	SdkManager = nullptr;
	Scene = nullptr;
	SdkManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(SdkManager, IOSROOT);
	SdkManager->SetIOSettings(ios);
	FbxImporter* Importer = FbxImporter::Create(SdkManager, "");
	if (!Importer->Initialize(Fbxfilename, -1, SdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", Importer->GetStatus().GetErrorString());
		exit(-1);
	}
	Scene = FbxScene::Create(SdkManager, "myScene");
	Importer->Import(Scene);

	//FbxLocalTime TimeStamp = Importer->GetFileHeaderInfo()->mCreationTimeStamp;
	Importer->Destroy();
	//time_t testTime;
	//struct tm y2k = {0};
	//y2k.tm_sec = TimeStamp.mSecond;
	//y2k.tm_min = TimeStamp.mMinute;
	//y2k.tm_hour = TimeStamp.mHour;
	//y2k.tm_mday = TimeStamp.mDay;
	//y2k.tm_mon = TimeStamp.mMonth;
	//y2k.tm_year = TimeStamp.mYear;

	//FILE* file = nullptr;
	//FileInfo::ExporterHeader* Header = new FileInfo::ExporterHeader();
	//
	//if (Header->ReadHeader(&file, filename, Fbxfilename)) {
	//	ReadInBin(Header, filename);
	//}
	//else {
	//	ClearInfo();
	//	FbxNode* RootNode = Scene->GetRootNode();
	//	ProcessSkeleton(RootNode);
	//	ExportFBX(RootNode);
	//	SetVertToBoneInds();
	//	SetWeightToBoneInds();
	//	ExportToBin(filename);
	//}
	ClearInfo();
	FbxNode* RootNode = Scene->GetRootNode();
	ProcessSkeleton(RootNode);
	ExportFBX(RootNode);
	SetVertToBoneInds();
	SetWeightToBoneInds();
	Scene->Destroy();
	SdkManager->Destroy();
}

FbxAMatrix FBXExporter::FBXExport::ConvertToDirectX(FbxAMatrix mat)
{
	//change matrixes in to be DirectX compatible (might be wrong who knows)
	FbxVector4 translation = mat.GetT();
	FbxVector4 rotation = mat.GetR();
	translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
	rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
	mat.SetT(translation);
	mat.SetR(rotation);
	return mat;
}

void FBXExporter::FBXExport::ExportFBX(FbxNode* NodeThing)
{
	if (NodeThing->GetNodeAttribute() != NULL && NodeThing->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
		FbxMesh* Mesh = (FbxMesh*)NodeThing->GetNodeAttribute();
		//gets all the vertexes, normals, and uvs
		FbxVector4* tempverts = Mesh->GetControlPoints();
		FbxGeometryElementUV* Euvs = Mesh->GetElementUV();
		FbxLayerElementArrayTemplate<FbxVector2>* tempUVs = 0;
		FbxArray<FbxVector4> tempNorms;
		Mesh->GetPolygonVertexNormals(tempNorms);
		Mesh->GetTextureUV(&tempUVs, FbxLayerElement::eTextureDiffuse);
		int spot = 0;

		for (int e = 0; e < Mesh->GetPolygonCount(); e++) {
			int NumV = Mesh->GetPolygonSize(e);
			if (NumV == 3) {
				for (int j = 0; j < NumV; j++) {
					int CPIndex = Mesh->GetPolygonVertex(e, j);
					Indecies.push_back(spot);
					CompInds.push_back(CPIndex);
					spot++;
					Vertex vert;
					Vertex newNormal;
					Vertex newUV;

					int UVIndex = Mesh->GetTextureUVIndex(e, j);
					FbxVector2 crud = tempUVs->GetAt(UVIndex);

					vert.pos[0] = (float)tempverts[CPIndex].mData[0];
					vert.pos[1] = (float)tempverts[CPIndex].mData[1];
					vert.pos[2] = -(float)tempverts[CPIndex].mData[2];
					vert.pos[3] = (float)tempverts[CPIndex].mData[3];

					newNormal.pos[0] = (float)tempNorms[e * 3 + j].mData[0];
					newNormal.pos[1] = (float)tempNorms[e * 3 + j].mData[1];
					newNormal.pos[2] = -(float)tempNorms[e * 3 + j].mData[2];
					newNormal.pos[3] = (float)tempNorms[e * 3 + j].mData[3];

					newUV.pos[0] = 1.0f - (float)crud.mData[0] - 0.5f;
					newUV.pos[1] = 1.0f - (float)crud.mData[1];

					Normals.push_back(newNormal);
					UVs.push_back(newUV);
					Verts.push_back(vert);
				}
			}
		}
		//gets all bones, the bind pose matrix, bone weights, vertex per bones, and animation
		int numDefs = Mesh->GetDeformerCount();
		FbxAMatrix geometryTransform = GetGeometryTransformation(NodeThing);
		FbxSkin* skin = (FbxSkin*)Mesh->GetDeformer(0, FbxDeformer::eSkin);
		if (skin != 0) {
			int boneCnt = skin->GetClusterCount();
			for (int BIndex = 0; BIndex < boneCnt; BIndex++) {
				FbxCluster* cl = skin->GetCluster(BIndex);
				FbxNode* bone = cl->GetLink();
				//Bone tempBone;
				std::string tempname = cl->GetLink()->GetName();
				int ind = 0;
				for (; ind < Skeleton.size(); ind++) {
					if (Skeleton[ind].name == tempname) {
						break;
					}
				}
				FbxAMatrix tempMat;
				FbxAMatrix transformMatrix;
				cl->GetTransformMatrix(transformMatrix);
				cl->GetTransformLinkMatrix(tempMat);
				//Matrix Conversion Here
				Skeleton[ind].bindPoseMatrix = ConvertToDirectX(tempMat * transformMatrix * geometryTransform);
				int* boneVertexInds = cl->GetControlPointIndices();
				double *boneVertexWeights = cl->GetControlPointWeights();
				int NumBoneVertInd = cl->GetControlPointIndicesCount();
				for (int BVIndex = 0; BVIndex < NumBoneVertInd; BVIndex++) {
					Skeleton[ind].BoneVertInds.push_back(boneVertexInds[BVIndex]);
					Skeleton[ind].BoneWeights.push_back((float)boneVertexWeights[BVIndex]);
				}
				//tempBone.parentIndex = ParentIndex;
				//Get Animation Info (only one take whatever that means)

				FbxAnimStack* currAnimStack = Scene->GetSrcObject<FbxAnimStack>(0);
				FbxString animStackName = currAnimStack->GetName();
				CurrentAnimName = animStackName.Buffer();
				FbxTakeInfo* takeInfo = Scene->GetTakeInfo(animStackName);
				FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
				AnimLength = (unsigned int)(end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24));
				for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); i++) {
					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames24);
					KeyFrame currAnim;
					currAnim.FrameNum = i;
					FbxAMatrix currentTransformOffset = NodeThing->EvaluateGlobalTransform(currTime) * geometryTransform;
					//Matrix Conversion here
					currAnim.GlobalTransform = ConvertToDirectX(currentTransformOffset.Inverse() * cl->GetLink()->EvaluateGlobalTransform(currTime));
					Skeleton[ind].frames.push_back(currAnim);
				}
				//Skeleton.push_back(tempBone);
			}
		}
	}
	for (int i = 0; i < NodeThing->GetChildCount(); i++) {
		int x = NodeThing->GetChildCount();
		FbxNode* ChildNode = NodeThing->GetChild(i);
		ExportFBX(ChildNode);
	}
}

FbxAMatrix FBXExporter::FBXExport::GetGeometryTransformation(FbxNode * inNode)
{
	if (!inNode)
	{
		throw std::exception("Null for mesh geometry");
	}

	const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

	return FbxAMatrix(lT, lR, lS);
}

void FBXExporter::FBXExport::ClearInfo()
{
	if (BoneVerts != nullptr) {
		for (int i = 0; i < Verts.size(); i++) {
			delete BoneVerts[i];
		}
		delete BoneVerts;
		BoneVerts = nullptr;
	}
	if (WeightVerts != nullptr) {
		for (int i = 0; i < Verts.size(); i++) {
			delete WeightVerts[i];
		}
		delete WeightVerts;
		WeightVerts = nullptr;
	}
	Verts.clear();
	Normals.clear();
	UVs.clear();
	Indecies.clear();
	CompInds.clear();
	Skeleton.clear();
	CurrentAnimName.clear();
	AnimLength = 0;
}

void FBXExporter::FBXExport::ProcessSkeleton(FbxNode * RootNode)
{
	for (int i = 0; i < RootNode->GetChildCount(); i++) {
		FbxNode* CurrNode = RootNode->GetChild(i);
		ProcessSkeletonRecur(CurrNode, 0, 0 , -1);
	}
}

void FBXExporter::FBXExport::ProcessSkeletonRecur(FbxNode * inNode, int inDepth, int myIndex, int inParentIndex)
{
	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Bone tempBone;
		tempBone.parentIndex = inParentIndex;
		tempBone.name = inNode->GetName();
		Skeleton.push_back(tempBone);
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonRecur(inNode->GetChild(i), inDepth + 1, (int)Skeleton.size(), myIndex);
	}
}

void FBXExporter::FBXExport::SetVertToBoneInds()
{
	BoneVerts = new int*[Verts.size()];
	for (int i = 0; i < Verts.size(); i++) {
		int* Bones = new int[4];
		int spot = 0;
		for (int e = 0; e < Skeleton.size(); e++) {
			if (spot >= 4) {
				break;
			}
			for (int j = 0; j < Skeleton[e].BoneVertInds.size(); j++) {
				if (Skeleton[e].BoneVertInds[j] == CompInds[i]) {
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
		BoneVerts[i] = Bones;
	}
}

void FBXExporter::FBXExport::SetWeightToBoneInds()
{
	WeightVerts = new float*[Verts.size()];
	for (int i = 0; i < Verts.size(); i++) {
		float* Bones = new float[4];
		int spot = 0;
		for (int e = 0; e < Skeleton.size(); e++) {
			if (spot >= 4) {
				break;
			}
			for (int j = 0; j < Skeleton[e].BoneVertInds.size(); j++) {
				if (Skeleton[e].BoneVertInds[j] == CompInds[i]) {
					Bones[spot] = Skeleton[e].BoneWeights[j];
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
		WeightVerts[i] = Bones;
	}
}

void FBXExporter::FBXExport::ExportToBin(const char * filename)
{
	std::string TestName = filename;
	int test = (int)TestName.find(".pwm");
	if (test >= 0) { //Mesh Export

	}
	test = (int)TestName.find(".pws");
	if (test >= 0) { //Rig / Skeleton / BindPose Export

	}
	test = (int)TestName.find(".pwa");
	if (test >= 0) { //Animation Export

	}
}

void FBXExporter::FBXExport::ReadInBin(FileInfo::ExporterHeader* Header, const char * filename)
{
	switch (Header->file) {
	case FileInfo::FILE_TYPES::MESH:
	{
		break;
	}
	case FileInfo::FILE_TYPES::BIND_POSE:
	{
		break;
	}
	case FileInfo::FILE_TYPES::ANIMATION:
	{
		break;
	}
	case FileInfo::FILE_TYPES::NAV_MESH:
	{
		//???????
		break;
	}
	}
}
