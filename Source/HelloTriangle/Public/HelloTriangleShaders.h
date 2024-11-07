#pragma once

#include "GlobalShader.h"
#include "Runtime/Renderer/Public/ScreenPass.h"

/**
 * This file defines all the resources needed for rendering a Hello Triangle
 */

 // Define VS and PS C++ classes
//----------------------------------------------------------------
BEGIN_SHADER_PARAMETER_STRUCT(FTriangleVSParams, )
END_SHADER_PARAMETER_STRUCT()
class FTriangleVS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FTriangleVS);
    using FParameters = FTriangleVSParams;
    SHADER_USE_PARAMETER_STRUCT(FTriangleVS, FGlobalShader);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
        return true;
    }
};
//----------------------------------------------------------------

BEGIN_SHADER_PARAMETER_STRUCT(FTrianglePSParams, )
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FTrianglePS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTrianglePS);
    using FParameters = FTrianglePSParams;
    SHADER_USE_PARAMETER_STRUCT(FTrianglePS, FGlobalShader)
};
//----------------------------------------------------------------
// Define shaders

IMPLEMENT_SHADER_TYPE(, FTriangleVS, TEXT("/CustomShaders/Private/HelloTriangle.usf"), TEXT("TriangleVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FTrianglePS, TEXT("/CustomShaders/Private/HelloTriangle.usf"), TEXT("TrianglePS"), SF_Pixel);

//----------------------------------------------------------------
// Render resources - buffers/textures/geometry etc.
struct FHelloVertex
{
    FVector2f Position;
    FVector4f Color;
};

class FTriangleVertexBuffer : public FVertexBuffer
{
public:
    void InitRHI(FRHICommandListBase& RHICmdList);
};

class FTriangleIndexBuffer : public FIndexBuffer
{
public:
    void InitRHI(FRHICommandListBase& RHICmdList);
};

class FTrianlgeVertexBufferElementDesc : public FRenderResource
{
public:
    FVertexDeclarationRHIRef VertexDeclarationRHI;
    virtual ~FTrianlgeVertexBufferElementDesc() {}

    virtual void InitRHI(FRHICommandListBase& RHICmdList);
    virtual void ReleaseRHI();
};

extern HELLOTRIANGLE_API TGlobalResource<FTriangleVertexBuffer> GTriangleVertexBuf;
extern HELLOTRIANGLE_API TGlobalResource<FTrianlgeVertexBufferElementDesc> GTriangleVertexBufElementDesc;
extern HELLOTRIANGLE_API TGlobalResource<FTriangleIndexBuffer> GTriangleIndexBuf;

