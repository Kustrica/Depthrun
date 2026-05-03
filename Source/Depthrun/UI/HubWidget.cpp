// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "HubWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Items/RunItemCollection.h"

void UHubWidget::RefreshUI()
{
	// TODO (Stage 9B): load save data, populate upgrade slots, item grid
	OnUIRefreshed();
}

void UHubWidget::SelectItem(int32 ItemIndex)
{
	if (!ItemCollection || !ItemCollection->Items.IsValidIndex(ItemIndex)) return;
	SelectedItemIndices.AddUnique(ItemIndex);
}

void UHubWidget::OnStartRunPressed()
{
	// TODO (Stage 9B): push SelectedItems into GameInstance/RunItemInventory, then open level
	UGameplayStatics::OpenLevel(this, GameplayLevelName);
}

void UHubWidget::OnBackToMenuPressed()
{
	UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}
