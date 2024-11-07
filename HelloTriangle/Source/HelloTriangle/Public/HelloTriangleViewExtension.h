#pragma once

#include "SceneViewExtension.h"
#include "RenderResource.h"

DECLARE_GPU_STAT_NAMED_EXTERN(TrianglePass, TEXT("TrianglePass"))

class HELLOTRIANGLE_API FHelloTriangleViewExtension : public FSceneViewExtensionBase
{
public:
    FHelloTriangleViewExtension(const FAutoRegister& AutoRegister);

	//~ Begin FSceneViewExtensionBase Interface
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	virtual void SubscribeToPostProcessingPass(EPostProcessingPass Pass, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled);
	//~ End FSceneViewExtensionBase Interface

private:
	
};