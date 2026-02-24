// Copyright Epic Games, Inc. All Rights Reserved.

#include "Archigram.h"
#include "ToolMenus.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"				// For TActorIterator
#include "Kismet/GameplayStatics.h"		// For GetAllActorsOfClass



#define LOCTEXT_NAMESPACE "FArchigramModule"

#pragma region Variables

// Style set name - used to reference our custom icons
const FName ArchigramStyleSetName = TEXT("ArchigramStyle");

// Path to the BP_PCG Blueprint actor (adjust if your path is different)
const TCHAR* FArchigramModule::PCGActorBlueprintPath = TEXT("/Archigram/Blueprints/BP_PCG.BP_PCG_C");

// Folder name in World Outliner for Archigram actors
const FName ArchigramOutlinerFolderName = FName(TEXT("Archigram"));

// Static member to track the spawned PCG actor
// TWeakObjectPtr automatically becomes invalid when the actor is deleted/garbage collected
TWeakObjectPtr<AActor> FArchigramModule::SpawnedPCGActor = nullptr;

#pragma endregion


#pragma region Functions

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

	// Bind to map opened event to handle level changes
	FEditorDelegates::OnMapOpened.AddRaw(this, &FArchigramModule::OnMapOpened);
}

void FArchigramModule::ShutdownModule()
{
	// Unbind from map opened event
	FEditorDelegates::OnMapOpened.RemoveAll(this);

	// Clean up menu registrations
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
	}

	// Unregister custom style
	UnregisterStyleSet();

	// Clear the PCG actor reference
	ClearSpawnedPCGActorReference();
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
			ResourcesDir / TEXT("Derek_2.png"),  // Use your icon file
			IconSize
		)
	);

	// Also register a small version for menus
	StyleSet->Set(
		"Archigram.ToolbarIcon.Small",
		new FSlateImageBrush(
			ResourcesDir / TEXT("Derek_2.png"),  // Same file, different size
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
			LOCTEXT("ArchigramToolbarButtonTooltip", "Execute Archigram action"),	// Tooltip
			FSlateIcon(ArchigramStyleSetName, "Archigram.ToolbarIcon", "Archigram.ToolbarIcon.Small")	// Icon from our style set
		)
	);
}

void FArchigramModule::ExecuteToolbarAction()
{
	UE_LOG(LogTemp, Warning, TEXT("***********************************************"));
	UE_LOG(LogTemp, Warning, TEXT("*  Archigram Toolbar Button Clicked!          *"));
	UE_LOG(LogTemp, Warning, TEXT("***********************************************"));

	// Check if a PCG actor already exists
	if (HasSpawnedPCGActor())
	{
		AActor* ExistingActor = GetSpawnedPCGActor();
		UE_LOG(LogTemp, Warning, TEXT("Archigram: PCG Actor already exists in level: %s"), 
			ExistingActor ? *ExistingActor->GetName() : TEXT("Unknown"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
				TEXT("Archigram: PCG Actor already exists! Delete it first to spawn a new one."));
		}

		// Optionally select the existing actor
		if (GEditor && ExistingActor)
		{
			GEditor->SelectNone(false, true);
			GEditor->SelectActor(ExistingActor, true, true);
		}
		return;
	}

	// Spawn the PCG actor at origin
	UE_LOG(LogTemp, Warning, TEXT("*  Spawning BP_PCG Actor at origin...         *"));
	AActor* NewActor = SpawnPCGActor(FVector::ZeroVector);

	// Display result on screen
	if (GEngine)
	{
		if (NewActor)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
				FString::Printf(TEXT("Archigram: Spawned %s at origin"), *NewActor->GetName()));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
				TEXT("Archigram: Failed to spawn PCG Actor"));
		}
	}
}

