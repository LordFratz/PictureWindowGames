#include "FBXExporter.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

FBXExporter::FBXExport::~FBXExport()
{
	NotLoadingMeshData = false;
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
	Importer->Destroy();
	FILE* file = nullptr;
	std::string TestName = filename;
	FileInfo::ExporterHeader* Header;
	int test = (int)TestName.find(".pwm");
	if (test >= 0) { //Mesh Export
		Header = new FileInfo::ExporterHeader(FileInfo::FILE_TYPES::MESH, Fbxfilename);
		NotLoadingMeshData = false;
	}
	test = (int)TestName.find(".pws");
	if (test >= 0) { //Rig / Skeleton / BindPose Export
		Header = new FileInfo::ExporterHeader(FileInfo::FILE_TYPES::BIND_POSE, Fbxfilename);
	}
	test = (int)TestName.find(".pwa");
	if (test >= 0) { //Animation Export
		Header = new FileInfo::ExporterHeader(FileInfo::FILE_TYPES::ANIMATION, Fbxfilename);
		NotLoadingMeshData = true;
	}

	if (Header->ReadHeader(&file, filename, Fbxfilename)) {
		ReadInBin(Header, file, filename);
		NotLoadingMeshData = true;
	}
	else {
		ClearInfo();
		FbxNode* RootNode = Scene->GetRootNode();
		ProcessSkeleton(RootNode);
		ExportFBX(RootNode);
		if (!NotLoadingMeshData) {
			GetSocketIndecies();
			RemoveSockets();
			SetVertToBoneInds();
			SetWeightToBoneInds();
		}
		ExportToBin(Header, filename, Fbxfilename);
	}
	Scene->Destroy();
	SdkManager->Destroy();
}

FbxAMatrix FBXExporter::FBXExport::ConvertToDirectX(FbxAMatrix mat)
{
	//change matrixes in to be DirectX compatible (might be wrong who knows)
	//FbxVector4 translation = mat.GetT();
	//FbxVector4 rotation = mat.GetR();
	//translation.Set(translation.mData[0], translation.mData[1], -translation.mData[2]);
	//rotation.Set(-rotation.mData[0], -rotation.mData[1], rotation.mData[2]);
	//mat.SetT(translation);
	//mat.SetR(rotation);
	mat.mData[1].mData[3] = -mat.mData[1].mData[3];
	mat.mData[2].mData[3] = -mat.mData[2].mData[3];
	mat.mData[3].mData[1] = -mat.mData[3].mData[1];
	mat.mData[3].mData[2] = -mat.mData[3].mData[2];
	mat.mData[3].mData[4] = -mat.mData[3].mData[4];
	mat.mData[4].mData[3] = -mat.mData[4].mData[3];
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

					newUV.pos[0] = (float)crud.mData[0];
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
				std::string tempname = cl->GetLink()->GetName();
				int ind = 0;
				for (; ind < Skeleton.size(); ind++) {
					if (boneNames[ind] == tempname) {
						break;
					}
				}
				FbxAMatrix tempMat;
				FbxAMatrix transformMatrix;
				cl->GetTransformMatrix(transformMatrix);
				cl->GetTransformLinkMatrix(tempMat);
				Skeleton[ind].bindPoseMatrix = ConvertToDirectX(tempMat * transformMatrix * geometryTransform);
				int* boneVertexInds = cl->GetControlPointIndices();
				double *boneVertexWeights = cl->GetControlPointWeights();
				int NumBoneVertInd = cl->GetControlPointIndicesCount();
				for (int BVIndex = 0; BVIndex < NumBoneVertInd; BVIndex++) {
					BoneVertInds[ind].push_back(boneVertexInds[BVIndex]);
					BoneWeights[ind].push_back((float)boneVertexWeights[BVIndex]);
				}
				//Get Animation Info (only one take whatever that means)
				FbxAnimStack* currAnimStack = Scene->GetSrcObject<FbxAnimStack>(0);
				FbxString animStackName = currAnimStack->GetName();
				CurrentAnimName = animStackName.Buffer();
				FbxTakeInfo* takeInfo = Scene->GetTakeInfo(animStackName);
				FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
				startTime = (float)start.GetSecondDouble();
				FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
				endTime = (float)end.GetSecondDouble();
				AnimLength = (unsigned int)(end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24));
				std::vector<KeyFrame> tempFrames;
				for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); i++) {
					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames24);
					KeyFrame currAnim;
					currAnim.timeStamp = (float)currTime.GetSecondDouble();
					currAnim.FrameNum = i;
					FbxAMatrix currentTransformOffset = NodeThing->EvaluateGlobalTransform(currTime) * geometryTransform;
					//Matrix Conversion here
					currAnim.GlobalTransform = ConvertToDirectX(currentTransformOffset.Inverse() * cl->GetLink()->EvaluateGlobalTransform(currTime));
					tempFrames.push_back(currAnim);
				}
				frames[ind] = tempFrames;
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
	frames.clear();
	BoneVertInds.clear();
	BoneWeights.clear();
	SocketInds.clear();
	if (!NotLoadingMeshData) {
		BoneVerts.clear();
		WeightVerts.clear();
	}
	Verts.clear();
	Normals.clear();
	UVs.clear();
	Indecies.clear();
	CompInds.clear();
	Skeleton.clear();
	boneNames.clear();
	CurrentAnimName.clear();
	AnimLength = 0;
}

