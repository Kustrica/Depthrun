#include "DepthrunHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/HUDOverlayWidget.h"
#include "UI/DebugAdaptiveWidget.h"
#include "UI/ChestRewardWidget.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/AdaptiveEnemy.h"
#include "Player/DepthrunCharacter.h"
#include "RoomGeneration/ChestActor.h"

ADepthrunHUD::ADepthrunHUD() {}

void ADepthrunHUD::ShowChestReward(const FChestRewardPayload& Payload)
{
	if (!ChestRewardWidgetClass) return;
	UChestRewardWidget* Widget = CreateWidget<UChestRewardWidget>(GetOwningPlayerController(), ChestRewardWidgetClass);
	if (Widget)
		Widget->Show(Payload.Diamonds, Payload.Potions, Payload.ItemName,
			Payload.DiamondIcon, Payload.PotionIcon, Payload.ItemIcon);
}

void ADepthrunHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MainWidgetClass)
	{
		MainWidget = CreateWidget<UUserWidget>(GetWorld(), MainWidgetClass);
		if (MainWidget) MainWidget->AddToViewport();
	}

	if (DebugAdaptiveWidgetClass)
	{
		DebugWidget = CreateWidget<UUserWidget>(GetWorld(), DebugAdaptiveWidgetClass);
		if (DebugWidget)
		{
			DebugWidget->AddToViewport(99);
			DebugWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

UHUDOverlayWidget* ADepthrunHUD::GetHUDOverlay() const
{
	return Cast<UHUDOverlayWidget>(MainWidget);
}

void ADepthrunHUD::UpdatePlayerHP(float Current, float Max)
{
	if (UHUDOverlayWidget* Overlay = GetHUDOverlay())
		Overlay->SetHP(Current, Max);
}

void ADepthrunHUD::ToggleAdaptiveDebugWidget()
{
	if (!DebugWidget) return;

	bDebugWidgetVisible = !bDebugWidgetVisible;
	ESlateVisibility NewVisibility = bDebugWidgetVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	DebugWidget->SetVisibility(NewVisibility);

	if (bDebugWidgetVisible)
	{
		AActor* EnemyActor = UGameplayStatics::GetActorOfClass(GetWorld(), AAdaptiveEnemy::StaticClass());
		if (EnemyActor)
		{
			UAdaptiveBehaviorComponent* AdaptiveComp = EnemyActor->FindComponentByClass<UAdaptiveBehaviorComponent>();
			if (AdaptiveComp)
			{
				if (UDebugAdaptiveWidget* CastWidget = Cast<UDebugAdaptiveWidget>(DebugWidget))
					CastWidget->BindToComponent(AdaptiveComp);
			}
		}
	}
}

void ADepthrunHUD::RegisterChest(AChestActor* Chest)
{
	if (!Chest || SubscribedChests.Contains(Chest)) return;
	SubscribedChests.Add(Chest);
	Chest->OnChestOpened.AddUObject(this, &ADepthrunHUD::ShowChestReward);
}
