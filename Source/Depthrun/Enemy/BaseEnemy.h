#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "BaseEnemy.generated.h"

class UEnemyHealthComponent;

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Melee      UMETA(DisplayName = "Melee"),
	Ranged     UMETA(DisplayName = "Ranged"),
	Adaptive   UMETA(DisplayName = "Adaptive")
};

UCLASS(Abstract)
class DEPTHRUN_API ABaseEnemy : public APaperCharacter
{
	GENERATED_BODY()

public:
	ABaseEnemy();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEnemyHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyType EnemyType;

	// Callbacks for Health
	UFUNCTION()
	virtual void OnDeath();

public:
	virtual void Tick(float DeltaTime) override;

	// Virtual methods specified in PLAN.MD
	virtual void OnSpawned();
	virtual void OnKilled();
};
