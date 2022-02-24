#include "ComputeShaderDeclaration.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"


#include "Modules/ModuleManager.h"

//FGenGrassCSManager* FGenGrassCSManager::instance;

#define NUM_THREADS_PER_GROUP_DIMENSION 32

/// <summary>
/// Internal class thet holds the parameters and connects the HLSL Shader to the engine
/// </summary>
class FGenGrassCS : public FGlobalShader
{
public:
	//Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FGenGrassCS);
	//Tells the engine that this shader uses a structure for its parameters
	SHADER_USE_PARAMETER_STRUCT(FGenGrassCS, FGlobalShader);
	/// <summary>
	/// DECLARATION OF THE PARAMETER STRUCTURE
	/// The parameters must match the parameters in the HLSL code
	/// For each parameter, provide the C++ type, and the name (Same name used in HLSL code)
	/// </summary>
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		/*SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)*/
		/*SHADER_PARAMETER(FVector2D, Dimensions)*/
		SHADER_PARAMETER(float, TimeOffset)
		SHADER_PARAMETER(FUintVector4,g_grassPara)
		SHADER_PARAMETER_SRV(StructuredBuffer<TestStruct>,mtest)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<FVector>, g_outVertexBuffer)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<FVector>, g_outNormalBuffer)
	END_SHADER_PARAMETER_STRUCT()

public:
	//Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	//Modifies the compilations environment of the shader
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for NUM_THREADS_PER_GROUP_DIMENSION
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), THREAD_WIDTH);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), THREAD_HEIGHT);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};

// This will tell the engine to create the shader and where the shader entry point is.
//                        ShaderType              ShaderPath             Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FGenGrassCS, "/CustomShaders/GenGrassCS.usf", "MainComputeShader", SF_Compute);


//Static members
FGenGrassCSManager* FGenGrassCSManager::instance = nullptr;

//Begin the execution of the compute shader each frame
void FGenGrassCSManager::BeginRendering()
{
	//If the handle is already initalized and valid, no need to do anything
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	bCachedParamsAreValid = false;
	//Get the Renderer Module and add our entry to the callbacks so it can be executed each frame after the scene rendering is done
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FGenGrassCSManager::Execute_RenderThread);
	}
}