void FBXExporter::FBXExport::ProcessSkeleton(FbxNode * RootNode)
{
	for (int i = 0; i < RootNode->GetChildCount(); i++) {
		FbxNode* CurrNode = RootNode->GetChild(i);
		ProcessSkeletonRecur(CurrNode, 0, 0 , -1);
	}
	BoneVertInds.resize(Skeleton.size());
	BoneWeights.resize(Skeleton.size());
	frames.resize(Skeleton.size());
}

void FBXExporter::FBXExport::ProcessSkeletonRecur(FbxNode * inNode, int inDepth, int myIndex, int inParentIndex)
{
	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Bone tempBone;
		tempBone.parentIndex = inParentIndex;
		boneNames.push_back(inNode->GetName());
		Skeleton.push_back(tempBone);
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonRecur(inNode->GetChild(i), inDepth + 1, (int)Skeleton.size(), myIndex);
	}
}

void FBXExporter::FBXExport::GetSocketIndecies()
{
	for (int i = 0; i < frames.size(); i++) {
		if (frames[i].size() <= 0) {
			SocketInds.push_back(i);
		}
	}
}

void FBXExporter::FBXExport::RemoveSockets()
{
	std::vector<std::vector<KeyFrame>> RS;
	std::vector<std::vector<int>> VS;
	std::vector<std::vector<float>> WS;
	std::vector<Bone> SS;
	std::vector<std::string> BS;
	int adj = 0;
	bool bad = false;
	for (int i = 0; i < Skeleton.size(); i++) {
		for (int e = 0; e < SocketInds.size(); e++) {
			if (i == SocketInds[e]) {
				bad = true;
			}
			if (Skeleton[i].parentIndex > SocketInds[e]) {
				adj++;
			}
		}
		if (!bad) {
			Bone tempBone = Skeleton[i];
			tempBone.parentIndex -= adj;
			SS.push_back(tempBone);
			BS.push_back(boneNames[i]);
			RS.push_back(frames[i]);
			VS.push_back(BoneVertInds[i]);
			WS.push_back(BoneWeights[i]);
		}
		bad = false;
		adj = 0;
	}
	boneNames = BS;
	Skeleton = SS;
	frames = RS;
	BoneVertInds = VS;
	BoneWeights = WS;
}

void FBXExporter::FBXExport::SetVertToBoneInds()
{
	for (int i = 0; i < Verts.size(); i++) {
		Vertexint BoneVert;
		int spot = 0;
		for (int e = 0; e < Skeleton.size(); e++) {
			if (spot >= 4) {
				break;
			}
			for (int j = 0; j < BoneVertInds[e].size(); j++) {
				if (BoneVertInds[e][j] == CompInds[i]) {
					BoneVert.pos[spot] = e;
					spot++;
					break;
				}
			}
		}
		if (spot < 4) {
			for (int e = spot; e < 4; e++) {
				BoneVert.pos[e] = -1;
			}
		}
		BoneVerts.push_back(BoneVert);
	}
}

