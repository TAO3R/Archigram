// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"

class FArchigramModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

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

	/** Toolbar button action */
	static void ExecuteToolbarAction();

	/** Custom style set for icons */
	TSharedPtr<FSlateStyleSet> StyleSet;

	/** Set the collision type of the HDA actor's mesh component to `default` everytime the actor get cooked in the scene */
	static void SetHDAMeshCollisionTypeToDefault(const FName& PackageName, EPackageFlags PackageFlags, const FString& PackageFileName, const FString& AssetPackageName);
};
