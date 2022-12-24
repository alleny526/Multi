// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MultiGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API AMultiGameStateBase : public AGameState
{
	GENERATED_BODY()
	
public:
	AMultiGameStateBase();
	virtual void Tick(float DeltaSeconds) override;

	virtual void HandleMatchIsWaitingToStart() override;

	UPROPERTY(Replicated)
	int RedTeamScore = 0;

	UPROPERTY(Replicated)
	int BlueTeamScore = 0;

	void IncrementRedScore() { RedTeamScore++; }
	void IncrementBlueScore() { BlueTeamScore++; }

	int GetRedScore() const { return RedTeamScore; }
	int GetBlueScore() const { return BlueTeamScore; }

	UPROPERTY(Replicated)
	int NumReadyPlayers = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float WaitingToStartTime;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSendAlert(const FString& TextToShow, FLinearColor Color, float Duration);

	FString AlertString = FString::Printf(TEXT("STARTING MATCH"));

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
