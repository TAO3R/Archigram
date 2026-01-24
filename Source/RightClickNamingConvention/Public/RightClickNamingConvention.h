// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FRightClickNamingConventionModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;	// gets called when the module is loaded into memory
	virtual void ShutdownModule() override;	// gets called just before the module is unloaded from memory

private:
	void RegisterMenus();	// adding menu entry to the content browser right-click menu
	static void ExecuteAddPrefix(const struct FToolMenuContext& MenuContext);	// the parameter comes from the tool menu dependency specified in the .Build.cs file
};
