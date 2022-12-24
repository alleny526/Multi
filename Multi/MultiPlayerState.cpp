// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MultiPlayerState.h"
#include <Net/UnrealNetwork.h>
#include <Kismet/GameplayStatics.h>
#include "MultiPlayerController.h"
#include "MultiGameStateBase.h"
#include "HUDWidget.h"

AMultiPlayerState::AMultiPlayerState()
{
	NetUpdateFrequency = 4;

	StreakStringMap.Add(TPair<int, FString>(3, FString::Printf(TEXT(" is on a killing spree!"))));
	StreakStringMap.Add(TPair<int, FString>(5, FString::Printf(TEXT(" is dominating!"))));
	StreakStringMap.Add(TPair<int, FString>(7, FString::Printf(TEXT(" is UNSTOPPABLE!"))));
	StreakStringMap.Add(TPair<int, FString>(9, FString::Printf(TEXT(" is GODLIKE!"))));
}

void AMultiPlayerState::IncrementStreak()
{
	Streak++;
	TMap<EMultiTeam, FLinearColor> TeamColor;
	TeamColor.Add(TPair<EMultiTeam, FLinearColor>(EMultiTeam::Red, FLinearColor::Red));
	TeamColor.Add(TPair<EMultiTeam, FLinearColor>(EMultiTeam::Blue, FLinearColor::Blue));

	if (GetLocalRole() == ROLE_Authority)
	{
		if (StreakStringMap.Contains(Streak))
		{
			FString StreakString = GetPlayerName() + StreakStringMap.FindRef(Streak);
			GetWorld()->GetGameState<AMultiGameStateBase>()->MulticastSendAlert(StreakString, TeamColor.FindRef(Team), 2.5f);
		}
	}
}

void AMultiPlayerState::ServerSendMessage_Implementation(EMultiTeam TeamParam, const FString& Sender, const FString& Message)
{
	AMultiGameStateBase* GameState = GetWorld()->GetGameState<AMultiGameStateBase>();
	for (auto State : GameState->PlayerArray)
	{
		AMultiPlayerState* PlayerState = Cast<AMultiPlayerState>(State);
		if (TeamParam == EMultiTeam::None)
		{
			PlayerState->ClientReceiveMessage(TeamParam, Sender, Message);
		}
		else if (TeamParam == EMultiTeam::Red)
		{
			if (PlayerState->Team == EMultiTeam::Red)
			{
				PlayerState->ClientReceiveMessage(TeamParam, Sender, Message);
			}
		}
		else
		{
			if (PlayerState->Team == EMultiTeam::Blue)
			{
				PlayerState->ClientReceiveMessage(TeamParam, Sender, Message);
			}
		}
	}
}

void AMultiPlayerState::ClientReceiveMessage_Implementation(EMultiTeam TeamParam, const FString& Sender, const FString& Message)
{
	Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(this, 0))->HUDWidgetInstance->AddChatMessage(TeamParam, Sender, Message);
}

void AMultiPlayerState::ServerSendReadyInfo_Implementation(bool IsReady)
{
	if (IsReady)
	{
		GetWorld()->GetGameState<AMultiGameStateBase>()->NumReadyPlayers++;
	}
	else
	{
		GetWorld()->GetGameState<AMultiGameStateBase>()->NumReadyPlayers--;
	}
}

void AMultiPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiPlayerState, Team);
	DOREPLIFETIME(AMultiPlayerState, PlayerScore);
}
