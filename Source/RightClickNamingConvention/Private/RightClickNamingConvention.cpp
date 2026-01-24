// Copyright Epic Games, Inc. All Rights Reserved.

#include "RightClickNamingConvention.h"
#include "ToolMenu.h"
#include "ContentBrowserMenuContexts.h"
#include "AssetToolsModule.h"	// provides asset data
#include "Engine/Blueprint.h"
#include "Materials/Material.h"
#include "Engine/Texture.h"

#define LOCTEXT_NAMESPACE "FRightClickNamingConventionModule"

void FRightClickNamingConventionModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	if (UToolMenus::IsToolMenuUIEnabled())
	{
		// don't try to register menus before the system is ready
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRightClickNamingConventionModule::RegisterMenus)
		);
	}
}

void FRightClickNamingConventionModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenus::UnRegisterStartupCallback(this);	// removing a registered startup callback
		UToolMenus::UnregisterOwner(this);				// removes all menu items that were associated with a particular owner
	}
}

void FRightClickNamingConventionModule::RegisterMenus()
{
	UToolMenus* ToolMenus = UToolMenus::Get();	// store a pointer to the UToolMenus instance
	if (!ToolMenus)
	{
		return;
	}

	// whatever menu entries created from now on belongs to this module
	// when module is unloaded, it's going to automatically get rid of these entries
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = ToolMenus->ExtendMenu("ContentBrowser.AssetContextMenu");
	if (!Menu)
	{
		return;
	}

	FToolMenuSection& Section = Menu->AddSection(
		"RigthClickNamingConventionSection",
		FText::FromString(TEXT("Naming"))	// section name
	);

	FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
		"RigthClickNamingConventionSection",
		FText::FromString(TEXT("Apply Naming Convention")),	// entry name
		FText::FromString(TEXT("Adds BP_, M_, MI_, and T_ naming convention to selected assets")),	// entry mouse-hover info
		FSlateIcon(),
		FToolMenuExecuteAction::CreateStatic(
			&FRightClickNamingConventionModule::ExecuteAddPrefix)	// actual function being executed
	);

	Section.AddEntry(Entry);	// Actually add the new entry

}	// end of RegisterMenus

void FRightClickNamingConventionModule::ExecuteAddPrefix(const FToolMenuContext& MenuContext)
{
	// read the context that gets selected in the content browser
	const UContentBrowserAssetContextMenuContext* AssetContext = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
	if (!AssetContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("RightClickNamingConvention: No context found for the asset."));
		return;
	}

	const TArray<FAssetData>& SelectedAsset = AssetContext->SelectedAssets;
	if (SelectedAsset.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RightClickNamingConvention: No asset has been selected."));
		return;
	}

	TArray<FAssetRenameData> AssetsToRename;
	AssetsToRename.Reserve(SelectedAsset.Num());

	for (const FAssetData& AssetData : SelectedAsset)
	{	// Traversing through the selected asset array and binding to the AssetData variable
		UObject* Asset = AssetData.GetAsset();

		if (!Asset)
		{
			continue;
		}

		FString CorrectConvention;

		if (Asset->IsA<UBlueprint>())
		{
			CorrectConvention = TEXT("BP_");
		}
		else if (Asset->IsA<UMaterial>())
		{
			CorrectConvention = TEXT("M_");
		}
		else if (Asset->IsA<UMaterialInstance>())
		{
			CorrectConvention = TEXT("MI_");
		}
		else if (Asset->IsA<UTexture>())
		{
			CorrectConvention = TEXT("T_");
		}
		else
		{
			continue;
		}

		const FString OriginalName = Asset->GetName();

		if (OriginalName.StartsWith(CorrectConvention))
		{
			continue;
		}
		
		const FString NewName = CorrectConvention + OriginalName;

		FAssetRenameData RenameData(Asset, AssetData.PackagePath.ToString(), NewName);

		AssetsToRename.Add(MoveTemp(RenameData));
	}

	if (AssetsToRename.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RightClickNamingConvention: No Asset need to be renamed."));
		return;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	AssetToolsModule.Get().RenameAssets(AssetsToRename);

}	// end of ExecuteAddPrefix


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRightClickNamingConventionModule, RightClickNamingConvention)