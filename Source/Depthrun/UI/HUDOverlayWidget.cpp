// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "UI/HUDOverlayWidget.h"
#include "Player/DepthrunCharacter.h"
#include "Player/PlayerEconomy.h"
#include "Kismet/GameplayStatics.h"

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
	OnHPUpdated(Pct, Current, Max);
}

void UHUDOverlayWidget::SetDiamonds(int32 Amount)
{
	OnDiamondsUpdated(Amount);
}

void UHUDOverlayWidget::SetPotions(int32 Amount)
{
	OnPotionsUpdated(Amount);
}

void UHUDOverlayWidget::SetRoomInfo(int32 Current, int32 Total)
{
	OnRoomInfoUpdated(Current, Total);
}

void UHUDOverlayWidget::SetActiveWeaponSlot(int32 SlotIndex)
{
	OnWeaponSlotUpdated(SlotIndex);
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
