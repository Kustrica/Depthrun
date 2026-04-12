#include "BaseEnemy.h"
#include "EnemyHealthComponent.h"
#include "Components/CapsuleComponent.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true; // May be needed for FSM later, or timers might suffice

	HealthComponent = CreateDefaultSubobject<UEnemyHealthComponent>(TEXT("HealthComponent"));
	
	// Default orientation
	EnemyType = EEnemyType::Melee;

	// Configure collision so projectiles can hit the enemy
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
		GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	}
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ABaseEnemy::OnDeath);
	}

	OnSpawned();
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseEnemy::OnSpawned()
{
	// Log or initialize when spawned
	UE_LOG(LogTemp, Log, TEXT("ABaseEnemy::OnSpawned -> %s"), *GetName());
}

void ABaseEnemy::OnKilled()
{
	// Disable collision, visually die, or destroy
	UE_LOG(LogTemp, Log, TEXT("ABaseEnemy::OnKilled -> %s"), *GetName());
	
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	SetLifeSpan(3.0f); // Destroy after 3 seconds
}

void ABaseEnemy::OnDeath()
{
	OnKilled();
}