void FBXExporter::FBXExport::SetWeightToBoneInds()
{
	for (int i = 0; i < Verts.size(); i++) {
		Vertex BoneVert;
		int spot = 0;
		for (int e = 0; e < Skeleton.size(); e++) {
			if (spot >= 4) {
				break;
			}
			for (int j = 0; j < BoneVertInds[e].size(); j++) {
				if (BoneVertInds[e][j] == CompInds[i]) {
					BoneVert.pos[spot] = BoneWeights[e][j];
					spot++;
					break;
				}
			}
		}
		if (spot < 4) {
			for (int e = spot; e < 4; e++) {
				BoneVert.pos[e] = 0;
			}
		}
		WeightVerts.push_back(BoneVert);
	}
}

void FBXExporter::FBXExport::ExportToBin(FileInfo::ExporterHeader* Header, const char * filename, const char* Fbxfilename)
{
	std::ofstream file;
	file.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	std::string TestName = filename;

	switch (Header->file) {
	case FileInfo::FILE_TYPES::MESH:
	{
		//Header stuff
		Header = new FileInfo::ExporterHeader(FileInfo::FILE_TYPES::MESH, Fbxfilename);
		Header->mesh.numPoints = (uint32_t)Verts.size();
		Header->mesh.numIndex = (uint32_t)CompInds.size();
		Header->mesh.vertSize = sizeof(Vertex);
		Header->mesh.modelType = FileInfo::MODEL_TYPES::BASIC;
		Header->mesh.index = FileInfo::INDEX_TYPES::TRI_STRIP;
		file.write((char*)Header, sizeof(*Header));

		//Export Verts
		file.write((char*)&Verts[0], Verts.size() * sizeof(Vertex));

		//Export Normals
		file.write((char*)&Normals[0], Normals.size() * sizeof(Vertex));

		//Export UVs
		file.write((char *)&UVs[0], UVs.size() * sizeof(Vertex));

		//Export CompInds
		file.write((char *)&CompInds[0], CompInds.size() * sizeof(int));

		//Export Verts Inds and Weights for each Bones
		int tempSize = (int)BoneVertInds.size();
		file.write((char*)&tempSize, sizeof(int));
		for (int i = 0; i < BoneVertInds.size(); i++) {
			tempSize = (int)BoneVertInds[i].size();
			file.write((char*)&tempSize, sizeof(int));
			if (tempSize > 0) {
				file.write((char*)&BoneVertInds[i][0], BoneVertInds[i].size() * sizeof(int));
			}
			tempSize = (int)BoneWeights[i].size();
			file.write((char*)&tempSize, sizeof(int));
			if (tempSize > 0) {
				file.write((char*)&BoneWeights[i][0], BoneWeights[i].size() * sizeof(float));
			}
		}
		//Export VertsToBones and WeigthsToBones
		tempSize = (int)BoneVerts.size();
		file.write((char*)&tempSize, sizeof(int));
		file.write((char*)&BoneVerts[0], BoneVerts.size() * sizeof(Vertexint));
		tempSize = (int)WeightVerts.size();
		file.write((char*)&tempSize, sizeof(int));
		file.write((char*)&WeightVerts[0], WeightVerts.size() * sizeof(Vertex));
		break;
	}
	case FileInfo::FILE_TYPES::BIND_POSE:
	{
		//header stuff
		Header = new FileInfo::ExporterHeader(FileInfo::FILE_TYPES::BIND_POSE, Fbxfilename);
		Header->bind.numBones = (uint32_t)Skeleton.size();
		Header->bind.nameSize = (uint32_t)CurrentAnimName.size(); //Check this when loading
		file.write((char*)Header, sizeof(*Header));
		//Export Bones
		file.write((char*)&Skeleton[0], Skeleton.size() * sizeof(Bone));
		break;
	}
	case FileInfo::FILE_TYPES::ANIMATION:
	{
		//header stuff
		Header = new FileInfo::ExporterHeader(FileInfo::FILE_TYPES::ANIMATION, Fbxfilename);
		Header->anim.numBones = (uint32_t)frames.size();
		Header->anim.numFrames = (uint32_t)AnimLength;
		Header->anim.startTime = startTime; //to be created if needed
		Header->anim.endTime = endTime; //to be created if needed
		file.write((char*)Header, sizeof(*Header));
		//Curr Animation name
		int NameSize = (int)CurrentAnimName.size();
		file.write((char*)&NameSize, sizeof(int));
		file.write(CurrentAnimName.c_str(), sizeof(CurrentAnimName.c_str())); //not currently working right^^^
		//Export animations per bone
		for (int i = 0; i < frames.size(); i++) {
			int tempSize = (int)frames[i].size();
			file.write((char*)&tempSize, sizeof(int));
			if (tempSize > 0) {
				file.write((char*)&frames[i][0], frames[i].size() * sizeof(KeyFrame));
			}
		}
		break;
	}
	case FileInfo::FILE_TYPES::NAV_MESH:
	{
		//???????
		break;
	}
	}
	file.close();
}

