// Fill out your copyright notice in the Description page of Project Settings.


#include "Slate/AdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "Debugheader.h"
#include "SuperManager.h"

#define ListAll TEXT("List All Available Assets")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	StoredAssetsDataArray = InArgs._AssetsDataToStore;
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;
	CheckBoxsArray.Empty();
	AssetsDataToDeleteArray.Empty();
	
	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));

	ChildSlot
		[
			//Main vertival box
			SNew(SVerticalBox)

			//First vertical slot for title text
			+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Advance Deletion")))
					.Font(TitleTextFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FColor::White)
				]

			//Second slot for drop down to specify the listing condition and help text
			+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					//Combox Box Slot
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						ConstructComboBox()
					]

				]

			//Third slot for asset list
			+SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				[
					SNew(SScrollBox)
					+SScrollBox::Slot()
					[
						ConstructAssetListView()
					]
				]

			//fourth slot for buttons
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(10.f)
					.Padding(5.f)
					[
						ConstructButtonForDeleteAll()
					]
					+SHorizontalBox::Slot()
					.FillWidth(10.f)
					.Padding(5.f)
					[
						ConstructButtonForSelectAll()
					]
					+ SHorizontalBox::Slot()
					.FillWidth(10.f)
					.Padding(5.f)
					[
						ConstructButtonForDeselectAll()
					]

				]
		];

}

#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString> > > SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox < TSharedPtr <FString> > > ConstructedComboBox =
		SNew(SComboBox < TSharedPtr <FString> >)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SAdvanceDeletionTab::OnGenerateComboContent)
		.OnSelectionChanged(this, &SAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock).Text(FText::FromString(TEXT("List Assets Option")))
		];
	return ConstructedComboBox;
}


TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText = SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	DebugHeader::Print(*SelectedOption.Get(), FColor::Cyan);

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));
}

#pragma endregion

#pragma region RowWidgetForAssetListView



TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView< TSharedPtr<FAssetData> >)
		.ItemHeight(24.f)
		.ListItemsSource(&StoredAssetsDataArray)
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList);

	return ConstructedAssetListView.ToSharedRef();
}

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay->IsValid()) return SNew(STableRow< TSharedPtr < FAssetData > >, OwnerTable);
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClass.ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10;
	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15;

	TSharedRef<STableRow <TSharedPtr <FAssetData> > >ListViewRowWidget =
	SNew(STableRow< TSharedPtr < FAssetData > >, OwnerTable).Padding(5.f)
	[
		SNew(SHorizontalBox)
		//First slot for check box
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]

		//Second slot for displaying asset class name
		
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.FillWidth(.5f)
		[
			ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
		]
		//Third slot for displaying asset name
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
		]
		
		//Fourth slot for a button
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForRowWidget(AssetDataToDisplay)
		]
	];
	return ListViewRowWidget;
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	CheckBoxsArray.Add(ConstructedCheckBox);

	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked,AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		if (StoredAssetsDataArray.Contains(ClickedAssetData))
		{
			StoredAssetsDataArray.Remove(ClickedAssetData);
		}
	}

	return FReply::Handled();
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	CheckBoxsArray.Empty();

	AssetsDataToDeleteArray.Empty();

	if (ConstructedAssetListView->IsVolatile())
	{
		ConstructAssetListView()->RebuildList();
	}
}

#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForDeleteAll()
{
	TSharedRef<SButton>ConstructedButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	ConstructedButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return ConstructedButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForSelectAll()
{
	TSharedRef<SButton>ConstructedButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);

	ConstructedButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return ConstructedButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForDeselectAll()
{
	TSharedRef<SButton>ConstructedButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeselectAllButtonClicked);

	ConstructedButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));

	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{

	if (AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;

	for (const TSharedPtr<FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FSuperManagerModule& SuperManagerMoudule = 
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetsDelete = SuperManagerMoudule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);

	if (bAssetsDelete)
	{
		for (const TSharedPtr<FAssetData>& DeleteData : AssetsDataToDeleteArray)
		{
			if (StoredAssetsDataArray.Contains(DeleteData))
			{
				StoredAssetsDataArray.Remove(DeleteData);
			}
		}

		RefreshAssetListView();
	}

	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	
	if (CheckBoxsArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxsArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}


	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	if (CheckBoxsArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxsArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15;

	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextFont)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion