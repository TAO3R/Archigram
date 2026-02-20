// Copyright Epic Games, Inc. All Rights Reserved.

#include "Archigram.h"
#include "ToolMenus.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FArchigramModule"

// Style set name - used to reference our custom icons
const FName ArchigramStyleSetName = TEXT("ArchigramStyle");

void FArchigramModule::StartupModule()
{
	// Register custom style (icons) first
	RegisterStyleSet();

	// Register menus and toolbar when the ToolMenus system is ready
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FArchigramModule::RegisterMenuBarMenus)
		);
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FArchigramModule::RegisterToolbarButton)
		);
	}
}

void FArchigramModule::ShutdownModule()
{
	// Clean up menu registrations
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

	// Unregister custom style
	UnregisterStyleSet();
}

void FArchigramModule::RegisterStyleSet()
{
	// Create a new style set
	StyleSet = MakeShareable(new FSlateStyleSet(ArchigramStyleSetName));

	// Get the plugin's Resources folder path
	FString PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("Archigram"))->GetBaseDir();
	FString ResourcesDir = PluginBaseDir / TEXT("Resources");

	// Set the content root for finding images
	StyleSet->SetContentRoot(ResourcesDir);

	// Register toolbar icon (40x40 for toolbar, but it will scale)
	// You can add a dedicated toolbar icon like "ToolbarIcon40.png" or use Icon128.png
	const FVector2D IconSize(40.0f, 40.0f);
	const FVector2D SmallIconSize(20.0f, 20.0f);

	StyleSet->Set(
		"Archigram.ToolbarIcon",
		new FSlateImageBrush(
			ResourcesDir / TEXT("Derek.png"),  // Use your icon file
			IconSize
		)
	);

	// Also register a small version for menus
	StyleSet->Set(
		"Archigram.ToolbarIcon.Small",
		new FSlateImageBrush(
			ResourcesDir / TEXT("Derek.png"),  // Same file, different size
			SmallIconSize
		)
	);

	// Register the style set with Slate
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet);
}

void FArchigramModule::UnregisterStyleSet()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet);
		StyleSet.Reset();
	}
}

void FArchigramModule::RegisterMenuBarMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	// All menu entries created from now on belong to this module
	FToolMenuOwnerScoped OwnerScoped(this);

	// ========== Option 1: Add to existing menu (e.g., Tools menu) ==========
	// UToolMenu* ToolsMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.Tools");
	
	// ========== Option 2: Create a new top-level menu (Archigram menu) ==========
	UToolMenu* MainMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu");
	if (!MainMenu)
	{
		return;
	}

	// Add new "Archigram" menu to the main menu bar
	FToolMenuSection& Section = MainMenu->AddSection("ArchigramMenuBarSection", FText::FromString(TEXT("Archigram")));
	
	// Create a submenu
	Section.AddSubMenu(
		"ArchigramMenuBarSubMenu",													// Internal name
		LOCTEXT("ArchigramMenuBarSubMenu", "Archigram"),							// Display name on the menu bar
		LOCTEXT("ArchigramMenuBarSubMenuTooltip", "Tools for Archigram plugin"),	// Tooltip of the submenu
		FNewToolMenuChoice()														// Function that creates the submenu content
	);

	// extend the submenu just created
	UToolMenu* ArchigramSubMenu = ToolMenus->ExtendMenu("LevelEditor.MainMenu.ArchigramMenuBarSubMenu");
	if (!ArchigramSubMenu)
	{
		return;
	}

	// Add section to the submenu
	FToolMenuSection& ArchigramSection = ArchigramSubMenu->AddSection(
		"ArchigramTools",
		LOCTEXT("ArchigramToolsSection", "Archigram Tools")	// Display name of the section
	);

	// Add "Pipeline Test" menu entry
	ArchigramSection.AddMenuEntry(
		"PipelineTestLog",																		// Internal name
		LOCTEXT("PipelineTestLog", "Pipeline Test Log"),										// Display name of an entry on the menu
		LOCTEXT("PipelineTestLogTooltip", "Outputs a test message to the log"),					// Tooltip of the entry
		FSlateIcon(),																			// Icon (empty for now)
		FUIAction(FExecuteAction::CreateStatic(&FArchigramModule::ExecutePipelineTestLog))		// Function that the entry executes
	);

}	// end of registerMenuBarMenus

void FArchigramModule::RegisterToolbarButton()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	// All toolbar entries created from now on belong to this module
	FToolMenuOwnerScoped OwnerScoped(this);

	// Extend the Level Editor Toolbar
	UToolMenu* ToolBar = ToolMenus->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	if (!ToolBar)
	{
		return;
	}

	// Add a section for our button
	FToolMenuSection& Section = ToolBar->FindOrAddSection("Archigram");

	// Create the toolbar button with our custom icon
	FToolMenuEntry& Entry = Section.AddEntry(
		FToolMenuEntry::InitToolBarButton(
			"ArchigramToolbarButton",											// Internal name
			FUIAction(FExecuteAction::CreateStatic(&FArchigramModule::ExecuteToolbarAction)),
			LOCTEXT("ArchigramToolbarButton", "Archigram"),						// Label
			LOCTEXT("ArchigramToolbarButtonTooltip", "Execute Archigram action and output to log"),	// Tooltip
			FSlateIcon(ArchigramStyleSetName, "Archigram.ToolbarIcon", "Archigram.ToolbarIcon.Small")	// Icon from our style set
		)
	);
}

void FArchigramModule::ExecuteToolbarAction()
{
	// Output to the Output Log
	UE_LOG(LogTemp, Warning, TEXT("***********************************************"));
	UE_LOG(LogTemp, Warning, TEXT("*  Archigram Toolbar Button Clicked!          *"));
	UE_LOG(LogTemp, Warning, TEXT("*  This is the toolbar action output.         *"));
	UE_LOG(LogTemp, Warning, TEXT("***********************************************"));

	// Display on screen
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Archigram Toolbar Button Clicked!"));
	}

	// TODO: add the HDA actor to the level's origin (or a user specified position)
	
}

void FArchigramModule::SetHDAMeshCollisionTypeToDefault(const FName& PackageName, EPackageFlags PackageFlags, const FString& PackageFileName, const FString& AssetPackageName)
{
	
}

void FArchigramModule::ExecutePipelineTestLog()
{
	// Output to the Output Log
	UE_LOG(LogTemp, Warning, TEXT("==========================================="));
	UE_LOG(LogTemp, Warning, TEXT("Archigram Pipeline Test Log Executed!"));
	UE_LOG(LogTemp, Warning, TEXT("This confirms the menu system is working."));
	UE_LOG(LogTemp, Warning, TEXT("==========================================="));
	
	// Also display on screen if in editor (optional)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Archigram: Pipeline Test Log Executed!"));
	}

}	// end of excutePipelineTestLog

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FArchigramModule, Archigram)
