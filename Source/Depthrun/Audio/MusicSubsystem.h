// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio/MusicTypes.h"
#include "MusicSubsystem.generated.h"

class USoundBase;
class UAudioComponent;

/**
 * UMusicSubsystem
 * UGameInstanceSubsystem (persists across level transitions).
 * Manages adaptive music with crossfade between Hub / Explore / Combat tracks.
 *
 * Crossfade timings:
 *   Hub → Explore: 1.5s linear
 *   Explore → Combat: 0.3s (fast)
 *   Combat → Explore: 2.0s (slow)
 *   Any → Hub: 1.5s
 */
UCLASS()
class DEPTHRUN_API UMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Fade in the given track. If another track is playing it will be faded out.
	 * @param Track    Which track to play.
	 * @param FadeIn   Seconds to fade-in the new track (0 = instant).
	 * @param FadeOut  Seconds to fade-out the old track (0 = instant stop).
	 */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void PlayMusic(EMusicTrack Track, float FadeIn = 1.5f, float FadeOut = 1.5f);

	/** Stop all music. */
	UFUNCTION(BlueprintCallable, Category = "Music")
	void StopMusic(float FadeOut = 1.0f);

	/** Returns currently active (or transitioning-to) track. */
	UFUNCTION(BlueprintPure, Category = "Music")
	EMusicTrack GetCurrentTrack() const { return CurrentTrack; }

	// ─── Track references — assign in BP_DepthrunGameInstance defaults ────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Tracks")
	TObjectPtr<USoundBase> HubMusic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Tracks")
	TObjectPtr<USoundBase> ExploreMusic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Music|Tracks")
	TObjectPtr<USoundBase> CombatMusic;

private:
	/** Returns the SoundBase for a given track enum. */
	USoundBase* GetSoundForTrack(EMusicTrack Track) const;

	/** Get or create an AudioComponent for a track. */
	UAudioComponent* GetOrCreateComponent(EMusicTrack Track);

	/** Tick delegate — handles volume interpolation. */
	bool OnTick(float DeltaTime);
	FTSTicker::FDelegateHandle TickHandle;

	/** Master volume multiplier (0.0-1.0). Used by settings. */
	UPROPERTY()
	float MasterVolume = 1.f;

	/** Volume reduction during crossfade transitions. */
	UPROPERTY()
	float TransitionDuckMultiplier = 0.8f;

	/** Audio components for each track (persist across switches, paused when inactive). */
	UPROPERTY()
	TMap<EMusicTrack, TObjectPtr<UAudioComponent>> TrackComponents;

	/** Currently playing/active component. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> ActiveComponent;

	/** Component that is fading out to pause. */
	UPROPERTY()
	TObjectPtr<UAudioComponent> FadingComponent;

	EMusicTrack CurrentTrack = EMusicTrack::None;
	EMusicTrack PendingTrack = EMusicTrack::None;

	float FadeInTime = 0.f;
	float FadeOutTime = 0.f;
	float FadeProgress = 0.f;
	bool bIsCrossfading = false;
};
