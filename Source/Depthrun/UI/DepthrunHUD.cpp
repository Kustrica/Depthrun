#include "DepthrunHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/HUDOverlayWidget.h"
#include "UI/DebugAdaptiveWidget.h"
#include "AdaptiveBehavior/AdaptiveBehaviorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/AdaptiveEnemy.h"

ADepthrunHUD::ADepthrunHUD() {}

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
	// Logic for HP bar update in MainWidget will be added in Stage 9
}

void ADepthrunHUD::ToggleAdaptiveDebugWidget()
{
	if (!DebugWidget) return;

	bDebugWidgetVisible = !bDebugWidgetVisible;
	ESlateVisibility NewVisibility = bDebugWidgetVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
	DebugWidget->SetVisibility(NewVisibility);

	if (bDebugWidgetVisible)
	{
		// Find an adaptive enemy to bind to
		AActor* EnemyActor = UGameplayStatics::GetActorOfClass(GetWorld(), AAdaptiveEnemy::StaticClass());
		if (EnemyActor)
		{
			UAdaptiveBehaviorComponent* AdaptiveComp = EnemyActor->FindComponentByClass<UAdaptiveBehaviorComponent>();
			if (AdaptiveComp)
			{
				UDebugAdaptiveWidget* CastWidget = Cast<UDebugAdaptiveWidget>(DebugWidget);
				if (CastWidget)
				{
					CastWidget->BindToComponent(AdaptiveComp);
				}
			}
		}
	}
}
