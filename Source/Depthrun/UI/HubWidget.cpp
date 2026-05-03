// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "HubWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Items/RunItemCollection.h"
#include "Data/DepthrunSaveSubsystem.h"

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

// ─── 9G: Metaprogression ────────────────────────────────────────────────

static UDepthrunSaveSubsystem* GetSave(const UObject* Ctx)
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(Ctx))
		return GI->GetSubsystem<UDepthrunSaveSubsystem>();
	return nullptr;
}

int32 UHubWidget::GetTotalDiamonds() const
{
	if (UDepthrunSaveSubsystem* Save = GetSave(this))
		return Save->GetTotalDiamonds();
	return 0;
}

int32 UHubWidget::GetUpgradeLevel(EHubUpgrade Type) const
{
	if (UDepthrunSaveSubsystem* Save = GetSave(this))
		return Save->GetUpgradeLevel(Type);
	return 0;
}

int32 UHubWidget::GetUpgradeCost(EHubUpgrade Type) const
{
	if (UDepthrunSaveSubsystem* Save = GetSave(this))
		return Save->GetUpgradeCost(Type);
	return -1;
}

void UHubWidget::OnUpgradePressed(EHubUpgrade Type)
{
	if (UDepthrunSaveSubsystem* Save = GetSave(this))
	{
		if (Save->BuyUpgrade(Type))
		{
			RefreshUI();
		}
	}
}
