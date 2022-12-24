// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class UCapsuleComponent;
class UProjectileMovementComponent;

UCLASS()
class MULTI_API AGrenade : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly)
	UCapsuleComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovement;
	
public:	
	// Sets default values for this actor's properties
	AGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	void Explode();

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ExplodeSoundBase;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> ExplodeParticleSystem;

	void DestroyGrenade();
};
