// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiCharacter.h"
#include "MultiProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "Net/UnrealNetwork.h"
#include "TP_WeaponComponent.h"
#include "MultiPlayerController.h"
#include "MultiGameMode.h"
#include "HUDWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/CharacterMovementComponent.h"


//////////////////////////////////////////////////////////////////////////
// AMultiCharacter

AMultiCharacter::AMultiCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	TurnRateGamepad = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh3P"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(RootComponent);
	Mesh3P->bCastDynamicShadow = true;
	Mesh3P->CastShadow = true;
	Mesh3P->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	Mesh3P->SetRelativeLocation(FVector(0.0f, 0.0f, -100.0f));

	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(Mesh3P);
	ThirdPersonCameraComponent->bUsePawnControlRotation = false;
	ThirdPersonCameraComponent->bAutoActivate = false;
	ThirdPersonCameraComponent->SetRelativeLocation(FVector(0.0f, -400.0f, 350.0f));
	ThirdPersonCameraComponent->SetRelativeRotation(FRotator(-45.0f, 90.0f, 0.0f));

	Gun3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterGun3P"));
	Gun3P->SetOwnerNoSee(true);
	Gun3P->SetupAttachment(RootComponent);
	Gun3P->bCastDynamicShadow = true;
	Gun3P->CastShadow = true;

}

void AMultiCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetLocalRole() == ROLE_Authority)
	{
		ReplicatedControllerRotation = GetControlRotation();
	}
}

void AMultiCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority && WeaponOnSpawn)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.Instigator = this;
		WeaponActor = GetWorld()->SpawnActor<AActor>(WeaponOnSpawn, GetActorLocation(), FRotator::ZeroRotator, Params);
		UTP_WeaponComponent* WeaponComp = WeaponActor ? WeaponActor->FindComponentByClass<UTP_WeaponComponent>() : nullptr;
		if (WeaponComp)
		{
			WeaponComp->AttachWeapon(this);
		}
	}

	if (Gun3P && Mesh3P)
	{
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		Gun3P->AttachToComponent(Mesh3P, AttachmentRules, FName(TEXT("GripPoint")));
	}
}

void AMultiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiCharacter, WeaponActor);
	DOREPLIFETIME(AMultiCharacter, Team);
	DOREPLIFETIME(AMultiCharacter, AmmoNum);
	DOREPLIFETIME(AMultiCharacter, bIsAlive);
	DOREPLIFETIME(AMultiCharacter, ReplicatedControllerRotation);
}

void AMultiCharacter::OnRep_Team()
{
	if (Team == EMultiTeam::Red)
	{
		Mesh1P->SetVectorParameterValueOnMaterials(BodyColorParamName, FVector(RedColor.R, RedColor.G, RedColor.B));
		Mesh3P->SetVectorParameterValueOnMaterials(BodyColorParamName, FVector(RedColor.R, RedColor.G, RedColor.B));
	}
	else
	{
		Mesh1P->SetVectorParameterValueOnMaterials(BodyColorParamName, FVector(BlueColor.R, BlueColor.G, BlueColor.B));
		Mesh3P->SetVectorParameterValueOnMaterials(BodyColorParamName, FVector(BlueColor.R, BlueColor.G, BlueColor.B));
	}
}

void AMultiCharacter::OnRep_IsAlive()
{
	Mesh1P->SetOwnerNoSee(true);
	WeaponActor->FindComponentByClass<USkeletalMeshComponent>()->SetOwnerNoSee(true);

	Mesh3P->SetOwnerNoSee(false);
	Gun3P->SetOwnerNoSee(false);

	FirstPersonCameraComponent->Deactivate();
	ThirdPersonCameraComponent->Activate();

	FString AlertString = FString::Printf(TEXT("You are dead :("));
	if (GetController())
	{
		Cast<AMultiPlayerController>(GetController())->SetAlert(AlertString, FLinearColor::White, 5.0f);
		Cast<AMultiPlayerController>(GetController())->ActivateRespawnTimer();
	}
	
	if (Mesh3P && Mesh1P)
	{
		FTimerHandle TimerHandle3P;
		float DeadMontage3PLength = Mesh3P->GetAnimInstance()->Montage_Play(DeadMontage3P);
		GetWorldTimerManager().SetTimer(TimerHandle3P, this, &AMultiCharacter::PauseAnimMontage, DeadMontage3PLength - 0.25f);

		FTimerHandle TimerHandle1P;
		float DeadMontage1PLength = Mesh1P->GetAnimInstance()->Montage_Play(DeadMontage1P);
		GetWorldTimerManager().SetTimer(TimerHandle1P, this, &AMultiCharacter::PauseAnimMontage, DeadMontage1PLength - 0.25f);
	}
}

void AMultiCharacter::PauseAnimMontage()
{
	Mesh3P->GetAnimInstance()->Montage_Pause();
	Mesh1P->GetAnimInstance()->Montage_Pause();
}

void AMultiCharacter::Respawn()
{
	AMultiPlayerController* C = GetController<AMultiPlayerController>();
	C->UnPossess();
	GetWorld()->GetAuthGameMode<AMultiGameMode>()->RestartPlayer(C);
	Destroy();
}

void AMultiCharacter::OnPrimaryAction()
{
	ServerOnPrimaryAction();
}

void AMultiCharacter::OnGrenade()
{
	if (bCanFireGrenade)
	{
		bCanFireGrenade = false;

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AMultiCharacter::ChangeCanFireGrenade, 5.0f);

		Cast<AMultiPlayerController>(GetController())->HUDWidgetInstance->Grenade->SetVisibility(ESlateVisibility::Hidden);
		FTimerHandle GrenadeTextTimerHandle;
		GetWorldTimerManager().SetTimer(GrenadeTextTimerHandle, this, &AMultiCharacter::ChangeGrenadeText, 5.0f);

		ServerOnGrenade();
	}
}

void AMultiCharacter::SetTeam(EMultiTeam InTeam)
{
	Team = InTeam;
	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		OnRep_Team();
	}
}

void AMultiCharacter::Die()
{
	bIsAlive = false;

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetPlayerState<AMultiPlayerState>()->ClearStreaks();

	FTimerHandle DeadTimerHandle;
	GetWorldTimerManager().SetTimer(DeadTimerHandle, this, &AMultiCharacter::Respawn, 5.0f);
	if (GetWorld()->GetNetMode() == NM_ListenServer)
	{
		OnRep_IsAlive();
	}
}

void AMultiCharacter::ServerOnPrimaryAction_Implementation()
{
	if (AmmoNum > 0 && bIsAlive)
	{
		bGrenade = false;

		// Trigger the OnItemUsed Event
		OnUseItem.Broadcast();
		AmmoNum--;
	}
}

void AMultiCharacter::ServerOnGrenade_Implementation()
{
	if (bIsAlive)
	{
		bGrenade = true;

		// Trigger the OnItemUsed Event
		OnUseItem.Broadcast();
	}
}

void AMultiCharacter::ChangeGrenadeText()
{
	Cast<AMultiPlayerController>(GetController())->HUDWidgetInstance->Grenade->SetVisibility(ESlateVisibility::HitTestInvisible);
}