//Stop the compute shader execution
void FGenGrassCSManager::EndRendering()
{
	//If the handle is not valid then there's no cleanup to do
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	//Get the Renderer Module and remove our entry from the ResolvedSceneColorCallbacks
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

//Update the parameters by a providing an instance of the Parameters structure used by the shader manager
void FGenGrassCSManager::UpdateParameters(FGenGrassCSParameters& params)
{
	cachedParams = params;
	bCachedParamsAreValid = true;
}

FGenGrassCSManager::FGenGrassCSManager()
{
	mData.Init(FVector(1, 1, 1), mVertexCount);
	VertexData.Init(FVector(1, 1, 1), mVertexCount);
	VertexData.SetAllowCPUAccess(true);
	FRHIResourceCreateInfo createInfo;
	createInfo.ResourceArray = &VertexData;

	/*mRHIVertexBuffer = RHICreateStructuredBuffer()*/


	/*mRHIVertexBuffer = RHICreateStructuredBuffer(mVertexCount * sizeof(FVector), BUF_ShaderResource | BUF_UnorderedAccess, createInfo);*/

	mRHIVertexBuffer = RHICreateStructuredBuffer(sizeof(FVector), VertexData.Num() * sizeof(FVector), BUF_ShaderResource | BUF_UnorderedAccess, createInfo);
	////auto res = RHICreateStructuredBuffer(VertexData.GetStride(),VertexData.GetStride()*VertexData.Num(), BUF_StructuredBuffer | BUF_UnorderedAccess, createInfo);
	mVertexUAV = RHICreateUnorderedAccessView(mRHIVertexBuffer.GetReference(), false,false);


	mNormalData.SetNum(mVertexCount);
	mNormalRAData.SetNum(mVertexCount);
	mNormalRAData.SetAllowCPUAccess(true);
	FRHIResourceCreateInfo create_normal_info;
	create_normal_info.ResourceArray = &mNormalRAData;
	mRHINormalBuffer = RHICreateStructuredBuffer(sizeof(FVector), VertexData.Num() * sizeof(FVector), BUF_ShaderResource | BUF_UnorderedAccess, create_normal_info);
	mNormalUAV = RHICreateUnorderedAccessView(mRHINormalBuffer.GetReference(), false, false);

}



/// <summary>
/// Creates an instance of the shader type parameters structure and fills it using the cached shader manager parameter structure
/// Gets a reference to the shader type from the global shaders map
/// Dispatches the shader using the parameter structure instance
/// </summary>
void FGenGrassCSManager::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	//If there's no cached parameters to use, skip
	//If no Render Target is supplied in the cachedParams, skip
	if (!(bCachedParamsAreValid))
	{
		return;
	}

	//Render Thread Assertion
	check(IsInRenderingThread());


	//If the render target is not valid, get an element from the render target pool by supplying a Descriptor
	//if (!ComputeShaderOutput.IsValid())
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Not Valid"));
	//	FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(cachedParams.GetRenderTargetSize(), cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
	//	ComputeShaderOutputDesc.DebugName = TEXT("GenGrassCS_Output_RenderTarget");
	//	GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("GenGrassCS_Output_RenderTarget"));
	//}


	//Specify the resource transition, we're executing this in post scene rendering so we set it to Graphics to Compute
	//RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, ComputeShaderOutput->GetRenderTargetItem().UAV);
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, mVertexUAV);
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, mNormalUAV);
	//


	//Fill the shader parameters structure with tha cached data supplied by the client
	FGenGrassCS::FParameters PassParameters;
	/*PassParameters.OutputTexture = ComputeShaderOutput->GetRenderTargetItem().UAV;*/
	//PassParameters.Dimensions = FVector2D(cachedParams.GetRenderTargetSize().X, cachedParams.GetRenderTargetSize().Y);
	PassParameters.TimeOffset = cachedParams.TimeOffset;
	PassParameters.g_outVertexBuffer = mVertexUAV;
	PassParameters.g_outNormalBuffer = mNormalUAV;
	PassParameters.g_grassPara = FUintVector4(8, 16, 32, 64);
	
	/*RHICreateShaderResourceView()*/



	//Get a reference to our shader type from global shader map
	TShaderMapRef<FGenGrassCS> GenGrassCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	//Dispatch the compute shader
	FComputeShaderUtils::Dispatch(RHICmdList, GenGrassCS, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(MAX_GRASS_STRAWS_1D, THREAD_WIDTH),
			FMath::DivideAndRoundUp(MAX_GRASS_STRAWS_1D, THREAD_HEIGHT), 1));

	//FComputeShaderUtils::Dispatch(RHICmdList, GenGrassCS, PassParameters,
	//	FIntVector(FMath::DivideAndRoundUp(100, NUM_THREADS_PER_GROUP_DIMENSION),
	//		FMath::DivideAndRoundUp(100, NUM_THREADS_PER_GROUP_DIMENSION), 1));

	//Copy shader's output to the render target provided by the client
	/*RHICmdList.CopyTexture(ComputeShaderOutput->GetRenderTargetItem().ShaderResourceTexture, cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());*/

	//RHICmdList.CopyBufferRegion()

	/*RHICmdList.LockStructuredBuffer(res.GetReference(), 0, 0, RLM_ReadOnly);
	FMemory::Memcpy(dest, psource, size);*/
	
	//void* psource = RHICmdList.LockStructuredBuffer(source->StructuredBuffer, 0, size, RLM_ReadOnly);
	void* psource = RHICmdList.LockStructuredBuffer(mRHIVertexBuffer, 0, sizeof(FVector) * mVertexCount, RLM_ReadOnly);
	void* normal_source = RHICmdList.LockStructuredBuffer(mRHINormalBuffer, 0, sizeof(FVector) * mVertexCount, RLM_ReadOnly);
	FMemory::Memcpy(mData.GetData(), psource, sizeof(FVector) * mVertexCount);
	FMemory::Memcpy(mNormalData.GetData(), normal_source, sizeof(FVector) * mVertexCount);
	RHICmdList.UnlockStructuredBuffer(mRHIVertexBuffer);
	RHICmdList.UnlockStructuredBuffer(mRHINormalBuffer);

}
  