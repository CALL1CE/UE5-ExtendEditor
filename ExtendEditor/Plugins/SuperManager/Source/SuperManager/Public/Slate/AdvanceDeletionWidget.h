// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab :public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray< TSharedPtr<FAssetData> >,AssetsDataToStore)

	SLATE_ARGUMENT(FString, CurrentSelectedFolder)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
private:

	TArray< TSharedPtr<FAssetData> > StoredAssetsDataArray;

	TArray< TSharedPtr<FAssetData> > DisplayedAssetsData;

#pragma region ComboBoxForListingCondition

	TSharedRef<SComboBox<TSharedPtr<FString> > > ConstructComboBox();

	TArray<TSharedPtr<FString> > ComboBoxSourceItems;

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);

	TSharedPtr<STextBlock> ComboDisplayTextBlock;

	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& TextContent, ETextJustify::Type TextJustify);

#pragma endregion


#pragma region RowWidgetForAssetListView

	TArray< TSharedRef <SCheckBox> > CheckBoxsArray;

	TArray <TSharedPtr<FAssetData> > AssetsDataToDeleteArray;

	TSharedRef< SListView< TSharedPtr<FAssetData> > > ConstructAssetListView();

	TSharedPtr< SListView< TSharedPtr<FAssetData> > > ConstructedAssetListView;

	void RefreshAssetListView();

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);

	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);

#pragma endregion

#pragma region TabButtons

	TSharedRef<SButton> ConstructButtonForDeleteAll();

	TSharedRef<SButton> ConstructButtonForSelectAll();

	TSharedRef<SButton> ConstructButtonForDeselectAll();

	FReply OnDeleteAllButtonClicked();

	FReply OnSelectAllButtonClicked();

	FReply OnDeselectAllButtonClicked();

	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);

#pragma endregion

	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }

};
