// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiPlayerState.h"
#include "MultiCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;

// Declaration of the delegate that will be called when the Primary Action is triggered
// It is declared as dynamic so it can be accessed also in Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUseItem);

UCLASS(config=Game)
class AMultiCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Third person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* ThirdPersonCameraComponent;

public:
	AMultiCharacter();

	virtual void Tick(float DeltaSeconds) override;

	bool IsAlive() const { return bIsAlive; };
	void SetAlive(bool Status) { bIsAlive = Status; };

protected:
	virtual void BeginPlay();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> WeaponOnSpawn;

	UPROPERTY(Replicated, Transient)
	AActor* WeaponActor;

	UPROPERTY(Replicated, Transient)
	class AGrenade* GrenadeActor;

	UPROPERTY(Replicated)
	int AmmoNum = 5;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FRotator ReplicatedControllerRotation;

	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive = true;

	UFUNCTION(Server, Reliable)
	void ServerOnPrimaryAction();

	UFUNCTION(Server, Reliable)
	void ServerOnGrenade();

	UFUNCTION()
	void OnRep_Team();

	UFUNCTION()
	void OnRep_IsAlive();

	UPROPERTY(EditDefaultsOnly)
	FName BodyColorParamName = FName(TEXT("BodyColor"));

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UAnimMontage> DeadMontage1P;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class UAnimMontage> DeadMontage3P;

	void PauseAnimMontage();

	void Respawn();

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float TurnRateGamepad;

	/** Delegate to whom anyone can subscribe to receive this event */
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnUseItem OnUseItem;

	void ChangeCanFireGrenade() { bCanFireGrenade = !bCanFireGrenade; }
	void ChangeGrenadeText();
	
	/** Fires a projectile. */
	void OnPrimaryAction();

	void OnGrenade();

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	EMultiTeam Team;

	void SetTeam(EMultiTeam InTeam);
	void Die();

	bool bGrenade = false;
	bool bCanFireGrenade = true;

	int GetAmmo() const { return AmmoNum; }
	void AddAmmo(int AmmoAmount) { AmmoNum += AmmoAmount; }

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> Mesh3P;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USkeletalMeshComponent> Gun3P;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RedColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BlueColor = FLinearColor::Blue;

};

