#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

#define MAX_GRASS_STRAWS_1D 20
#define THREAD_WIDTH 8
#define THREAD_HEIGHT 8

struct TestStruct{
	int a;
};

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
struct FGenGrassCSParameters
{

	FGenGrassCSParameters() { }
	
public:
	float TimeOffset;
	FUintVector4 GrassPara;
};


class GENGRASS_API FGenGrassCSManager
{
public:
	//Get the instance
	static FGenGrassCSManager* Get()
	{
		if (!instance)
			instance = new FGenGrassCSManager();
		return instance;
	};

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();

	// Call this whenever you have new parameters to share.
	void UpdateParameters(FGenGrassCSParameters& DrawParameters);
	void SetIndexBuffer(const FIndexBufferRHIRef& src);

	void SteRTGeo(FRayTracingGeometry* aim);

private:
	//Private constructor to prevent client from instanciating
	FGenGrassCSManager();
public:

	int mVertexCount = 6000;

	
	void TestAssign(FVertexBufferRHIRef& aim);

	//The singleton instance
	static FGenGrassCSManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	FGenGrassCSParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	/*TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;*/

	TResourceArray<FVector> VertexData;
	FStructuredBufferRHIRef mRHIVertexBuffer;
	FUnorderedAccessViewRHIRef mVertexUAV;
	TArray<FVector> mData;

	TResourceArray<FVector> mNormalRAData;
	FStructuredBufferRHIRef mRHINormalBuffer;
	FUnorderedAccessViewRHIRef mNormalUAV;
	TArray<FVector> mNormalData;

	TResourceArray<float> mTestRAData;
	FVertexBufferRHIRef  mRHITestBuffer;
	FUnorderedAccessViewRHIRef mTestUAV;
	TArray<float> mTestData;

	FIndexBufferRHIRef mTestIndexBuffer;
	UTexture2D *WindNoiseTexture;



	FRayTracingGeometrySegment mRTGeoSeg;
	FRayTracingGeometryRHIRef mRTGeoRHIRef;
	TMemoryImageArray<FRayTracingGeometrySegment> mRTGeoSegArr;
	FAccelerationStructureBuildParams mAccStructBuildPara;


	
	

public:
	void Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
};
