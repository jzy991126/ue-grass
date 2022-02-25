// Fill out your copyright notice in the Description page of Project Settings.



#include "MyActor.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	mCSManager = FGenGrassCSManager::Get();

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("static_mesh"), false);


	TimeStamp = 0;


	SetRootComponent(StaticMeshComp);

	staticMesh = CreateDefaultSubobject<UStaticMesh>("test");

	FStaticMeshAttributes Attributes(mMeshDesc);
	Attributes.Register();

	
	mMeshDescBuilder.SetMeshDescription(&mMeshDesc);
	mMeshDescBuilder.EnablePolyGroups();
	mMeshDescBuilder.SetNumUVLayers(1);

	mUVs[0] = FVector2D(0, 1);
	mUVs[1] = FVector2D(1, 0);
	mUVs[2] = FVector2D(0, 0);

	mGrassCount = 20;

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			GenGrass(i*10+j, FVector(i*10, j*10, i+j));
		}
	}
	{
		mVertexIDs.SetNum(mGrassCount*_VertexCount);

		for (unsigned int i = 0; i < mGrassCount * _VertexCount; i++) {
			mVertexIDs[i] = mMeshDescBuilder.AppendVertex(mVertex[i]);
		}

		FPolygonGroupID polygonGroup = mMeshDescBuilder.AppendPolygonGroup();

		for (unsigned int i = 0; i < mGrassCount * _FaceCount; i++) {
			TArray<VertexInfo> infos;
			for (unsigned int j = 0; j < 3; j++) {
				infos.Push(VertexInfo(mIndices[i * 3 + j], mNormal[mIndices[i * 3 + j]], mUVs[j]));
			}
			AppendTriangle(mMeshDescBuilder, mVertexIDs, polygonGroup, infos);
		}
	}
	
	mdParams.bBuildSimpleCollision = false;

	
	mMeshDescPtrs.Emplace(&mMeshDesc);

	staticMesh->BuildFromMeshDescriptions(mMeshDescPtrs, mdParams);

	// 将 StaticMesh 指定给 StaticMeshComponent组件
	StaticMeshComp->SetStaticMesh(staticMesh);
	

	

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	mCSManager->BeginRendering();

	

	UMaterialInstanceDynamic* MID = StaticMeshComp->CreateAndSetMaterialInstanceDynamic(0);
	//MID->SetTextureParameterValue("InputTexture", (UTexture*)RenderTarget);
}

void AMyActor::AppendTriangle(FMeshDescriptionBuilder& meshDescBuilder, TArray<FVertexID>& vertexIDs, FPolygonGroupID polygonGroup, TArray<VertexInfo> vertex)
{
			TArray<FVertexInstanceID > vertexInsts;//三角形中的每个顶点
			for (int i = 0; i < 3; i++)
			{
				FVertexInstanceID instance = meshDescBuilder.AppendInstance(vertexIDs[vertex[i].ID]);
				meshDescBuilder.SetInstanceNormal(instance, vertex[i].InstanceNormal);			
				meshDescBuilder.SetInstanceUV(instance, vertex[i].InstanceUV, 0);				
				meshDescBuilder.SetInstanceColor(instance, FVector4(1.0f, 1.0f, 1.0f, 1.0f));	
				vertexInsts.Add(instance);
			}
			meshDescBuilder.AppendTriangle(vertexInsts[0], vertexInsts[1], vertexInsts[2], polygonGroup);
}

FVector AMyActor::CalcFaceNorm(FVector a, FVector b, FVector c)
{
	auto res = FVector::CrossProduct(b - a, c - a);
	return res.GetSafeNormal();
}

void AMyActor::CalcNorm(int offset)
{
	
}

void AMyActor::GenGrass(int offset,FVector rootPos)
{
	FVector inNormal(0, 0, 1);
	float scale = 10.0f;

	const int vertexOffset = offset * _VertexCount;

	FVector2D noiseuv(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f));
	
	for (int i = 0; i < 7; i++) {
		float bendInterp = pow(mGrassY[i] / mGrassY[6], 1.5);
		FVector y = scale * inNormal * mGrassY[i];
		FVector xz = scale * (mGrassX[i] * FVector(1, 1, 1) + bendInterp *FVector(1,1,1));
		mVertex[vertexOffset +i] = rootPos + y + xz;
	}

	const int faceOffset = offset * _FaceCount;
	const int indexOffset = offset * _FaceCount * 3;

	for (int i = 0; i < 5; i++) {
		mIndices[indexOffset +i*3] = mGrassIndices[i].X+ vertexOffset;
		mIndices[indexOffset +i*3+1] = mGrassIndices[i].Y + vertexOffset;
		mIndices[indexOffset +i*3+2] = mGrassIndices[i].Z + vertexOffset;
	}
	for (int i = 0; i < 5; i++) {
		mFaceNorml[faceOffset +i] = CalcFaceNorm(mVertex[vertexOffset+ mGrassIndices[i].X], mVertex[vertexOffset+ mGrassIndices[i].Y], mVertex[vertexOffset+ mGrassIndices[i].X]);
	}


	mNormal[vertexOffset + 0] = mFaceNorml[vertexOffset];
	mNormal[vertexOffset + 1] = 0.5f * (mFaceNorml[vertexOffset] + mFaceNorml[vertexOffset + 1]);
	mNormal[vertexOffset + 2] = (1.f / 3) * (mFaceNorml[vertexOffset + 0] + mFaceNorml[vertexOffset + 1] + mFaceNorml[vertexOffset + 2]);
	mNormal[vertexOffset + 3] = (1.f / 3) * (mFaceNorml[vertexOffset + 1] + mFaceNorml[vertexOffset + 2] + mFaceNorml[vertexOffset + 3]);
	mNormal[vertexOffset + 4] = (1.f / 3) * (mFaceNorml[vertexOffset + 2] + mFaceNorml[vertexOffset + 3] + mFaceNorml[vertexOffset + 4]);
	mNormal[vertexOffset + 5] = 0.5f * (mFaceNorml[vertexOffset + 3] + mFaceNorml[vertexOffset + 4]);
	mNormal[vertexOffset + 6] = 0.5f * mFaceNorml[vertexOffset + 4];
	for (int i = 0; i < 7; i++) {
		mNormal[vertexOffset + i].Normalize();
	}


}

void AMyActor::SetVertex(FVector* vertex, unsigned int count)
{
	for (unsigned int i = 0; i < count; i++) {
		mMeshDescBuilder.SetPosition(mVertexIDs[i], vertex[i]);
	}

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	_timecount += DeltaTime;

	FGenGrassCSParameters parameters;
	TimeStamp++;
	parameters.TimeOffset = _timecount;
	mCSManager->UpdateParameters(parameters);


	//for (int i = 0; i < 10; i++) {
	//	for (int j = 0; j < 10; j++) {
	//		GenGrass(i * 10 + j, FVector(i * 10+ DeltaTime*100, j * 10+ DeltaTime * 100, _timecount));
	//	}
	//}
	SetVertex(mCSManager->mData.GetData(), mGrassCount * _VertexCount);

	staticMesh->BuildFromMeshDescriptions(mMeshDescPtrs, mdParams);
	staticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.GetVertexData();
	staticMesh->SetMaterial(0, ss);


	// 将 StaticMesh 指定给 StaticMeshComponent组件
	StaticMeshComp->SetStaticMesh(staticMesh);


	

}

