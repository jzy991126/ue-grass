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

	//UPROPERTY()
	//	USceneComponent* Root;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
	//	class UTextureRenderTarget2D* RenderTarget;

	uint32_t TimeStamp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	struct VertexInfo
	{
		int ID;					//对应的顶点ID
		FVector InstanceNormal;	//此三角形中顶点的法线
		FVector2D InstanceUV;	//此三角形中顶点的UV
		VertexInfo(int InID, FVector InInstanceNormal, FVector2D InInstanceUV)
			:ID(InID), InstanceNormal(InInstanceNormal), InstanceUV(InInstanceUV)
		{
		}
	};

	const unsigned int _VertexCount = 7;
	const unsigned int _FaceCount = 5;
	UStaticMesh* staticMesh;
	FGenGrassCSManager* mCSManager;

	TArray< FVertexID > mVertexIDs;
	float mGrassX[7] = { -0.329877, 0.329877, -0.212571, 0.212571, -0.173286, 0.173286, 0.000000 };
	float mGrassY[7] = { 0.000000, 0.000000, 2.490297, 2.490297, 4.847759, 4.847759, 8.000000 };

	FIntVector mGrassIndices[5] = { {0, 2, 1}, {1, 2, 3}, {2, 4, 3}, {3, 4, 5}, {4, 6, 5} };
	UStaticMesh::FBuildMeshDescriptionsParams mdParams;

	static const unsigned int MAXSIZE = 10000;

	FMeshDescriptionBuilder mMeshDescBuilder;
	FMeshDescription mMeshDesc;
	FVector mVertex[MAXSIZE];
	int mGrassCount;
	float _timecount = 0.0f;

	FVector mNormal[MAXSIZE],mFaceNorml[MAXSIZE];
	int mIndices[MAXSIZE];
	TArray<const FMeshDescription*> mMeshDescPtrs;

	FVector2D mUVs[20];

	void AppendTriangle(FMeshDescriptionBuilder& meshDescBuilder, TArray< FVertexID >& vertexIDs, FPolygonGroupID polygonGroup, TArray<VertexInfo> vertex);

	FVector CalcFaceNorm(FVector a, FVector b, FVector c);
	void CalcNorm(int offset);
	void GenGrass(int offset,FVector GenGrass);
	void SetVertex(FVector* vertex, unsigned int count);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
