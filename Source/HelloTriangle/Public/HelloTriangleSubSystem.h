#pragma once

#include "CoreMinimal.h"
#include "SubSystems/EngineSubsystem.h"

#include "HelloTriangleSubSystem.generated.h" //???

class FHelloTriangleViewExtension;

UCLASS()
class UHelloTriangleSubSystem : public UEngineSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    TSharedPtr<FHelloTriangleViewExtension, ESPMode::ThreadSafe> HelloTriangleViewExtension;
};
