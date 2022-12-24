// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class USpacer> Spacer_Continue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> RefreshButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> FriendBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UFriendButtonWidget> FriendButtonClass;

	UFUNCTION()
	void RefreshButtonOnClicked();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);
};
