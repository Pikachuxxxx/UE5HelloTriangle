// Copyright Epic Games, Inc. All Rights Reserved.
#include "HelloTriangle.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FHelloTriangleModule"

void FHelloTriangleModule::StartupModule()
{
    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

    // Register a custom VFS link to plugin shaders
    FString baseDir = IPluginManager::Get().FindPlugin(TEXT("HelloTriangle"))->GetBaseDir();
    FString pluginShaderBaseDir = FPaths::Combine(baseDir, TEXT("Shaders"));
    // This is a virtual folder and can be the same name as OG or anything.
    AddShaderSourceDirectoryMapping(TEXT("/CustomShaders"), pluginShaderBaseDir);

    // TODO: Init the subsystem here
}

void FHelloTriangleModule::ShutdownModule()
{
    // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
    // we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FHelloTriangleModule, HelloTriangle)