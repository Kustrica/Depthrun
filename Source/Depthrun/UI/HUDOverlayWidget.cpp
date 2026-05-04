// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/HUDOverlayWidget.h"
#include "Player/DepthrunCharacter.h"
#include "Player/PlayerEconomy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHUDOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
	{
		CachedPlayer = Player;
		BindToPlayerDelegates(Player);
		RefreshFromPlayer();
	}
}

void UHUDOverlayWidget::NativeDestruct()
{
	UnbindFromPlayerDelegates();
	Super::NativeDestruct();
}

void UHUDOverlayWidget::BindToPlayerDelegates(ADepthrunCharacter* Player)
{
	if (!Player || !Player->PlayerEconomy) return;

	Player->PlayerEconomy->OnDiamondsChanged.AddDynamic(
		this, &UHUDOverlayWidget::SetDiamondsFromDelegate);
	Player->PlayerEconomy->OnPotionsChanged.AddDynamic(
		this, &UHUDOverlayWidget::SetPotionsFromDelegate);
}

void UHUDOverlayWidget::UnbindFromPlayerDelegates()
{
	if (CachedPlayer && CachedPlayer->PlayerEconomy)
	{
		CachedPlayer->PlayerEconomy->OnDiamondsChanged.RemoveDynamic(
			this, &UHUDOverlayWidget::SetDiamondsFromDelegate);
		CachedPlayer->PlayerEconomy->OnPotionsChanged.RemoveDynamic(
			this, &UHUDOverlayWidget::SetPotionsFromDelegate);
	}
}

void UHUDOverlayWidget::RefreshFromPlayer()
{
	if (!CachedPlayer) return;

	SetHP(CachedPlayer->CurrentHP, CachedPlayer->MaxHP);

	if (CachedPlayer->PlayerEconomy)
	{
		SetDiamonds(CachedPlayer->PlayerEconomy->RunDiamonds);
		SetPotions(CachedPlayer->PlayerEconomy->HealthPotions);
	}
}

void UHUDOverlayWidget::SetHP(float Current, float Max)
{
	const float Pct = (Max > 0.f) ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f;
	if (HPBar)  HPBar->SetPercent(Pct);
	if (HPText) HPText->SetText(FText::FromString(
		FString::Printf(TEXT("%.0f / %.0f"), Current, Max)));
	OnHPUpdated(Pct, Current, Max);
}

void UHUDOverlayWidget::SetDiamonds(int32 Amount)
{
	if (DiamondText) DiamondText->SetText(FText::AsNumber(Amount));
	OnDiamondsUpdated(Amount);
}

void UHUDOverlayWidget::SetPotions(int32 Amount)
{
	if (PotionText) PotionText->SetText(
		FText::FromString(FString::Printf(TEXT("x%d"), Amount)));
	OnPotionsUpdated(Amount);
}

void UHUDOverlayWidget::SetRoomInfo(int32 Current, int32 Total)
{
	if (RoomText) RoomText->SetText(FText::FromString(
		FString::Printf(TEXT("Room %d / %d"), Current, Total)));
	OnRoomInfoUpdated(Current, Total);
}

void UHUDOverlayWidget::SetActiveWeaponSlot(int32 SlotIndex)
{
	UpdateWeaponSlotVisuals(SlotIndex);
	OnWeaponSlotUpdated(SlotIndex);
}

void UHUDOverlayWidget::UpdateWeaponSlotVisuals(int32 ActiveSlot,
	float ActiveAlpha, float InactiveAlpha)
{
	const FLinearColor ActiveColor(1.f, 1.f, 1.f, 1.f);
	const FLinearColor InactiveColor(0.3f, 0.3f, 0.3f, 1.f);

	const bool bSlot0Active = (ActiveSlot == 0);

	if (BorderSlot1) BorderSlot1->SetContentColorAndOpacity(
		bSlot0Active ? ActiveColor : InactiveColor);
	if (BorderSlot2) BorderSlot2->SetContentColorAndOpacity(
		bSlot0Active ? InactiveColor : ActiveColor);

	if (IconSlot1) IconSlot1->SetRenderOpacity(bSlot0Active ? ActiveAlpha : InactiveAlpha);
	if (IconSlot2) IconSlot2->SetRenderOpacity(bSlot0Active ? InactiveAlpha : ActiveAlpha);
}

// ─── Private delegate receivers ─────────────────────────────────────────────

void UHUDOverlayWidget::SetDiamondsFromDelegate(int32 /*Old*/, int32 New)
{
	SetDiamonds(New);
}

void UHUDOverlayWidget::SetPotionsFromDelegate(int32 /*Old*/, int32 New)
{
	SetPotions(New);
}
