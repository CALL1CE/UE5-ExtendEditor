// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a VALID number!"));
		//Print(TEXT("Please enter a VALID number!"), FColor::Red);
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& AssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = AssetData.ObjectPath.ToString();
			const FString NewDuplicatedAssetName = AssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(AssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"));
		//Print(TEXT("Successfully duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);
	}
}

void UQuickAssetAction::AddPrefix()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* Object : SelectedObjects)
	{
		if(!Object) continue;

		FString* PrefixFound = PrefixMap.Find(Object->GetClass());

		//如果返回null或者返回的字符串是空的
		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			Print(TEXT("Failed to find prefix for class ") + Object->GetClass()->GetName(), FColor::Red);
			continue;
		}

		FString OldName = Object->GetName();

		if (OldName.StartsWith(*PrefixFound))
		{
			Print(TEXT(" already has prefix added"), FColor::Red);
			continue;
		}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(Object, NewNameWithPrefix);
		++Counter;
	}
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(Counter) + " assets");
	}

}
