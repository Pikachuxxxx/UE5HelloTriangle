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

Now is a greate time to create 2 new files called HelloTriangleShader.h/cpp these will contain the code to create the shaders and other rendering resources like Vertex/index buffers etc.

Add this struct to the the file, we will use this later to fill the vertex buffer with vertices data.




## Demo
![UE5HelloTriangle](https://github.com/user-attachments/assets/c1a83bf0-0d7a-42ef-b513-e05f1821292e)
<img src="https://github.com/user-attachments/assets/dc7f6a13-0598-4dbc-88cf-8b1f9936bfc4" alt="drawing" height="400"/>

## Notes:
- https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Subsystems/UEngineSubsystem?application_version=5.4
> UEngineSubsystems are dynamic and will be initialized when the module is loaded if necessary. This means that after StartupModule() is called on the module containing a subsystem, the subsystem collection with instantiate and initialize the subsystem **automatically**. If the subsystem collection is created post module load then the instances will be created at collection initialization time.

## References
- https://itscai.us/blog/post/ue-view-extensions/
- https://dev.epicgames.com/community/learning/knowledge-base/0ql6/unreal-engine-using-sceneviewextension-to-extend-the-rendering-system
- https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine
- https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-subsystems-in-unreal-engine


