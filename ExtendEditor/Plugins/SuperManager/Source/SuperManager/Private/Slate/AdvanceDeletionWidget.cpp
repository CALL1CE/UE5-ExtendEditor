// Fill out your copyright notice in the Description page of Project Settings.


#include "Slate/AdvanceDeletionWidget.h"
#include "SlateBasics.h"

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	AssetsDataStoredArray = InArgs._AssetsDataArray;
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

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
				]

			//Third slot for asset list
			+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SScrollBox)
				]

			//fourth slot for buttons
			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
				]
		];

}
