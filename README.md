# UE5HelloTriangle
How to render a Hello Triangle in UE5 using it's RHI and FSceneViewExtensionBase class and UEngineSubSystem

**Engine Version: 5.4 Source Build**
For how to build unreal engine 5 from source, check out this blog from epic. https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source

Since we will be referencing private classes of RDG and RHI, we need to use 

## TUTORIAL
In this tutorial, we’ll cover how to render a simple triangle on the screen using Unreal Engine's RHI. We’ll implement this through a custom UEngineSubsystem and extend the rendering system with FSceneViewExtensionBase. We build this as a plugin. Unreal renderer itself is a module. Using plugins helps us keep the code organized and avoid cluttering unreal's rendering code. We can almost do anything without having to dig deeper into unreal source and get away with plugin and using the RDG (aka frame graph) for most cases. 

### Setup

[How to setup a plugin in Unreal Engine](https://www.quodsoler.com/blog/how-to-create-an-unreal-engine-plugin-a-step-by-step-guide-with-examples#:~:text=Creating%20Your%20First%20Unreal%20Engine%20Plugin&text=Navigate%20to%20the%20'Edit'%20menu,will%20depend%20on%20your%20needs.)
, this should be same for all engine versions. We start of with a basic plugin setup, we use the `PostConfigInit` phase and setup the plugin as a `Runtime` module.


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


