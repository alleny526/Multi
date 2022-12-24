// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MultiSaveSystem.h"
#include "MultiSaveGame.h"
#include <Kismet/GameplayStatics.h>
#include "TagActor.h"
#include "MultiPlayerController.h"
#include "HUDWidget.h"
#include "Components/CircularThrobber.h"

void UMultiSaveSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GroundTruth = NewObject<UMultiSaveGame>();
}

void UMultiSaveSystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMultiSaveSystem::SaveGame()
{
	if (bIsAsyncSave)
	{
		if (GetWorld()->GetNetMode() == NM_ListenServer)
		{
			AMultiPlayerController* Controller = Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
			if (Controller)
			{
				Controller->HUDWidgetInstance->AutoSaveThrobber->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}

		UGameplayStatics::AsyncSaveGameToSlot(GroundTruth, FString::Printf(TEXT("SaveGame")), 0);

		if (GetWorld()->GetNetMode() == NM_ListenServer)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UMultiSaveSystem::HideHUDAutoSaveThrobber, 0.25f);
		}
	}
	else
	{
		UGameplayStatics::SaveGameToSlot(GroundTruth, FString::Printf(TEXT("SaveGame")), 0);
	}
}

void UMultiSaveSystem::LoadGame()
{
	if (UGameplayStatics::LoadGameFromSlot(FString::Printf(TEXT("SaveGame")), 0) == nullptr)
	{
		GroundTruth = NewObject<UMultiSaveGame>();
	}
	else
	{
		GroundTruth = Cast<UMultiSaveGame>(UGameplayStatics::LoadGameFromSlot(FString::Printf(TEXT("SaveGame")), 0));

		for (auto& TagData : GroundTruth->TagActors)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ATagActor* TagActor = GetWorld()->SpawnActor<ATagActor>(TagData.ClassType, TagData.Transform.GetLocation(), FRotator::ZeroRotator, SpawnParameters);
			TagActor->SetActorRotation(TagData.Transform.GetRotation());
			TagActor->SetActorScale3D(TagData.Transform.GetScale3D());
		}
	}
}

void UMultiSaveSystem::HideHUDAutoSaveThrobber()
{
	AMultiPlayerController* Controller = Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (Controller)
	{
		Controller->HUDWidgetInstance->AutoSaveThrobber->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMultiSaveSystem::CreateNewSave()
{
	GroundTruth = NewObject<UMultiSaveGame>();
	UGameplayStatics::SaveGameToSlot(GroundTruth, FString::Printf(TEXT("SaveGame")), 0);
}

bool UMultiSaveSystem::SaveFileExists()
{
	return UGameplayStatics::DoesSaveGameExist(FString::Printf(TEXT("SaveGame")), 0);
}
