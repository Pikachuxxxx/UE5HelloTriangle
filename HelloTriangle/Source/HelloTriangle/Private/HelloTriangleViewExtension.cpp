#include "HelloTriangleViewExtension.h"

#include "HelloTriangleShaders.h"

#include "ScreenPass.h"
#include "PixelShaderUtils.h"
#include "PostProcess/PostProcessing.h"
#include "DynamicResolutionState.h"
#include "FXRenderingUtils.h"
#include "RenderGraphUtils.h"

// https://mcro.de/c/rdg

DEFINE_GPU_STAT(TrianglePass)

FHelloTriangleViewExtension::FHelloTriangleViewExtension(const FAutoRegister& AutoRegister)
    : FSceneViewExtensionBase(AutoRegister)
{

}

void FHelloTriangleViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
    // NO dynamic cast is possible
    checkSlow(View.bIsViewInfo);
    const FIntRect Viewport = static_cast<const FViewInfo&>(View).ViewRect;

    // Get the scene texture
    Inputs.Validate();

    const FSceneViewFamily& ViewFamily = *View.Family;

    // We need to make sure to take Windows and Scene scale into account.
    float ScreenPercentage = ViewFamily.SecondaryViewFraction;

    if (ViewFamily.GetScreenPercentageInterface())
    {
        DynamicRenderScaling::TMap<float> UpperBounds = ViewFamily.GetScreenPercentageInterface()->GetResolutionFractionsUpperBound();
        ScreenPercentage *= UpperBounds[GDynamicPrimaryResolutionFraction];
    }

    const FIntRect PrimaryViewRect = UE::FXRenderingUtils::GetRawViewRectUnsafe(View);

    FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

    if (!SceneColor.IsValid())
    {
        return;
    }

    {
        RDG_EVENT_SCOPE(GraphBuilder, "TrianglePass");
        RDG_GPU_STAT_SCOPE(GraphBuilder, TrianglePass);

        FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
        // Shader Parameter Setup, nothing for VS
        FTrianglePSParams* PixelShaderParams = GraphBuilder.AllocParameters<FTrianglePSParams>();

        // Set the Render Target In this case is the Scene Color, clear the scene RT
        PixelShaderParams->RenderTargets[0] = FRenderTargetBinding(SceneColor.Texture, ERenderTargetLoadAction::EClear);

        // Create FTriangleVS/PS Shader refs
        TShaderMapRef<FTrianglePS> PixelShader(GlobalShaderMap);
        check(PixelShader.IsValid());
        TShaderMapRef<FTriangleVS> VertexShader(GlobalShaderMap);
        check(VertexShader.IsValid());

        ClearUnusedGraphResources(PixelShader, PixelShaderParams); // ???

        // Adding the RDG pass
        GraphBuilder.AddPass(
            RDG_EVENT_NAME("HelloTriangle"),
            PixelShaderParams,
            ERDGPassFlags::Raster,
            [PixelShaderParams, PrimaryViewRect, VertexShader, PixelShader](FRHICommandList& RHICmdList)
            {
                // Create the Graphics Pipeline
                FGraphicsPipelineStateInitializer GraphicsPSOInit;
                // create stuff with default template args
                GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
                GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
                GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<>::GetRHI();
                GraphicsPSOInit.PrimitiveType = PT_TriangleList;
                GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
                GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
                GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTriangleVertexBufElementDesc.VertexDeclarationRHI;

                RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

                // Set viewport/scissor rect
                RHICmdList.SetViewport(
                    PrimaryViewRect.Min.X, PrimaryViewRect.Min.Y, 0.0f,
                    PrimaryViewRect.Max.X, PrimaryViewRect.Max.Y, 1.0f);

                RHICmdList.SetScissorRect(true, PrimaryViewRect.Min.X, PrimaryViewRect.Min.Y, PrimaryViewRect.Max.X, PrimaryViewRect.Max.Y);

                // this creates and caches the PSO using the init desc
                SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0); // no stencil ref

                // Bind shader params for each stage
                SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PixelShaderParams);

                // Bind VB/IB and draw
                RHICmdList.SetStreamSource(0, GTriangleVertexBuf.VertexBufferRHI, 0);
                RHICmdList.DrawIndexedPrimitive(GTriangleIndexBuf.IndexBufferRHI, 0, 0, 3, 0, 1, 1);
            });
    }

}

void FHelloTriangleViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass Pass, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
    //if (Pass == ISceneViewExtension::EPostProcessingPass::Tonemap)
    //{
    //    InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FHelloTriangleViewExtension::TrianglePass_RenderThread));
    //}
}
