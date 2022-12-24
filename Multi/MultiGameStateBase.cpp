// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MultiGameStateBase.h"
#include <Net/UnrealNetwork.h>
#include "MultiGameMode.h"
#include "kismet/GameplayStatics.h"
#include "MultiPlayerController.h"

AMultiGameStateBase::AMultiGameStateBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMultiGameStateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetMatchState() == MatchState::WaitingToStart)
	{
		if (PlayerArray.Num() > 0 && NumReadyPlayers == PlayerArray.Num())
		{
			WaitingToStartTime -= DeltaSeconds;
			if (WaitingToStartTime <= 0.0f)
			{
				WaitingToStartTime = 0.0f;
				if (GetLocalRole() == ROLE_Authority)
				{
					if (AMultiGameMode* GameMode = GetWorld()->GetAuthGameMode<AMultiGameMode>())
					{
						GameMode->StartMatch();

						MulticastSendAlert(AlertString, FLinearColor::Green, 3.0f);
					}
				}
			}
		}
	}
}

void AMultiGameStateBase::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	
	if (GetLocalRole() == ROLE_Authority)
	{
		WaitingToStartTime = GetDefaultGameMode<AMultiGameMode>()->WaitingToStartDuration;
	}
}

void AMultiGameStateBase::MulticastSendAlert_Implementation(const FString& TextToShow, FLinearColor Color, float Duration)
{
	if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->IsLocalController())
	{
		Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0))->SetAlert(TextToShow, Color, Duration);
	}
}

void AMultiGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiGameStateBase, RedTeamScore);
	DOREPLIFETIME(AMultiGameStateBase, BlueTeamScore);
	DOREPLIFETIME(AMultiGameStateBase, NumReadyPlayers);

	// Conditionally replicate the waiting to start time only on initial replication
	DOREPLIFETIME_CONDITION(AMultiGameStateBase, WaitingToStartTime, COND_InitialOnly);
}
