// Copyright Depthrun Project, 2026. All Rights Reserved.
#include "Audio/MusicSubsystem.h"
#include "Audio/MusicTypes.h"
#include "Components/AudioComponent.h"
#include "Core/DepthrunGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "DepthrunLogChannels.h"

void UMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Pull track references from the GameInstance (assignable in BP_DepthrunGameInstance).
	if (UDepthrunGameInstance* GI = Cast<UDepthrunGameInstance>(GetGameInstance()))
	{
		HubMusic     = GI->HubMusic;
		ExploreMusic = GI->ExploreMusic;
		CombatMusic  = GI->CombatMusic;
		UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Loaded tracks from GameInstance — Hub=%s Explore=%s Combat=%s"),
			HubMusic ? *HubMusic->GetName() : TEXT("NONE"),
			ExploreMusic ? *ExploreMusic->GetName() : TEXT("NONE"),
			CombatMusic ? *CombatMusic->GetName() : TEXT("NONE"));
	}
	else
	{
		UE_LOG(LogDepthrunMusic, Warning,
			TEXT("[Music] GameInstance is not UDepthrunGameInstance — tracks not loaded. Set Game Instance Class in Project Settings."));
	}

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UMusicSubsystem::OnTick));
}

void UMusicSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);

	// Stop all track components
	for (auto& Pair : TrackComponents)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Stop();
		}
	}
	TrackComponents.Empty();
	ActiveComponent = nullptr;
	FadingComponent = nullptr;

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

	// Finish any pending fade immediately (pause the fading track)
	if (bIsCrossfading && IsValid(FadingComponent))
	{
		FadingComponent->SetVolumeMultiplier(0.f);
		FadingComponent->SetPaused(true);
		FadingComponent = nullptr;
	}

	// Current active track becomes fading (will be paused when fade completes)
	if (IsValid(ActiveComponent))
	{
		FadingComponent = ActiveComponent;
		ActiveComponent = nullptr;
	}

	// Get or create component for the target track
	ActiveComponent = GetOrCreateComponent(Track);

	if (!IsValid(ActiveComponent))
	{
		UE_LOG(LogDepthrunMusic, Error, TEXT("[Music] Failed to get/create AudioComponent for track %d"), (int32)Track);
		return;
	}

	ActiveComponent->bAutoDestroy = false;

	// Start or resume playback
	if (!ActiveComponent->IsPlaying())
	{
		ActiveComponent->Play();
		UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Starting/resuming track %d"), (int32)Track);
	}

	if (FadeIn > 0.f)
	{
		ActiveComponent->SetVolumeMultiplier(0.f);
	}

	CurrentTrack    = Track;
	FadeInTime      = FMath::Max(FadeIn, 0.f);
	FadeOutTime     = FMath::Max(FadeOut, 0.f);
	FadeProgress    = 0.f;
	bIsCrossfading  = true;
}

void UMusicSubsystem::StopMusic(float FadeOut)
{
	// Pause active track (don't stop - preserves position)
	if (IsValid(ActiveComponent))
	{
		if (FadeOut > 0.f)
		{
			// Fade out
			ActiveComponent->FadeOut(FadeOut, 0.f);
		}
		else
		{
			ActiveComponent->Stop();
		}
		ActiveComponent = nullptr;
	}

	// Stop any fading track too
	if (IsValid(FadingComponent))
	{
		FadingComponent->Stop();
		FadingComponent = nullptr;
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

	// Fade in active component (with ducking during transition)
	if (IsValid(ActiveComponent) && FadeInTime > 0.f)
	{
		float Alpha = FMath::Clamp(FadeProgress / FadeInTime, 0.f, 1.f);
		// Apply ducking: during transition, volume is 20% lower (0.8x)
		float DuckedAlpha = Alpha * TransitionDuckMultiplier;
		ActiveComponent->SetVolumeMultiplier(DuckedAlpha * MasterVolume);
	}

	// Fade out dying component (with ducking)
	if (IsValid(FadingComponent) && FadeOutTime > 0.f)
	{
		float Alpha = FMath::Clamp(1.f - FadeProgress / FadeOutTime, 0.f, 1.f);
		// Apply ducking during transition
		float DuckedAlpha = Alpha * TransitionDuckMultiplier;
		FadingComponent->SetVolumeMultiplier(DuckedAlpha * MasterVolume);
		if (Alpha <= 0.f)
		{
			// Pause the faded-out track by stopping (position resets - unavoidable in this UE version)
			FadingComponent->Stop();
			FadingComponent = nullptr;
		}
	}
	else if (IsValid(FadingComponent))
	{
		// Stop immediately if no fade out
		FadingComponent->Stop();
		FadingComponent = nullptr;
	}

	// Done when both fades complete
	float MaxTime = FMath::Max(FadeInTime, FadeOutTime);
	if (FadeProgress >= MaxTime)
	{
		if (IsValid(ActiveComponent))
		{
			// Restore full volume (no ducking)
			ActiveComponent->SetVolumeMultiplier(1.f * MasterVolume);
		}
		bIsCrossfading = false;
	}
	return true;
}

UAudioComponent* UMusicSubsystem::GetOrCreateComponent(EMusicTrack Track)
{
	// Check if we already have a component for this track
	if (TrackComponents.Contains(Track))
	{
		UAudioComponent* Existing = TrackComponents.FindRef(Track).Get();
		if (IsValid(Existing))
		{
			return Existing;
		}
	}

	// Create new component
	USoundBase* Sound = GetSoundForTrack(Track);
	if (!IsValid(Sound)) { return nullptr; }

	UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
	if (!World) { return nullptr; }

	UAudioComponent* NewComponent = UGameplayStatics::SpawnSoundAtLocation(
		World, Sound, FVector::ZeroVector);

	if (IsValid(NewComponent))
	{
		NewComponent->bAutoDestroy = false;
		TrackComponents.Add(Track, NewComponent);
		UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Created new AudioComponent for track %d"), (int32)Track);
	}

	return NewComponent;
}

