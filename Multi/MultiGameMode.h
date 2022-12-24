// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MultiGameMode.generated.h"

UCLASS(minimalapi)
class AMultiGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AMultiGameMode();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual bool ShouldSpawnAtStartSpot(AController* Player) override { return false; };
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// Amount of time (in seconds) to wait before starting the match
	UPROPERTY(EditDefaultsOnly)
	float WaitingToStartDuration = 5.0f;

	bool ReadyToEndMatch_Implementation() override;
	virtual void HandleMatchHasEnded() override;
	virtual void HandleMatchIsWaitingToStart() override;

protected:
	virtual void GenericPlayerInitialization(AController* C) override;

private:
	int RedTeamNum = 0;
	int BlueTeamNum = 0;

	float AutoSaveTimer = 5.0f;
};



