// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "DebugHeader.h"
#include "ObjectTools.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Slate/AdvanceDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}



#pragma region  ContentBrowserMenuExtention

void FSuperManagerModule::InitCBMenuExtention()
{
	auto& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	auto& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	//FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	//CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	//ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);
	
	//添加委托
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::
		CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));

}

//定义插入菜单的位置
TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	//see hook : Editor preferences->display ui extension points
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"), // Extention hook, position to insert
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(), //custom hotkeys
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)); //second bind
		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

//define the details for the custom menu entry
void FSuperManagerModule::AddCBMenuEntry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")), // Title
		FText::FromString(TEXT("Safty delete all unused assets under folder")), //Tooltip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnDeleteUnusedAssetButtonClicked) //the actual function
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")), // Title
		FText::FromString(TEXT("Safty delete all Empty Folders")), //Tooltip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked) //the actual function
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance Deletion")), // Title
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")), //Tooltip
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked) //the actual function
	);

}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("you can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets found under selected folder"),false);
		return;
	}

	EAppReturnType::Type ConfirmResult =
	DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT(" A total of ") + FString::FromInt(AssetsPathNames.Num())
	+ TEXT(" assets need to be checked.\nWould you like to proceed"),false);

	if (ConfirmResult == EAppReturnType::No) return;

	// 删除前先处理重定向器
	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder. may crash.
		if (AssetPathName.Contains(TEXT("Developers")) || 
			AssetPathName.Contains(TEXT("Collections")) || 
			AssetPathName.Contains(TEXT("__ExternalActors__")) || 
			AssetPathName.Contains(TEXT("__ExternalObjects__"))
			)
		{
			continue;
		}

		if(!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"),false);
	}

	ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel, TEXT("Would you like to delete empty folders now?"), false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	OnDeleteEmptyFoldersButtonClicked();
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;
	FString EmptyFolderPathNames;
	TArray<FString> EmptyFolderPathArray;

	for (const FString& FolderPath: FolderPathsArray)
	{
		//Don't touch root folder. may crash.
		if (FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__"))
			)
		{
			continue;
		}

		if(!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathNames.Append(FolderPath);
			EmptyFolderPathNames.Append("\n");

			EmptyFolderPathArray.Add(FolderPath);
		}
	}

	if (EmptyFolderPathArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		TEXT("Empty folders found in:\n") + EmptyFolderPathNames + TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath : EmptyFolderPathArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath) ? ++Counter : DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted ") + FString::FromInt(Counter) + (Counter==1?TEXT(" folder"): TEXT(" folders")));
	}
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const auto& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#pragma endregion


#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab)).SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return 
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab).AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	for (const FString& PathNAme : AssetsPathNames)
	{
		//Don't touch root folder. may crash.
		if (PathNAme.Contains(TEXT("Developers")) ||
			PathNAme.Contains(TEXT("Collections")) ||
			PathNAme.Contains(TEXT("__ExternalActors__")) ||
			PathNAme.Contains(TEXT("__ExternalObjects__"))
			)
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(PathNAme)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(PathNAme);
		AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
	}
	return AvailableAssetsData;
}

#pragma endregion

#pragma region ProcessDataForAdvanceDeletionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}

	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}

	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter, TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetDataToFilter)
	{
		TArray<FString> AssetReferences = UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());

		if (AssetReferences.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}

}
#pragma endregion

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)