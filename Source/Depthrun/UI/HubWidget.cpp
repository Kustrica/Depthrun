// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "HubWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Items/RunItemConfig.h"

void UHubWidget::RefreshUI()
{
	// TODO (Stage 9B): load save data, populate upgrade slots, item grid
	OnUIRefreshed();
}

void UHubWidget::SelectItem(URunItemConfig* Config)
{
	if (!Config) return;
	SelectedItems.AddUnique(Config);
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
