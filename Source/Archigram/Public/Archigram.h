// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FArchigramModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** 
	 * Spawns the BP_PCG actor at the specified location.
	 * Will not spawn if an actor already exists (check with GetSpawnedPCGActor()).
	 * @param Location - World location to spawn the actor (default: origin)
	 * @return The spawned actor, or nullptr if spawn failed or actor already exists
	 */
	static AActor* SpawnPCGActor(FVector Location = FVector::ZeroVector);

	/**
	 * Gets the currently spawned PCG actor, if it still exists.
	 * @return The spawned actor, or nullptr if none exists or it was deleted
	 */
	static AActor* GetSpawnedPCGActor();

	/**
	 * Checks if a PCG actor has been spawned and still exists in the level.
	 * @return True if actor exists, false if not spawned or deleted
	 */
	static bool HasSpawnedPCGActor();

	/**
	 * Clears the reference to the spawned PCG actor (useful for manual reset).
	 */
	static void ClearSpawnedPCGActorReference();

private:
	/** Register custom Slate style (icons) */
	void RegisterStyleSet();
	void UnregisterStyleSet();

	/** Register menu bar menus when ToolMenus system is ready */
	void RegisterMenuBarMenus();

	/** Register main toolbar button */
	void RegisterToolbarButton();

	/** Test function that outputs to log */
	static void ExecutePipelineTestLog();

	/** Toolbar button action - spawns the PCG actor */
	static void ExecuteToolbarAction();

	/** Custom style set for icons */
	TSharedPtr<FSlateStyleSet> StyleSet;

	/** Path to the BP_PCG Blueprint actor */
	static const TCHAR* PCGActorBlueprintPath;

	/** 
	 * Weak pointer to track the spawned PCG actor.
	 * Automatically becomes invalid when the actor is deleted from the level.
	 */
	static TWeakObjectPtr<AActor> SpawnedPCGActor;

	/** 
	 * Called when a map/level is opened in the editor.
	 * Clears current reference and searches for existing PCG actor in the new level.
	 */
	void OnMapOpened(const FString& Filename, bool bAsTemplate);

	/**
	 * Searches the current level for an existing PCG actor (BP_PCG) and updates the reference.
	 * @return The found actor, or nullptr if none exists
	 */
	static AActor* FindExistingPCGActorInLevel();

	/** Set the collision type of the HDA actor's mesh component to `default` everytime the actor get cooked in the scene */
	static void SetHDAMeshCollisionTypeToDefault(const FName& PackageName, EPackageFlags PackageFlags, const FString& PackageFileName, const FString& AssetPackageName);
};
