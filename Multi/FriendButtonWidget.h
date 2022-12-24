// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FriendButtonWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UFriendButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> JoinButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> FriendName;

	int FriendIndex;

	UFUNCTION()
	void JoinButtonOnClicked();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
