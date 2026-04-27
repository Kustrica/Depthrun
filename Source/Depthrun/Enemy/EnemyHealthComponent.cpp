#include "EnemyHealthComponent.h"
#include "GameFramework/Actor.h"

UEnemyHealthComponent::UEnemyHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxHP = 100.0f;
	CurrentHP = MaxHP;
}

void UEnemyHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UEnemyHealthComponent::HandleTakeAnyDamage);
	}
}

void UEnemyHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || IsDead())
	{
		return;
	}

	ApplyDamage(Damage);
}

void UEnemyHealthComponent::ApplyDamage(float Amount)
{
	if (Amount <= 0.0f || IsDead())
	{
		return;
	}

	float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP - Amount, 0.0f, MaxHP);

	OnHealthChanged.Broadcast(OldHP, CurrentHP, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		OnDeath.Broadcast();
	}
}

float UEnemyHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.0f || IsDead())
	{
		return 0.0f;
	}

	float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + Amount, 0.0f, MaxHP);
	float ActualHealed = CurrentHP - OldHP;

	OnHealthChanged.Broadcast(OldHP, CurrentHP, MaxHP);
	return ActualHealed;
}
