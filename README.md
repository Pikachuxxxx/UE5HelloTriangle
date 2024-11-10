# UE5HelloTriangle
How to render a Hello Triangle in UE5 using it's RHI and FSceneViewExtensionBase class and UEngineSubSystem.

**Engine Version: 5.4 Source Build**


## TUTORIAL
In this tutorial, we’ll cover how to render a simple triangle on the screen using Unreal Engine's RHI. We’ll implement this through a custom UEngineSubsystem and extend the rendering system with FSceneViewExtensionBase. We build this as a plugin. Unreal renderer itself is a module. Using plugins helps us keep the code organized and avoid cluttering unreal's rendering code. We can almost do anything without having to dig deeper into unreal source and get away with plugin and using the RDG (aka frame graph) for most cases. 

### Setup

Since we will be referencing private classes of RDG and RHI, we need to use a source build for this. For how to build unreal engine 5 from source, check out this blog from epic. https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source

[How to setup a plugin in Unreal Engine](https://www.quodsoler.com/blog/how-to-create-an-unreal-engine-plugin-a-step-by-step-guide-with-examples#:~:text=Creating%20Your%20First%20Unreal%20Engine%20Plugin&text=Navigate%20to%20the%20'Edit'%20menu,will%20depend%20on%20your%20needs.)
, this should be same for all engine versions. We start of with a basic plugin setup, we use the `PostConfigInit` phase and setup the plugin as a `Runtime` module. Go ahead and setup a plugin called HelloTriangle or call it what you like. `Runtime` tells us that this is an Engine plugin and `PostConfigInit` tell us the initialization stage of this plugin during the engine startup. Check the above blog for more info.

## Plugin Setup
Add the private files to be visibile to the plugin by adding it to the Build.cs file called `HelloTriangle.Build.cs` file in your plugin Source folder.
```C#
 PublicIncludePaths.AddRange(
    new string[] {
        EngineDirectory + "/Source/Runtime/Renderer/Private"
    }
);
```

Next add the following **Public** modules to link to: `RHI`, `RenderCore`, `Renderer` and `Projects` to the build file
```C#
PublicDependencyModuleNames.AddRange(
    new string[]
    {
        "Core",
        "RHI",
        "Renderer",
        "RenderCore",
        "Projects"
    }
);
```
Now your build file for the plugin should be ready. If you need to link with any other private/public modules you can modify the `HelloTriangle.Build.cs` accordingly. 

We also need to do some setup when the module loads:
- Setup a Virtual File System (VFS) link to the plugin shaders directory to load and give a __unique__ name to it.

Remember how we stress unique, if you have multiple plugins they can't use the same VFS mapping. And also make sure the Unique VFS starts with / before the name otherwise the engine can't see this is a directory and will cause a crash.


```C++
void FHelloTriangleModule::StartupModule()
{
    // Register a custom VFS link to plugin shaders
    FString baseDir = IPluginManager::Get().FindPlugin(TEXT("HelloTriangle"))->GetBaseDir();
    FString pluginShaderBaseDir = FPaths::Combine(baseDir, TEXT("Shaders"));
    // This is a virtual folder and can be the same name as OG or anything.
    AddShaderSourceDirectoryMapping(TEXT("/CustomShaders"), pluginShaderBaseDir);
}
```

Now you're done with the plugin setup, onto the next steps we go. SWOOSH!!

## Setting up SceneViewExtensionBase
Okay now we got the engine to launch and load the `Hello Triangle` plugin and created Virtual File System links to load shaders/resources etc. we can focus on the Rendering code now. Instead of directly modifying engine source, unreal provides us with `FSceneViewExtensionBase` class that when inherited from helps us inject rnedering code using the RDG (Render Dependency Graph) or directly via writing into the Command Buffer (`FRHICommandList`). So we create a .h/.cpp files called `FHelloTriangleViewExtension` and derive it from `FSceneViewExtensionBase`.

> Note: If you're getting build errors, make sure you include the right headers, check the source and engine for what header files you might need to include from time to time.

We override the following methods from the ISceneViewExtensionBase interface. We only focus on `PrePostProcess_RenderThread` function for now. Our goal currently is to draw a trianlge before post process and after the lightting pass has been done. Check the source for ex. PostProcessing.cpp and other files on what function you can use to inject, you can inject your own RDG render pass at various stages of the rendering pipeline based on the functions in `ISceneViewExtensionBase` class, just check the function names that end with _RenderThread. You shoudl be able to do it before/after lighting/depth pases, before/after post processing and in b/w post processing stages by using the `SubscribePostProcssing` function. For the sake of this tutorial we will be just working with `PrePostProcess_RenderThread` function so we override only that. Check docs/source for what each virtual function does.

HelloTriangleViewExtension.h class

```C++
#include "SceneViewExtension.h"
#include "RenderResource.h"

class HELLOTRIANGLE_API FHelloTriangleViewExtension : public FSceneViewExtensionBase
{
public:
    FHelloTriangleViewExtension(const FAutoRegister& AutoRegister);

	//~ Begin FSceneViewExtensionBase Interface
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
	//~ End FSceneViewExtensionBase Interface
};
```
HELLOTRIANGLE_API is defines by the generated headers so it's typically looks like YOURMODULENAME_API. This define makes this class publicly visibile.

we will discuss the HelloTriangleViewExtension.cpp implementation later. Let's next focus on setting up the render resources for drawing our triangle, we need:
- Shaders (.usf and C++ equivalent classes)
- Vertex and Index Buffers
Triangle doesn't need a index buffer but we used to demonstrate how to bind one.

## Setting up Shaders
Create a new folder in the same dfirectory as Source. Add another folder called Private to it and place an empty file called HelloTriangle.usf (.usf = Unreal Shader File). This is the shader that will be compiled and loaded by the engine. This single file contains both the Vertex and Pixel Shader code. USF is very similar to HLSL, consided it a super set with some unreal specific macros and functions to make writing HLSL shaders for unreal easy.

```HLSL
#include "/Engine/Public/Platform.ush"
#include "/Engine/Private/Common.ush"
#include "/Engine/Private/ScreenPass.ush"
#include "/Engine/Private/PostProcessCommon.ush"

// VS main
void TriangleVS( in float2 InPosition : ATTRIBUTE0, in float4 InColor : ATTRIBUTE1, out float4 OutPosition : SV_POSITION , out float4 OutColor : COLOR0)
{
	OutPosition = float4(InPosition, 0, 1);
	OutColor = InColor;
}

// PS main
void TrianglePS(in float4 InPosition : SV_POSITION, in float4 InColor : COLOR0, out float4 OutColor : SV_Target0)
{
	OutColor = InColor;
}
```
What this shader is doing is it's taking a vertex as input and writing some attributes to be read by the Pixel shader. 
The Vertex is of the format:
```C++
struct FHelloVertex
{
    FVector2f Position;
    FVector4f Color;
};
```

Now is a great time to create 2 new files called HelloTriangleShader.h/cpp these will contain the code to create the shaders and other rendering resources like Vertex/index buffers etc.

Add this struct to the the file, we will use this later to fill the vertex buffer with vertices data.

Now before we craete render resources let's start with the Shader C++ code. We need to define class for each sahder stage that derives from `FGlobalShader`. This is how it looks.

Vertex Shader
```C++
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
```
Since we have no uniforms and bindinable resources the BEGIN_SHADER_PARAMETER_STRUCT and END_SHADER_PARAMETER_STRUCT doesn't define any parameters. This is how C++ and HLSL talk about dindable parameters in the shader.

Pixel Shader
```C++
BEGIN_SHADER_PARAMETER_STRUCT(FTrianglePSParams, )
    RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FTrianglePS : public FGlobalShader
{
    DECLARE_GLOBAL_SHADER(FTrianglePS);
    using FParameters = FTrianglePSParams;
    SHADER_USE_PARAMETER_STRUCT(FTrianglePS, FGlobalShader)
};
```
We define RENDER_TARGET_BINDING_SLOTS() in the Pixel Shader to tell that we have output render targets to write to. Next go ahead and define the shader implementation.
```C++
IMPLEMENT_SHADER_TYPE(, FTriangleVS, TEXT("/CustomShaders/Private/HelloTriangle.usf"), TEXT("TriangleVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FTrianglePS, TEXT("/CustomShaders/Private/HelloTriangle.usf"), TEXT("TrianglePS"), SF_Pixel);
```
`IMPLEMENT_SHADER_TYPE` takes in the Shader Class, The location of the shader file (this is a Virtual File Path, relative to the folder you maped during plugin initialization.), then the Main function to call to the in shader file, in our case we named it TriangleVS and TrianglePS for each shader stage and then finally what shader stage it is, pixel, vertex, compute, geometry etc.

## Setting up render resources

Now that we have fonned the shaders ready and setup let's now create the rendering resources, we will need a VertexBuffer to hold the Trinagle Vertex Data, another resource to tell the Vertex Element layout (this defines how the resources are layed out and passed to the GPU) and finally a Index buffer (used only for demo purposes).

Add these to the HelloTriangleShaders.h
```C++
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
```
Now the C++ file looks like this.
HelloTriangleShaders.cpp

```C++
TGlobalResource<FTriangleVertexBuffer> GTriangleVertexBuf;
TGlobalResource<FTrianlgeVertexBufferElementDesc> GTriangleVertexBufElementDesc;
TGlobalResource<FTriangleIndexBuffer> GTriangleIndexBuf;

void FTriangleVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
    TResourceArray<FHelloVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    Vertices.SetNumUninitialized(3);
    Vertices[0].Position = FVector2f(0.0f, 0.75f);
    Vertices[0].Color = FVector4f(1, 0, 0, 1);
    Vertices[1].Position = FVector2f(0.75, -0.75);
    Vertices[1].Color = FVector4f(0, 1, 0, 1);
    Vertices[2].Position = FVector2f(-0.75, -0.75);
    Vertices[2].Color = FVector4f(0, 0, 1, 1);
    FRHIResourceCreateInfo CreateInfo(TEXT("FScreenRectangleVertexBuffer"), &Vertices);
    VertexBufferRHI = RHICmdList.CreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
}

void FTriangleIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
    const uint16 Indices[] = { 0, 1, 2 };
    TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
    uint32 NumIndices = UE_ARRAY_COUNT(Indices);
    IndexBuffer.AddUninitialized(NumIndices);
    FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));
    FRHIResourceCreateInfo CreateInfo(TEXT("FTriangleIndexBuffer"), &IndexBuffer);
    IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}

void FTrianlgeVertexBufferElementDesc::InitRHI(FRHICommandListBase& RHICmdList)
{
    FVertexDeclarationElementList Elements;
    uint16 Stride = sizeof(FHelloVertex);
    Elements.Add(FVertexElement(0, STRUCT_OFFSET(FHelloVertex, Position), VET_Float2, 0, Stride));
    Elements.Add(FVertexElement(0, STRUCT_OFFSET(FHelloVertex, Color), VET_Float4, 1, Stride));
    VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
}
```

Here we create and overide their Init methods to create the resources.

## Setting up the rendering code using RDG pass
Now we have everything we need to start with rndering let's return to the HelloTriangleSceneViewExtension.cpp class and start using the RDG to create a new pass to render out triangle.

Step1: Get the input scene texture to draw onto, use inject the rendering before anykind of post processing is done, which is why re override the function `PrePostProcessPass_RenderThread` in our FHelloTriangleViewExtension class.

```C++
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

```
No we have done the setup we start with adding a new pass called "Triangle Pass" to the RDG.
- We create markers for GPU debugging
- Create the VS/PS shader instance using the GlobalShaderMap
- Bind the RenderTarget to the PixelShaderParams, in this case render to the scene texture, we we bind the scene color texture, and we clear it at start.
- Now that the shaders are bound add a pass to the RDG using `GraphBuilder.AddPass`, make sure you pass the necessary params to the AddPass pambda function that you want to refernce later during the execution phase.

Coming to the rendering code:
 - Create the PSO:
   - bind shaders
   - specify DepthStencil, Rasterization, BlendState, MSAA etc params
   - Specify the Shaders to use bind both the VS and PS
   - Specofy the primitive type, in this case we use triangle list
- Set the viewport and scissor rect
- Bind the PSO to Command List
- Bind the shader params for each shader stage in this case only the PS
- Issue draw calls, we only draw 3 vertices for the triangle
```C++
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
```
And voila! you have a Triangle!!!

## One last thing
You have successfull created the SceneViewExtension class but who calls them? The USubsystem we created at the beginnign has a Initializae function that is called automatically.
Add this to it and instantiate it.
```C++
HelloTriangleViewExtension = FSceneViewExtensions::NewExtension<FHelloTriangleViewExtension>();
```
### Note about UEngineSubsystem:
- https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?application_version=5.4
> UEngineSubsystems are dynamic and will be initialized when the module is loaded if necessary. This means that after StartupModule() is called on the module containing a subsystem, the subsystem collection with instantiate and initialize the subsystem **automatically**. If the subsystem collection is created post module load then the instances will be created at collection initialization time.

## Closing notes
If you have any queries or suggestions or fixes for the tutorial please feel free to open and issues.

## Demo
![UE5HelloTriangle](https://github.com/user-attachments/assets/c1a83bf0-0d7a-42ef-b513-e05f1821292e)
<img src="https://github.com/user-attachments/assets/dc7f6a13-0598-4dbc-88cf-8b1f9936bfc4" alt="drawing" height="400"/>


## References
- https://itscai.us/blog/post/ue-view-extensions/
- https://dev.epicgames.com/community/learning/knowledge-base/0ql6/unreal-engine-using-sceneviewextension-to-extend-the-rendering-system
- https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine
- https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-subsystems-in-unreal-engine
