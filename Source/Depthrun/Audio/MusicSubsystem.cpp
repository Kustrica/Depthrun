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

	// Save current track position before crossfading
	if (IsValid(ActiveComponent) && CurrentTrack != EMusicTrack::None)
	{
		SaveTrackPosition(CurrentTrack, ActiveComponent);
	}

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

	// Get saved position and play from there
	float StartTime = 0.f;
	if (float* SavedPos = TrackPlaybackPositions.Find(Track))
	{
		StartTime = *SavedPos;
		UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Resuming track %d from %.2fs"), (int32)Track, StartTime);
	}

	if (FadeIn > 0.f)
	{
		ActiveComponent->SetVolumeMultiplier(0.f);
	}
	ActiveComponent->Play(StartTime);

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
			// Restore full volume (no ducking)
			ActiveComponent->SetVolumeMultiplier(1.f * MasterVolume);
		}
		bIsCrossfading = false;
	}
	return true;
}

void UMusicSubsystem::SaveTrackPosition(EMusicTrack Track, UAudioComponent* Component)
{
	if (!IsValid(Component)) { return; }
	float Position = Component->GetPlaybackTime();
	TrackPlaybackPositions.Add(Track, Position);
	UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Saved position for track %d: %.2fs"), (int32)Track, Position);
}

void UMusicSubsystem::ResumeTrackFromPosition(EMusicTrack Track, UAudioComponent* Component)
{
	if (!IsValid(Component)) { return; }

	if (float* SavedPos = TrackPlaybackPositions.Find(Track))
	{
		UE_LOG(LogDepthrunMusic, Log, TEXT("[Music] Resuming track %d from %.2fs"), (int32)Track, *SavedPos);
		Component->SetSound(GetSoundForTrack(Track)); // Ensure sound is set
		Component->SetIntParameter(TEXT("PlaybackTime"), static_cast<int32>(*SavedPos * 1000)); // Try to set time
		// Note: SetPlaybackTime doesn't exist in this API version, we'll use Play with start time
	}
}
