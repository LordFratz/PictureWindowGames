#include "FBXExportTest.h"
#include <stdio.h>

int main()
{
	FBXExporter::FBXExport temp;
	temp.FBXConvert("../Resources/Box_Idle.fbx", "whatever");
	int x = 0;
	return 0;
}

void FBXExporter::FBXExport::FBXConvert(char* filename, char* OutputName) {
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
}

void FBXExporter::FBXExport::ExportFBX(FbxNode* NodeThing, int ParentIndex)
{
	if (NodeThing->GetNodeAttribute() != NULL && NodeThing->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
		FbxMesh* Mesh = (FbxMesh*)NodeThing->GetNodeAttribute();
		//gets all the vertexes, normals, and uvs
		FbxVector4* tempverts = Mesh->GetControlPoints();
		FbxGeometryElementUV* Euvs = Mesh->GetElementUV();
		FbxArray<FbxVector2> tempUVs;
		Mesh->GetPolygonVertexUVs(Euvs->GetName(), tempUVs);
		int spot = 0;
		for (int e = 0; e < Mesh->GetPolygonCount(); e++) {
			int NumV = Mesh->GetPolygonSize(e);
			if (NumV == 3) {
				for (int j = 0; j < NumV; j++) {
					int CPIndex = Mesh->GetPolygonVertex(e, j);
					Indecies.push_back(CPIndex);
					if (spot > CPIndex) {
						continue;
					}
					spot++;
					FbxVector4 tempNormal;
					Mesh->GetPolygonVertexNormal(j, CPIndex, tempNormal);
					int UVIndex = Mesh->GetTextureUVIndex(e, j);
					Vertex vert;
					Vertex newNormal;
					Vertex newUV;

					vert.pos[0] = (float)tempverts[CPIndex].mData[0];
					vert.pos[1] = (float)tempverts[CPIndex].mData[1];
					vert.pos[2] = (float)tempverts[CPIndex].mData[2];

					newNormal.pos[0] = (float)tempNormal.mData[0];
					newNormal.pos[1] = (float)tempNormal.mData[1];
					newNormal.pos[2] = (float)tempNormal.mData[2];
					newNormal.pos[3] = 1;

					newUV.pos[0] = (float)tempUVs[UVIndex].mData[0];
					newUV.pos[1] = (float)tempUVs[UVIndex].mData[1];

					Verts.push_back(vert);
					Normals.push_back(newNormal);
					UVs.push_back(newUV);
				}
			}
		}
		//Load Keyframes - the data on translation, rotation, and scaling for each keyframe of animation
		//FbxAnimStack* currAnimStack = Scene->GetSrcObject<FbxAnimStack>(0);
		//FbxString animStackName = currAnimStack->GetName();
		//FbxTakeInfo* takeInfo = Scene->GetTakeInfo(animStackName);
		//FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
		//FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
		//AnimLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
		int materialCount = NodeThing->GetSrcObjectCount<FbxSurfaceMaterial>();
		for (int index = 0; index < materialCount; index++)
		{
			FbxSurfaceMaterial* material = (FbxSurfaceMaterial*)NodeThing->GetSrcObject<FbxSurfaceMaterial>(index);
			if (material != NULL)
			{
				FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
				int textureCount = prop.GetSrcObjectCount<FbxTexture>();
				for (int j = 0; j < textureCount; j++)
				{
					FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(j));
					const char* textureName = texture->GetName();
					FbxProperty p = texture->RootProperty.Find("Filename");
				}
			}
		}
		//gets all bones, the bind pose matric, bone weights, vertex per bones
		int numDefs = Mesh->GetDeformerCount();
		FbxSkin* skin = (FbxSkin*)Mesh->GetDeformer(0, FbxDeformer::eSkin);
		if (skin != 0) {
			int boneCnt = skin->GetClusterCount();
			for (int BIndex = 0; BIndex < boneCnt; BIndex++) {
				FbxCluster* cl = skin->GetCluster(BIndex);
				FbxNode* bone = cl->GetLink();
				Bone tempBone;
				cl->GetTransformLinkMatrix(tempBone.bindPoseMatrix);
				int* boneVertexInds = cl->GetControlPointIndices();
				double *boneVertexWeights = cl->GetControlPointWeights();
				int NumBoneVertInd = cl->GetControlPointIndicesCount();
				for (int BVIndex = 0; BVIndex < NumBoneVertInd; BVIndex++) {
					tempBone.BoneVertInds.push_back(boneVertexInds[BVIndex]);
					tempBone.BoneWeights.push_back((float)boneVertexWeights[BVIndex]);
				}
				tempBone.parentIndex = ParentIndex;
				Skeleton.push_back(tempBone);
			}
		}
	}
	for (int i = 0; i < NodeThing->GetChildCount(); i++) {
		FbxNode* ChildNode = NodeThing->GetChild(i);
		ExportFBX(ChildNode, Skeleton.size() - 1);
	}
}