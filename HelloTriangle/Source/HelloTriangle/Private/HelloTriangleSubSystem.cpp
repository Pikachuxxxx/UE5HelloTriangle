#include "HelloTriangleSubSystem.h"

#include "SceneViewExtension.h"

#include "HelloTriangleViewExtension.h"

void UHelloTriangleSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Display, TEXT("[HelloTriangle] Loading HelloTriangle plugin..."));
    UE_LOG(LogTemp, Warning, TEXT("[HelloTriangle] View SubSystem Init..."));

    HelloTriangleViewExtension = FSceneViewExtensions::NewExtension<FHelloTriangleViewExtension>();
}

void UHelloTriangleSubSystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("[HelloTriangle] Deinitilalizing View SubSystem..."));

    // Prevent this SVE from being gathered, in case it is kept alive by a strong reference somewhere else.
    {
        HelloTriangleViewExtension->IsActiveThisFrameFunctions.Empty();

        FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

        IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
            {
                return TOptional<bool>(false);
            };

        HelloTriangleViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
    }

    ENQUEUE_RENDER_COMMAND(ReleaseSVE)([this](FRHICommandListImmediate& RHICmdList)
        {
            // Prevent this SVE from being gathered, in case it is kept alive by a strong reference somewhere else.
            {
                HelloTriangleViewExtension->IsActiveThisFrameFunctions.Empty();

                FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

                IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
                    {
                        return TOptional<bool>(false);
                    };

                HelloTriangleViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
            }

            HelloTriangleViewExtension.Reset();
            HelloTriangleViewExtension = nullptr;
        });

    // Finish all rendering commands before cleaning up actors.
    FlushRenderingCommands();

    UE_LOG(LogTemp, Display, TEXT("[HelloTriangle] Unloading HelloTriangle plugin..."));
}