void FBXExporter::FBXExport::ReadInBin(FileInfo::ExporterHeader* Header, FILE* file, const char * filename)
{
	switch (Header->file) {
	case FileInfo::FILE_TYPES::MESH:
	{
		//Load Verts
		Verts.clear();
		Verts.resize(Header->mesh.numPoints);
		fread(&Verts[0], Header->mesh.numPoints * sizeof(Vertex), 1, file);
		//Load Normals
		Normals.clear();
		Normals.resize(Header->mesh.numPoints);
		fread(&Normals[0], Header->mesh.numPoints * sizeof(Vertex), 1, file);
		//Load UVs
		UVs.clear();
		UVs.resize(Header->mesh.numPoints);
		fread(&UVs[0], Header->mesh.numPoints * sizeof(Vertex), 1, file);
		//Load CompInds
		CompInds.clear();
		CompInds.resize(Header->mesh.numIndex);
		fread(&CompInds[0], Header->mesh.numIndex * sizeof(int), 1, file);
		//Generate Base JerriRigged Inds
		Indecies.clear();
		for (int i = 0; i < Verts.size(); i++) {
			Indecies.push_back(i);
		}

		//Load Bone Verts and Bone weights for bones / unload unessecary Datas
		int tempSize;
		fread(&tempSize, sizeof(int), 1, file);
		BoneVertInds.clear();
		BoneVertInds.resize(tempSize);
		BoneWeights.clear();
		BoneWeights.resize(tempSize);
		for (int i = 0; i < tempSize; i++) {
			int tempSize2;
			fread(&tempSize2, sizeof(int), 1, file);
			BoneVertInds[i].clear();
			BoneVertInds[i].resize(tempSize2);
			if (tempSize2 > 0) {
				fread(&BoneVertInds[i][0], BoneVertInds[i].size() * sizeof(int), 1, file);
			}
			fread(&tempSize2, sizeof(int), 1, file);
			BoneWeights[i].clear();
			BoneWeights[i].resize(tempSize2);
			if (tempSize2 > 0) {
				fread(&BoneWeights[i][0], BoneWeights[i].size() * sizeof(float), 1, file);
			}
		}
		tempSize;
		fread(&tempSize, sizeof(int), 1, file);
		BoneVerts.clear();
		BoneVerts.resize(tempSize);
		fread(&BoneVerts[0], BoneVerts.size() * sizeof(Vertexint), 1, file);
		fread(&tempSize, sizeof(int), 1, file);
		WeightVerts.clear();
		WeightVerts.resize(tempSize);
		fread(&WeightVerts[0], WeightVerts.size() * sizeof(Vertex), 1, file);
		break;
	}
	case FileInfo::FILE_TYPES::BIND_POSE:
	{
		//Load Bones
		Skeleton.clear();
		Skeleton.resize(Header->bind.numBones);
		fread(&Skeleton[0], Skeleton.size() * sizeof(Bone), 1, file);
		break;
	}
	case FileInfo::FILE_TYPES::ANIMATION:
	{
		startTime = Header->anim.startTime;
		endTime = Header->anim.endTime;
		//Load Curr Anim Name
		CurrentAnimName.clear();
		int NameSize;
		fread(&NameSize, sizeof(int), 1, file);
		CurrentAnimName.resize(NameSize);
		fread(&CurrentAnimName[0], sizeof(CurrentAnimName.c_str()), 1, file);
		//Load Animation per Bone
		frames.clear();
		frames.resize(Header->anim.numBones);
		AnimLength = Header->anim.numFrames;
		for (unsigned int i = 0; i < Header->anim.numBones; i++) {
			int tempSize;
			fread(&tempSize, sizeof(int), 1, file);
			frames[i].resize(tempSize);
			if (tempSize > 0) {
				fread(&frames[i][0], frames[i].size() * sizeof(KeyFrame), 1, file);
			}
		}
		break;
	}
	case FileInfo::FILE_TYPES::NAV_MESH:
	{
		//???????
		break;
	}
	}
}
