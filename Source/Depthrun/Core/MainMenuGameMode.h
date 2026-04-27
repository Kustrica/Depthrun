// Copyright Depthrun Project, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

class UPaperTileMap;

/**
 * AMainMenuGameMode
 * GameMode for the L_MainMenu level.
 * On BeginPlay: plays Hub music, spawns MainMenuWidget.
 * Background is a single dungeon tilemap cell — set BackgroundTilemap and
 * BackgroundTileIndex in the Blueprint child (BP_MainMenuGameMode).
 */
UCLASS()
class DEPTHRUN_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

protected:
	virtual void BeginPlay() override;

public:
	/** Widget class to show as the main menu. Assign WBP_MainMenu. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	/** Optional: tilemap asset used as background image. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu|Background")
	TObjectPtr<UPaperTileMap> BackgroundTilemap;

	/** Which tile index (row-major) in the tileset to use as background. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MainMenu|Background")
	int32 BackgroundTileIndex = 0;
};
