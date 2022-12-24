// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "MultiCharacter.h"
#include "MultiProjectile.h"
#include "Grenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);

	SetIsReplicatedByDefault(true);
}


void UTP_WeaponComponent::Fire()
{
	if(Character == nullptr || Character->GetController() == nullptr || !Character->IsAlive())
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass.Num() != 0)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);
	
			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = Character;

			if (!Character->bGrenade)
			{
				// Spawn the projectile at the muzzle
				AMultiProjectile* CurrMultiProjectile = World->SpawnActor<AMultiProjectile>(ProjectileClass[0], SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
			else
			{
				FRotator GrenadeSpawnRotation = SpawnRotation + FRotator(0.0f, 0.0f, 90.0f);
				// Spawn the projectile at the muzzle
				AGrenade* CurrGrenade = World->SpawnActor<AGrenade>(ProjectileClass[1], SpawnLocation, GrenadeSpawnRotation, ActorSpawnParams);
			}

			MulticastOnFireEffects();
		}
	}
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(Character != nullptr)
	{
		// Unregister from the OnUseItem Event
		Character->OnUseItem.RemoveDynamic(this, &UTP_WeaponComponent::Fire);
	}
}

void UTP_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTP_WeaponComponent, Character);
}

void UTP_WeaponComponent::MulticastOnFireEffects_Implementation()
{
	if (Character)
	{
		// Try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
		}

		// Try and play a firing animation if specified
		if (FireAnimation != nullptr && FireAnimation3P != nullptr)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
			UAnimInstance* AnimInstance3P = Character->GetMesh3P()->GetAnimInstance();
			if (AnimInstance != nullptr)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
				AnimInstance3P->Montage_Play(FireAnimation3P, 1.f);
			}
		}
	}
}

void UTP_WeaponComponent::AttachWeapon(AMultiCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if(Character != nullptr)
	{
		// Attach the weapon to the First Person Character
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		GetOwner()->AttachToComponent(Character->GetMesh1P(),AttachmentRules, FName(TEXT("GripPoint")));

		// Register so that Fire is called every time the character tries to use the item being held
		Character->OnUseItem.AddDynamic(this, &UTP_WeaponComponent::Fire);
	}
}