AActor* FArchigramModule::SpawnPCGActor(FVector Location)
{
	// Check if actor already exists
	if (HasSpawnedPCGActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Archigram: PCG Actor already exists, not spawning another"));
		return GetSpawnedPCGActor();
	}

	// Get the editor world
	UWorld* World = nullptr;
	
	if (GEditor)
	{
		World = GEditor->GetEditorWorldContext().World();
	}

	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Archigram: Cannot spawn actor - no valid world found"));
		return nullptr;
	}

	// Load the Blueprint class
	UClass* PCGActorClass = LoadClass<AActor>(nullptr, PCGActorBlueprintPath);
	
	if (!PCGActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Archigram: Failed to load Blueprint class at path: %s"), PCGActorBlueprintPath);
		return nullptr;
	}

	// Set up spawn parameters
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn the actor
	AActor* NewActor = World->SpawnActor<AActor>(PCGActorClass, Location, FRotator::ZeroRotator, SpawnParams);

	if (NewActor)
	{
		// Store the weak reference to track this actor
		SpawnedPCGActor = NewActor;

		// Place the actor in the "Archigram" folder in World Outliner
		NewActor->SetFolderPath(ArchigramOutlinerFolderName);

		if (UPCGComponent* PCGComp = NewActor->FindComponentByClass<UPCGComponent>())
		{
			PCGComp->Generate();
			UE_LOG(LogTemp, Log, TEXT("Archigram: Triggered PCG generation for %s"), *NewActor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Archigram: No PCG component found on %s"), *NewActor->GetName());
		}

		UE_LOG(LogTemp, Log, TEXT("Archigram: Successfully spawned %s at location (%f, %f, %f) in folder '%s'"), 
			*NewActor->GetName(), Location.X, Location.Y, Location.Z, *ArchigramOutlinerFolderName.ToString());
		
		// Select the newly spawned actor in the editor
		if (GEditor)
		{
			GEditor->SelectNone(false, true);
			GEditor->SelectActor(NewActor, true, true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Archigram: SpawnActor returned nullptr"));
	}

	return NewActor;
}	// end of SpawnPCGActor

AActor* FArchigramModule::GetSpawnedPCGActor()
{
	// TWeakObjectPtr::Get() returns nullptr if the object has been destroyed
	return SpawnedPCGActor.Get();
}

bool FArchigramModule::HasSpawnedPCGActor()
{
	// TWeakObjectPtr::IsValid() returns false if:
	// - The pointer was never set (nullptr)
	// - The referenced object has been destroyed/garbage collected
	return SpawnedPCGActor.IsValid();
}

void FArchigramModule::ClearSpawnedPCGActorReference()
{
	SpawnedPCGActor.Reset();
	UE_LOG(LogTemp, Log, TEXT("Archigram: Cleared PCG Actor reference"));
}

void FArchigramModule::OnMapOpened(const FString& Filename, bool bAsTemplate)
{
	UE_LOG(LogTemp, Log, TEXT("Archigram: Map opened - %s"), *Filename);

	// Clear the current reference (it points to an actor in the old level)
	ClearSpawnedPCGActorReference();

	// Search for existing PCG actor in the newly opened level
	AActor* ExistingActor = FindExistingPCGActorInLevel();

	if (ExistingActor)
	{
		SpawnedPCGActor = ExistingActor;
		
		// Ensure the actor is in the Archigram folder
		if (ExistingActor->GetFolderPath() != ArchigramOutlinerFolderName)
		{
			ExistingActor->SetFolderPath(ArchigramOutlinerFolderName);
			UE_LOG(LogTemp, Log, TEXT("Archigram: Moved existing PCG Actor to 'Archigram' folder"));
		}

		UE_LOG(LogTemp, Log, TEXT("Archigram: Found existing PCG Actor in level: %s"), *ExistingActor->GetName());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
				FString::Printf(TEXT("Archigram: Found existing PCG Actor: %s"), *ExistingActor->GetName()));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Archigram: No existing PCG Actor found in level"));
	}
}

AActor* FArchigramModule::FindExistingPCGActorInLevel()
{
	// Get the editor world
	UWorld* World = nullptr;

	if (GEditor)
	{
		World = GEditor->GetEditorWorldContext().World();
	}

	if (!World)
	{
		return nullptr;
	}

	// Load the Blueprint class to check against
	UClass* PCGActorClass = LoadClass<AActor>(nullptr, PCGActorBlueprintPath);

	if (!PCGActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Archigram: Could not load PCG Actor class for search"));
		return nullptr;
	}

	// Search for actors of this class in the world
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->GetClass() == PCGActorClass)
		{
			// Found a matching actor
			return Actor;
		}
	}

	// Alternative: Use GetAllActorsOfClass (slightly less efficient but clearer)
	// TArray<AActor*> FoundActors;
	// UGameplayStatics::GetAllActorsOfClass(World, PCGActorClass, FoundActors);
	// if (FoundActors.Num() > 0)
	// {
	//     return FoundActors[0];
	// }

	return nullptr;
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

#pragma endregion



#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FArchigramModule, Archigram)
