#include "FBXExporter.h"
#include <stdio.h>

FBXExporter::FBXExport::~FBXExport()
{
}

void FBXExporter::FBXExport::FBXConvert(const char* filename, char* OutputName) {
	InputFilePath = filename;
	OutputFilePath = OutputName;
	SdkManager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(SdkManager, IOSROOT);
	SdkManager->SetIOSettings(ios);
	FbxImporter* Importer = FbxImporter::Create(SdkManager, "");
	if (!Importer->Initialize(filename, -1, SdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", Importer->GetStatus().GetErrorString());
		exit(-1);
	}
	Scene = FbxScene::Create(SdkManager, "myScene");
	Importer->Import(Scene);
	Importer->Destroy();
	FbxNode* RootNode = Scene->GetRootNode();
	ExportFBX(RootNode);
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

void FBXExporter::FBXExport::ExportFBX(FbxNode* NodeThing, int ParentIndex)
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
		//gets all bones, the bind pose matrix, bone weights, vertex per bones
		int numDefs = Mesh->GetDeformerCount();
		FbxAMatrix geometryTransform = GetGeometryTransformation(NodeThing);
		FbxSkin* skin = (FbxSkin*)Mesh->GetDeformer(0, FbxDeformer::eSkin);
		if (skin != 0) {
			int boneCnt = skin->GetClusterCount();
			for (int BIndex = 0; BIndex < boneCnt; BIndex++) {
				FbxCluster* cl = skin->GetCluster(BIndex);
				FbxNode* bone = cl->GetLink();
				Bone tempBone;
				tempBone.name = cl->GetLink()->GetName();
				FbxAMatrix tempMat;
				FbxAMatrix transformMatrix;
				cl->GetTransformMatrix(transformMatrix);
				cl->GetTransformLinkMatrix(tempMat);
				//Matrix Conversion Here
				tempBone.bindPoseMatrix = ConvertToDirectX(transformMatrix.Inverse() * transformMatrix * geometryTransform);
				int* boneVertexInds = cl->GetControlPointIndices();
				double *boneVertexWeights = cl->GetControlPointWeights();
				int NumBoneVertInd = cl->GetControlPointIndicesCount();
				for (int BVIndex = 0; BVIndex < NumBoneVertInd; BVIndex++) {
					tempBone.BoneVertInds.push_back(boneVertexInds[BVIndex]);
					tempBone.BoneWeights.push_back((float)boneVertexWeights[BVIndex]);
				}
				tempBone.parentIndex = ParentIndex;
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
					tempBone.frames.push_back(currAnim);
				}
				Skeleton.push_back(tempBone);
			}
		}
	}
	for (int i = 0; i < NodeThing->GetChildCount(); i++) {
		FbxNode* ChildNode = NodeThing->GetChild(i);
		ExportFBX(ChildNode, (int)Skeleton.size() - 1);
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
	InputFilePath.clear();
	OutputFilePath = nullptr;
	Verts.clear();
	Normals.clear();
	UVs.clear();
	Indecies.clear();
	Skeleton.clear();
	CurrentAnimName.clear();
	AnimLength = 0;
	Scene = nullptr;
	SdkManager = nullptr;
}
