// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiGameMode.h"
#include "MultiCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "MultiPlayerState.h"
#include "MultiGameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include <Kismet/GameplayStatics.h>
#include "MultiPlayerController.h"
#include "MultiSaveGame.h"
#include "MultiSaveSystem.h"

AMultiGameMode::AMultiGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	PlayerStateClass = AMultiPlayerState::StaticClass();
	GameStateClass = AMultiGameStateBase::StaticClass();

	bDelayedStart = true;
}

void AMultiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UMultiSaveSystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (SaveSubsystem)
	{
		SaveSubsystem->bIsAsyncSave = false;
		SaveSubsystem->SaveGame();
	}
}

void AMultiGameMode::Tick(float DeltaSeconds)
{
	if (IsMatchInProgress())
	{
		AutoSaveTimer -= DeltaSeconds;
		if (AutoSaveTimer <= 0.0f)
		{
			AutoSaveTimer = 5.0f;

			UMultiSaveSystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
			if (SaveSubsystem)
			{
				SaveSubsystem->bIsAsyncSave = true;
				SaveSubsystem->SaveGame();
			}
		}
	}
}

bool AMultiGameMode::ReadyToEndMatch_Implementation()
{
	if (GetGameState<AMultiGameStateBase>()->GetRedScore() == 10 || GetGameState<AMultiGameStateBase>()->GetBlueScore() == 10)
	{
		return true;
	}
	return false;
}

void AMultiGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	if (GetGameState<AMultiGameStateBase>()->GetRedScore() == 10)
	{
		FString AlertString = FString::Printf(TEXT("RED TEAM WINS"));
		GetGameState<AMultiGameStateBase>()->MulticastSendAlert(AlertString, FLinearColor::Red, 5.0f);

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AMultiGameMode::RestartGame, 5.0f);
	}
	else if (GetGameState<AMultiGameStateBase>()->GetBlueScore() == 10)
	{
		FString AlertString = FString::Printf(TEXT("BLUE TEAM WINS"));
		GetGameState<AMultiGameStateBase>()->MulticastSendAlert(AlertString, FLinearColor::Blue, 5.0f);

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AMultiGameMode::RestartGame, 5.0f);
	}

	UMultiSaveSystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (SaveSubsystem)
	{
		SaveSubsystem->bIsAsyncSave = false;
		SaveSubsystem->SaveGame();
	}
}

void AMultiGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	UMultiSaveSystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (SaveSubsystem)
	{
		SaveSubsystem->LoadGame();
	}
}

void AMultiGameMode::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);

	if (AMultiPlayerState* PlayerState = C->GetPlayerState<AMultiPlayerState>())
	{
		if (PlayerState->GetTeam() == EMultiTeam::None)
		{
			if (RedTeamNum <= BlueTeamNum)
			{
				PlayerState->SetRedTeam();
				RedTeamNum++;
			}
			else
			{
				PlayerState->SetBlueTeam();
				BlueTeamNum++;
			}
		}
	}
}

AActor* AMultiGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	// Choose a player start
	APlayerStart* FoundPlayerStart = nullptr;
	UClass* PawnClass = GetDefaultPawnClassForController(Player);
	APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
	TArray<APlayerStart*> UnOccupiedStartPoints;
	TArray<APlayerStart*> OccupiedStartPoints;
	TArray<APlayerStart*> BackupStartPoints;
	UWorld* World = GetWorld();

	FName PlayerStartTag = NAME_None;
	EMultiTeam Team = Player->GetPlayerState<AMultiPlayerState>()->Team;
	if (Team == EMultiTeam::Red)
	{
		PlayerStartTag = "Red";
	}
	else if (Team == EMultiTeam::Blue) 
	{
		PlayerStartTag = "Blue";
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			FoundPlayerStart = PlayerStart;
			break;
		}
		else if (PlayerStartTag == NAME_None || PlayerStartTag == PlayerStart->PlayerStartTag)
		{
			FVector ActorLocation = PlayerStart->GetActorLocation();
			const FRotator ActorRotation = PlayerStart->GetActorRotation();
			if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
			{
				UnOccupiedStartPoints.Add(PlayerStart);
			}
			else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
			{
				OccupiedStartPoints.Add(PlayerStart);
			}
		}
		else if (PlayerStart->PlayerStartTag == "Backup")
		{
			BackupStartPoints.Add(PlayerStart);
		}
	}

	for (TActorIterator<AMultiCharacter> It(World); It; ++It)
	{
		auto MyLambda = [It](APlayerStart* PlayerStart) {
			if ((It->Team == EMultiTeam::Red && PlayerStart->PlayerStartTag == "Blue") || (It->Team == EMultiTeam::Blue && PlayerStart->PlayerStartTag == "Red"))
			{
				return ((PlayerStart->GetActorLocation() - It->GetActorLocation()).Length() <= 1000.0f);
			}
			return false;
		};

		OccupiedStartPoints.RemoveAll(MyLambda);

		UnOccupiedStartPoints.RemoveAll(MyLambda);
	}

	if (FoundPlayerStart == nullptr)
	{
		if (UnOccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
		}
		else if (OccupiedStartPoints.Num() > 0)
		{
			FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
		}
		else if (UnOccupiedStartPoints.Num() == 0 && OccupiedStartPoints.Num() == 0)
		{
			FoundPlayerStart = BackupStartPoints[FMath::RandRange(0, BackupStartPoints.Num() - 1)];
		}
	}
	return FoundPlayerStart;
}