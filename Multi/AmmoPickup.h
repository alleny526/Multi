// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoPickup.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSphereComponentOnOverlap);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTI_API AAmmoPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoPickup();

	UFUNCTION()
	void HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UTextRenderComponent> TextComp;

	UPROPERTY(EditAnywhere)
	int AmmoAmount = 5;

public:
	UPROPERTY(BlueprintAssignable)
	FSphereComponentOnOverlap OnOverlap;
};
