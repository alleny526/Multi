// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "MultiCharacter.h"
#include "MultiGameStateBase.h"	
#include <GameFramework/GameMode.h>
#include "Components/CapsuleComponent.h"

AMultiProjectile::AMultiProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AMultiProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AMultiProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		TMap<EMultiTeam, FLinearColor> TeamColor;
		TeamColor.Add(TPair<EMultiTeam, FLinearColor>(EMultiTeam::Red, FLinearColor::Red));
		TeamColor.Add(TPair<EMultiTeam, FLinearColor>(EMultiTeam::Blue, FLinearColor::Blue));

		// Only add impulse and destroy projectile if we hit a physics
		if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) && OtherComp->IsSimulatingPhysics())
		{
			OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());

			Destroy();
		}
		if (GetWorld()->GetGameState<AMultiGameStateBase>()->GetMatchState() != MatchState::WaitingPostMatch)
		{

			if ((OtherActor != nullptr) && (OtherActor->IsA<AMultiCharacter>()))
			{
				AMultiCharacter* OtherMultiChar = Cast<AMultiCharacter>(OtherActor);

				AMultiPlayerState* InstigatorPlayerState = GetInstigator()->GetPlayerState<AMultiPlayerState>();
				AMultiPlayerState* OtherPlayerState = OtherMultiChar->GetPlayerState<AMultiPlayerState>();
				if (InstigatorPlayerState->GetTeam() != OtherPlayerState->GetTeam() && OtherMultiChar->IsAlive())
				{
					InstigatorPlayerState->IncrementScore();
					InstigatorPlayerState->IncrementStreak();

					if (OtherPlayerState->GetStreaks() >= 3)
					{
						FString EndStreakString = InstigatorPlayerState->GetPlayerName() + FString::Printf(TEXT(" ended ")) + OtherPlayerState->GetPlayerName() + FString::Printf(TEXT("'s streak!"));
						GetWorld()->GetGameState<AMultiGameStateBase>()->MulticastSendAlert(EndStreakString, TeamColor.FindRef(InstigatorPlayerState->Team), 2.5f);
					}

					OtherMultiChar->Die();
					UE_LOG(LogTemp, Display, TEXT("PlayerInstigator Score: %d"), InstigatorPlayerState->GetScore());

					if (InstigatorPlayerState->GetTeam() == EMultiTeam::Red)
					{
						GetWorld()->GetGameState<AMultiGameStateBase>()->IncrementRedScore();
						UE_LOG(LogTemp, Display, TEXT("Red Team Score: %d"), GetWorld()->GetGameState<AMultiGameStateBase>()->GetRedScore());
					}
					else
					{
						GetWorld()->GetGameState<AMultiGameStateBase>()->IncrementBlueScore();
						UE_LOG(LogTemp, Display, TEXT("Blue Team Score: %d"), GetWorld()->GetGameState<AMultiGameStateBase>()->GetBlueScore());
					}

					Destroy();
				}
			}
		}
	}
}