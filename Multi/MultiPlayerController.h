// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "MultiPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "MultiPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API AMultiPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetupInputComponent() override;
	virtual void BeginPlay() override;

	void SetAlert(const FString& TextToShow, FLinearColor Color, float Duration);
	void ActivateRespawnTimer();

	UPROPERTY(Transient)
	TObjectPtr<class UHUDWidget> HUDWidgetInstance;

protected:
	void OnJump();
	void OnStopJumping();
	void OnPrimaryAction();
	void OnTeamChat();
	void OnAllChat();
	void OnApplyTag();
	void OnGrenade();

	void OnTag1();
	void OnTag2();
	void OnTag3();
	void OnTag4();
	void OnTag5();

	void MoveForward(float Val);
	void MoveRight(float Val);
	void AddControllerYawInput(float Val);
	void AddControllerPitchInput(float Val);
	void TurnAtRate(float Val);
	void LookUpAtRate(float Val);

	virtual void OnPossess(APawn* aPawn) override;

	void FocusText(EMultiTeam Team = EMultiTeam::None);
	
	UFUNCTION(Client, Reliable)
	void ClientOnPossess();

	UFUNCTION(Server, Reliable)
	void ServerApplyTag(int TagIndex);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<class ATagActor>> TagActorClass;

	int CurrTagIndex = 0;
};
