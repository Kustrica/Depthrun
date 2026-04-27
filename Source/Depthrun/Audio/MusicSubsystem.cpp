// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Audio/MusicSubsystem.h"
#include "Audio/MusicTypes.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "DepthrunLogChannels.h"

void UMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Subsystem initialized"));

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UMusicSubsystem::OnTick));
}

void UMusicSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	if (IsValid(ActiveComponent))  { ActiveComponent->Stop(); }
	if (IsValid(FadingComponent))  { FadingComponent->Stop(); }

	Super::Deinitialize();
}

// ─── Public API ───────────────────────────────────────────────────────────────

void UMusicSubsystem::PlayMusic(EMusicTrack Track, float FadeIn, float FadeOut)
{
	if (Track == CurrentTrack && IsValid(ActiveComponent) && ActiveComponent->IsPlaying())
	{
		return;
	}

	USoundBase* NewSound = GetSoundForTrack(Track);
	if (!IsValid(NewSound))
	{
		UE_LOG(LogDepthrunMusic, Warning,
			TEXT("[Music] PlayMusic: no sound asset assigned for track %d. Assign tracks in BP_GameInstance defaults."),
			(int32)Track);
		return;
	}

	UE_LOG(LogDepthrunMusic, Log,
		TEXT("[Music] Track change: %d → %d (FadeIn=%.1fs FadeOut=%.1fs)"),
		(int32)CurrentTrack, (int32)Track, FadeIn, FadeOut);

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) { return; }

	// Stop any previous fade
	if (bIsCrossfading && IsValid(FadingComponent))
	{
		FadingComponent->Stop();
		FadingComponent = nullptr;
	}

	// Move active → fading
	if (IsValid(ActiveComponent) && ActiveComponent->IsPlaying())
	{
		FadingComponent = ActiveComponent;
		ActiveComponent = nullptr;
	}

	// Spawn new audio component
	ActiveComponent = UGameplayStatics::SpawnSoundAtLocation(
		World, NewSound, FVector::ZeroVector);

	if (!IsValid(ActiveComponent))
	{
		UE_LOG(LogDepthrunMusic, Error, TEXT("[Music] Failed to spawn AudioComponent for track %d"), (int32)Track);
		return;
	}

	ActiveComponent->bAutoDestroy = false;
	if (FadeIn > 0.f)
	{
		ActiveComponent->SetVolumeMultiplier(0.f);
	}
	ActiveComponent->Play();

	CurrentTrack    = Track;
	FadeInTime      = FMath::Max(FadeIn, 0.f);
	FadeOutTime     = FMath::Max(FadeOut, 0.f);
	FadeProgress    = 0.f;
	bIsCrossfading  = true;
}

void UMusicSubsystem::StopMusic(float FadeOut)
{
	if (IsValid(ActiveComponent))
	{
		if (FadeOut > 0.f)
		{
			ActiveComponent->FadeOut(FadeOut, 0.f);
		}
		else
		{
			ActiveComponent->Stop();
		}
		ActiveComponent = nullptr;
	}
	CurrentTrack = EMusicTrack::None;
	bIsCrossfading = false;
}

// ─── Private ──────────────────────────────────────────────────────────────────

USoundBase* UMusicSubsystem::GetSoundForTrack(EMusicTrack Track) const
{
	switch (Track)
	{
		case EMusicTrack::Hub:     return HubMusic;
		case EMusicTrack::Explore: return ExploreMusic;
		case EMusicTrack::Combat:  return CombatMusic;
		default:                   return nullptr;
	}
}

bool UMusicSubsystem::OnTick(float DeltaTime)
{
	if (!bIsCrossfading) { return true; }

	FadeProgress += DeltaTime;

	// Fade in active component
	if (IsValid(ActiveComponent) && FadeInTime > 0.f)
	{
		float Alpha = FMath::Clamp(FadeProgress / FadeInTime, 0.f, 1.f);
		ActiveComponent->SetVolumeMultiplier(Alpha);
	}

	// Fade out dying component
	if (IsValid(FadingComponent) && FadeOutTime > 0.f)
	{
		float Alpha = FMath::Clamp(1.f - FadeProgress / FadeOutTime, 0.f, 1.f);
		FadingComponent->SetVolumeMultiplier(Alpha);
		if (Alpha <= 0.f)
		{
			FadingComponent->Stop();
			FadingComponent = nullptr;
		}
	}
	else if (IsValid(FadingComponent))
	{
		FadingComponent->Stop();
		FadingComponent = nullptr;
	}

	// Done when both fades complete
	float MaxTime = FMath::Max(FadeInTime, FadeOutTime);
	if (FadeProgress >= MaxTime)
	{
		if (IsValid(ActiveComponent))
		{
			ActiveComponent->SetVolumeMultiplier(1.f);
		}
		bIsCrossfading = false;
	}
	return true;
}
