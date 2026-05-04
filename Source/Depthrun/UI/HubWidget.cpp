// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "HubWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Items/RunItemCollection.h"
#include "Data/DepthrunSaveSubsystem.h"
#include "UI/UISoundLibrary.h"
#include "Input/Events.h"

void UHubWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
}

FReply UHubWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnBackToMenuPressed();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

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

void UHubWidget::PlayHoverSound()
{
	if (UGameInstance* GI = GetGameInstance())
		if (UUISoundLibrary* SFX = GI->GetSubsystem<UUISoundLibrary>())
			SFX->PlayButtonHover();
}

void UHubWidget::PlayClickSound()
{
	if (UGameInstance* GI = GetGameInstance())
		if (UUISoundLibrary* SFX = GI->GetSubsystem<UUISoundLibrary>())
			SFX->PlayButtonClick();
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
