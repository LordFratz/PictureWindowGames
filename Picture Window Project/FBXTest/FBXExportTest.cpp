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
	ExportFBX();
}

void FBXExporter::FBXExport::ExportFBX()
{
	FbxNode* RootNode = Scene->GetRootNode();
	for (int i = 0; i < RootNode->GetChildCount(); i++) {
		FbxNode* ChildNode = RootNode->GetChild(i);
		if (ChildNode->GetNodeAttribute() != NULL && ChildNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
			FbxMesh* Mesh = (FbxMesh*)ChildNode->GetNodeAttribute();
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
		}
	}
}