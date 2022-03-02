// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"
#include "GenGrass/Private/ComputeShaderDeclaration.h"

#include "MyActor.generated.h"

UCLASS()
class GRASS_API AMyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyActor();

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(EditAnywhere)
		UMaterial *ss;

	UPROPERTY(EditAnywhere)
		UStaticMesh* GrassMesh;

	//UPROPERTY()
	//	USceneComponent* Root;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
	//	class UTextureRenderTarget2D* RenderTarget;

	uint32_t TimeStamp;

protected:


	struct FVertexInfo
	{
		int ID;
		FVector InstanceNormal;
		FVector2D InstanceUV;
		FVertexInfo(int InID, FVector InInstanceNormal, FVector2D InInstanceUV)
			:ID(InID), InstanceNormal(InInstanceNormal), InstanceUV(InInstanceUV)
		{
		}
	};


	const unsigned int ConstGrassVertexCount = 7;
	const unsigned int ConstGrassFaceCount = 5;
	static const unsigned int MaxArraySize = 10000;
	float ConstGrassPositionX[7] = { -0.329877, 0.329877, -0.212571, 0.212571, -0.173286, 0.173286, 0.000000 };
	float ConstGrassPositionY[7] = { 0.000000, 0.000000, 2.490297, 2.490297, 4.847759, 4.847759, 8.000000 };
	FIntVector ConstGrassIndices[5] = { {0, 2, 1}, {1, 2, 3}, {2, 4, 3}, {3, 4, 5}, {4, 6, 5} };
	FVector2D UVData[3] = { {0,1},{1,0},{0,0} };

	
	FGenGrassCSManager* GenGrassCSManager;
	TArray<FVertexID> GrassVertexIDs;
	UStaticMesh::FBuildMeshDescriptionsParams GrassBMDP;
	FMeshDescriptionBuilder GrassMeshDescBuilder;
	FMeshDescription GrassMeshDesc;
	FVector GrassVertex[MaxArraySize];
	int GrassNum;
	float TimeElapsed = 0.0f;
	FVector GrassVertexNormal[MaxArraySize],GrassFaceNorml[MaxArraySize];
	int GrassIndices[MaxArraySize];
	TArray<const FMeshDescription*> GrassMeshDescPtrs;


	
	void AppendTriangle(FMeshDescriptionBuilder& meshDescBuilder, TArray< FVertexID >& vertexIDs, FPolygonGroupID polygonGroup, TArray<FVertexInfo> vertex);
	FVector CalcFaceNorm(FVector a, FVector b, FVector c);
	void CalcNorm(int Offset);
	void GenGrassCPU(int Offset,FVector RootPos);
	void SetGrassVertex(FVector* Vertex, unsigned int VertexCount);
	virtual void BeginPlay() override;
	void InitGrassData();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
