// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MultiPlayerState.generated.h"

UENUM(BlueprintType)
enum class EMultiTeam : uint8
{
	/* Team not assigned yet*/
	None,
	/* On red team */
	Red,
	/* On blue team */
	Blue
};

/**
 * 
 */
UCLASS()
class MULTI_API AMultiPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	AMultiPlayerState();

	UPROPERTY(Replicated)
	EMultiTeam Team = EMultiTeam::None;

	UPROPERTY(Replicated)
	int PlayerScore = 0;

	EMultiTeam GetTeam() const { return Team; }
	void SetRedTeam() { Team = EMultiTeam::Red; }
	void SetBlueTeam() { Team = EMultiTeam::Blue; }

	void IncrementScore() { PlayerScore++; }
	int GetScore() const { return PlayerScore; }

	int Streak = 0;
	void IncrementStreak();
	int GetStreaks() const { return Streak; };
	void ClearStreaks() { Streak = 0; };

	UFUNCTION(Server, Reliable)
	void ServerSendMessage(EMultiTeam TeamParam, const FString& Sender, const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveMessage(EMultiTeam TeamParam, const FString& Sender, const FString& Message);

	UFUNCTION(Server, Reliable)
	void ServerSendReadyInfo(bool IsReady);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly)
	TMap<int, FString> StreakStringMap;
};
