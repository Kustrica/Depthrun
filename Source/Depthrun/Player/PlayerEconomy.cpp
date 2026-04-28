// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Player/PlayerEconomy.h"
#include "Core/DepthrunLogChannels.h"
#include "Data/DepthrunSaveSubsystem.h"
#include "DepthrunCharacter.h"

UPlayerEconomy::UPlayerEconomy()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerEconomy::AddDiamonds(int32 Amount)
{
	if (Amount <= 0) { return; }

	int32 OldValue = RunDiamonds;
	RunDiamonds += Amount;

	UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] Diamonds: %d → %d (+%d)"),
		OldValue, RunDiamonds, Amount);

	OnDiamondsChanged.Broadcast(OldValue, RunDiamonds);
}

void UPlayerEconomy::AddPotions(int32 Amount)
{
	if (Amount <= 0) { return; }

	int32 OldValue = HealthPotions;
	HealthPotions += Amount;

	UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] Potions: %d → %d (+%d)"),
		OldValue, HealthPotions, Amount);

	OnPotionsChanged.Broadcast(OldValue, HealthPotions);
}

bool UPlayerEconomy::UsePotion()
{
	if (HealthPotions <= 0)
	{
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] UsePotion failed: no potions"));
		return false;
	}

	if (IsPotionOnCooldown())
	{
		UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] UsePotion failed: cooldown active"));
		return false;
	}

	// Apply healing
	ADepthrunCharacter* Player = Cast<ADepthrunCharacter>(GetOwner());
	if (!Player)
	{
		UE_LOG(LogDepthrunEconomy, Warning, TEXT("[Economy] UsePotion: owner is not ADepthrunCharacter"));
		return false;
	}

	float HPBefore = Player->CurrentHP;
	Player->Heal(PotionHealAmount);
	float HPAfter = Player->CurrentHP;

	// Consume potion
	int32 OldPotions = HealthPotions;
	HealthPotions--;
	LastPotionUseTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] UsePotion: HP %.0f → %.0f (Potions: %d → %d)"),
		HPBefore, HPAfter, OldPotions, HealthPotions);

	OnPotionsChanged.Broadcast(OldPotions, HealthPotions);
	return true;
}

void UPlayerEconomy::OnPlayerDeath()
{
	int32 ConvertedDiamonds = FMath::FloorToInt(RunDiamonds * 0.5f);

	UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] Player death: RunDiamonds=%d, converting 50%%=%d to profile"),
		RunDiamonds, ConvertedDiamonds);

	// Save to profile via SaveSubsystem
	if (UDepthrunSaveSubsystem* Save = GetSaveSubsystem())
	{
		Save->AddDiamondsToProfile(ConvertedDiamonds);
	}

	ClearRunEconomy();
}

void UPlayerEconomy::OnRunExitToHub()
{
	UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] Run exit to Hub: converting %d diamonds (100%%) to profile"),
		RunDiamonds);

	// Save to profile via SaveSubsystem
	if (UDepthrunSaveSubsystem* Save = GetSaveSubsystem())
	{
		Save->AddDiamondsToProfile(RunDiamonds);
	}

	ClearRunEconomy();
}

UDepthrunSaveSubsystem* UPlayerEconomy::GetSaveSubsystem() const
{
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UDepthrunSaveSubsystem>();
		}
	}
	return nullptr;
}

void UPlayerEconomy::ClearRunEconomy()
{
	int32 OldDiamonds = RunDiamonds;
	int32 OldPotions = HealthPotions;

	RunDiamonds = 0;
	HealthPotions = 0;
	LastPotionUseTime = -999.f;

	UE_LOG(LogDepthrunEconomy, Log, TEXT("[Economy] ClearRunEconomy: Diamonds %d→0, Potions %d→0"),
		OldDiamonds, OldPotions);

	OnDiamondsChanged.Broadcast(OldDiamonds, 0);
	OnPotionsChanged.Broadcast(OldPotions, 0);
}

bool UPlayerEconomy::IsPotionOnCooldown() const
{
	if (!GetWorld()) { return false; }

	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - LastPotionUseTime) < PotionCooldown;
}
