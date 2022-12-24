// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatMessageWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UChatMessageWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Channel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Sender;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Message;
};
