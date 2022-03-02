// Fill out your copyright notice in the Description page of Project Settings.



#include "MyActor.h"

// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	// init variables

	GenGrassCSManager = FGenGrassCSManager::Get();
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("static_mesh"), false);
	TimeStamp = 0;
	SetRootComponent(StaticMeshComp);
	GrassMesh = CreateDefaultSubobject<UStaticMesh>("test");
	FStaticMeshAttributes Attributes(GrassMeshDesc);
	Attributes.Register();
	GrassMeshDescBuilder.SetMeshDescription(&GrassMeshDesc);
	GrassMeshDescBuilder.EnablePolyGroups();
	GrassMeshDescBuilder.SetNumUVLayers(1);


	GrassNum = 400;

	for (int i = 0; i < GrassNum; i++) {
		GenGrassCPU(i, FVector(i * 10 ,0,0));
	}


	// generate grass mesh via meshdescbuilder
	{
		GrassVertexIDs.SetNum(GrassNum*ConstGrassVertexCount);

		for (unsigned int i = 0; i < GrassNum * ConstGrassVertexCount; i++) {
			GrassVertexIDs[i] = GrassMeshDescBuilder.AppendVertex(GrassVertex[i]);
		}

		FPolygonGroupID polygonGroup = GrassMeshDescBuilder.AppendPolygonGroup();

		for (unsigned int i = 0; i < GrassNum * ConstGrassFaceCount; i++) {
			TArray<FVertexInfo> infos;
			for (unsigned int j = 0; j < 3; j++) {
				infos.Push(FVertexInfo(GrassIndices[i * 3 + j], GrassVertexNormal[GrassIndices[i * 3 + j]], UVData[j]));
			}
			AppendTriangle(GrassMeshDescBuilder, GrassVertexIDs, polygonGroup, infos);
		}
	}
	
	GrassMeshDescPtrs.Emplace(&GrassMeshDesc);

	GrassMesh->BuildFromMeshDescriptions(GrassMeshDescPtrs, GrassBMDP);

	// set static mesh
	StaticMeshComp->SetStaticMesh(GrassMesh);
	

	

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	GenGrassCSManager->BeginRendering();
	GenGrassCSManager->TestAssign(GrassMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.VertexBufferRHI);
	GenGrassCSManager->SetIndexBuffer(GrassMesh->RenderData->LODResources[0].IndexBuffer.IndexBufferRHI);
	GenGrassCSManager->SteRTGeo(&GrassMesh->RenderData->LODResources[0].RayTracingGeometry);

	//GenGrassCSManager->TestAssign(GrassMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer.VertexBufferRHI);
	UMaterialInstanceDynamic* MID = StaticMeshComp->CreateAndSetMaterialInstanceDynamic(0);
	//MID->SetTextureParameterValue("InputTexture", (UTexture*)RenderTarget);
	GrassMesh->SetMaterial(0, ss);
	StaticMeshComp->SetStaticMesh(GrassMesh);
	StaticMeshComp->bEvaluateWorldPositionOffset=true;
}

void AMyActor::InitGrassData()
{


}

void AMyActor::AppendTriangle(FMeshDescriptionBuilder& meshDescBuilder, TArray<FVertexID>& vertexIDs, FPolygonGroupID polygonGroup, TArray<FVertexInfo> vertex)
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

void AMyActor::GenGrassCPU(int offset,FVector rootPos)
{
	FVector inNormal(0, 0, 1);
	float scale = 10.0f;

	const int vertexOffset = offset * ConstGrassVertexCount;

	FVector2D noiseuv(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f));
	
	for (int i = 0; i < 7; i++) {
		float bendInterp = pow(ConstGrassPositionY[i] / ConstGrassPositionY[6], 1.5);
		FVector y = scale * inNormal * ConstGrassPositionY[i];
		FVector xz = scale * (ConstGrassPositionX[i] * FVector(1, 1, 1) + bendInterp *FVector(1,1,1));
		GrassVertex[vertexOffset +i] = rootPos + y + xz;
	}

	const int faceOffset = offset * ConstGrassFaceCount;
	const int indexOffset = offset * ConstGrassFaceCount * 3;

	for (int i = 0; i < 5; i++) {
		GrassIndices[indexOffset +i*3] = ConstGrassIndices[i].X+ vertexOffset;
		GrassIndices[indexOffset +i*3+1] = ConstGrassIndices[i].Y + vertexOffset;
		GrassIndices[indexOffset +i*3+2] = ConstGrassIndices[i].Z + vertexOffset;
	}
	for (int i = 0; i < 5; i++) {
		GrassFaceNorml[faceOffset +i] = CalcFaceNorm(GrassVertex[vertexOffset+ ConstGrassIndices[i].X], GrassVertex[vertexOffset+ ConstGrassIndices[i].Y], GrassVertex[vertexOffset+ ConstGrassIndices[i].X]);
	}


	GrassVertexNormal[vertexOffset + 0] = GrassFaceNorml[vertexOffset];
	GrassVertexNormal[vertexOffset + 1] = 0.5f * (GrassFaceNorml[vertexOffset] + GrassFaceNorml[vertexOffset + 1]);
	GrassVertexNormal[vertexOffset + 2] = (1.f / 3) * (GrassFaceNorml[vertexOffset + 0] + GrassFaceNorml[vertexOffset + 1] + GrassFaceNorml[vertexOffset + 2]);
	GrassVertexNormal[vertexOffset + 3] = (1.f / 3) * (GrassFaceNorml[vertexOffset + 1] + GrassFaceNorml[vertexOffset + 2] + GrassFaceNorml[vertexOffset + 3]);
	GrassVertexNormal[vertexOffset + 4] = (1.f / 3) * (GrassFaceNorml[vertexOffset + 2] + GrassFaceNorml[vertexOffset + 3] + GrassFaceNorml[vertexOffset + 4]);
	GrassVertexNormal[vertexOffset + 5] = 0.5f * (GrassFaceNorml[vertexOffset + 3] + GrassFaceNorml[vertexOffset + 4]);
	GrassVertexNormal[vertexOffset + 6] = 0.5f * GrassFaceNorml[vertexOffset + 4];
	for (int i = 0; i < 7; i++) {
		GrassVertexNormal[vertexOffset + i].Normalize();
	}


}

void AMyActor::SetGrassVertex(FVector* vertex, unsigned int count)
{
	for (unsigned int i = 0; i < count*3; i+=3) {

		GrassMeshDescBuilder.SetPosition(GrassVertexIDs[i], vertex[i]);
	}

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeElapsed += DeltaTime;

	
	//GrassMesh->RenderData->LODResources[0].VertexBuffers.StaticMeshVertexBuffer.TangentsVertexBuffer.VertexBufferRHI

	FGenGrassCSParameters parameters;
	TimeStamp++;
	parameters.TimeOffset = TimeElapsed;
	GenGrassCSManager->UpdateParameters(parameters);
	//GenGrassCSManager->SteRTGeo(&GrassMesh->RenderData->LODResources[0].RayTracingGeometry);

	//for (int i = 0; i < GrassNum; i++) {
	//	GenGrassCPU(i, FVector(i * 10, 0, TimeElapsed*10));
	//}

	//SetGrassVertex(GrassVertex, GrassNum * ConstGrassVertexCount);

	//GrassMesh->BuildFromMeshDescriptions(GrassMeshDescPtrs, GrassBMDP);

	
	StaticMeshComp->MarkRenderStateDirty();
	StaticMeshComp->MarkRenderDynamicDataDirty();


	

}

