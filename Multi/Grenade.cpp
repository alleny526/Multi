// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "Grenade.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include <Kismet/GameplayStatics.h>
#include "MultiCharacter.h"
#include "EngineUtils.h"
#include "MultiPlayerState.h"
#include "MultiGameStateBase.h"

// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CollisionComp->SetCapsuleSize(5.0f, 10.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");

	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;
	ProjectileMovement->ProjectileGravityScale = 1.5f;

	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle ExplosionTimerHandle;
	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AGrenade::Explode, 3.0f);
}

void AGrenade::Explode()
{
	UGameplayStatics::SpawnSoundAttached(ExplodeSoundBase, CollisionComp);
	UGameplayStatics::SpawnEmitterAttached(ExplodeParticleSystem, CollisionComp);

	if (GetLocalRole() == ROLE_Authority)
	{
		AMultiCharacter* GrenadeInstigator = Cast<AMultiCharacter>(GetInstigator());
		AMultiPlayerState* InstigatorPlayerState = GetInstigator()->GetPlayerState<AMultiPlayerState>();

		for (TActorIterator<AMultiCharacter> It(GetWorld()); It; ++It)
		{
			if (FVector::Distance(It->GetActorLocation(), this->GetActorLocation()) <= 250.0f && *It != GrenadeInstigator)
			{
				if (It->Team != GrenadeInstigator->Team && It->IsAlive())
				{
					It->Die();
					InstigatorPlayerState->IncrementScore();

					if (InstigatorPlayerState->GetTeam() == EMultiTeam::Red)
					{
						GetWorld()->GetGameState<AMultiGameStateBase>()->IncrementRedScore();
					}
					else
					{
						GetWorld()->GetGameState<AMultiGameStateBase>()->IncrementBlueScore();
					}
				}
			}
		}

		if (FVector::Distance(GrenadeInstigator->GetActorLocation(), this->GetActorLocation()) <= 250.0f)
		{
			GrenadeInstigator->Die();
		}

		FTimerHandle DestroyTimerHandle;
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AGrenade::DestroyGrenade, 0.2f);
	}
}

void AGrenade::DestroyGrenade()
{
	Destroy();
}