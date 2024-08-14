// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
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

		//处理MaterialInstance的重命名问题
		if (Object->IsA<UMaterialInstanceConstant>())
		{
			OldName.RemoveFromEnd(TEXT("_inst"));
			OldName.RemoveFromStart(TEXT("M_"));
		}
		//if (OldName.EndsWith(TEXT("_inst")))
		//{
		//	OldName.RemoveFromEnd(TEXT("_inst"));
		//}
		//if (OldName.StartsWith(TEXT("M_")))
		//{
		//	OldName.RemoveFromStart(TEXT("M_"));
		//}

		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(Object, NewNameWithPrefix);
		++Counter;
	}
	if (Counter > 0)
	{
		ShowNotifyInfo(TEXT("Successfully renamed ") + FString::FromInt(Counter) + " assets");
	}

}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetData;

	for (const FAssetData& AssetData : SelectedAssetsData)
	{
		TArray<FString>AssetRefrencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData.ObjectPath.ToString());

		if (AssetRefrencers.Num() == 0)
		{
			//Print(AssetData.ObjectPath.ToString() + TEXT(" AssetRefrencers num: ") + FString::FromInt(AssetRefrencers.Num()), FColor::Red);
			UnusedAssetData.Add(AssetData);
		}
	}

	if (UnusedAssetData.Num() == 0)
	{
		ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found among selected assets"), false);
		return;
	}

	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetData);
	if (NumOfAssetsDeleted == 0) return;
	ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets")));

}
