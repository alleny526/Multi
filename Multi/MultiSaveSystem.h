// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiSaveSystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UMultiSaveSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection);

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize();

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UMultiSaveGame> GroundTruth;

	void SaveGame();
	void LoadGame();

	void HideHUDAutoSaveThrobber();

	UFUNCTION(BlueprintCallable)
	void CreateNewSave();

	bool SaveFileExists();

	bool bIsAsyncSave = false;
};
