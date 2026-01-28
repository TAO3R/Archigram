// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchigramEditor.h"

#define LOCTEXT_NAMESPACE "FArchigramEditorModule"

void FArchigramEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// LoadingPhase is PostEngineInit, so the engine is fully initialized when this runs
}

void FArchigramEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FArchigramEditorModule, ArchigramEditor)
