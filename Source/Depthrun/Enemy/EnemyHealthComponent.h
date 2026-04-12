#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnemyHealthChanged, float, OldHP, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEPTHRUN_API UEnemyHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEnemyHealthComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHP;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHP;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDeath OnDeath;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHP() const { return CurrentHP; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHP() const { return MaxHP; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const { return CurrentHP <= 0.0f; }
};
