// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "FriendButtonWidget.h"
#include "Components/Button.h"
#include "MultiOnlineSystem.h"

void UFriendButtonWidget::JoinButtonOnClicked()
{
	UMultiOnlineSystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiOnlineSystem>();
	if (Subsystem)
	{
		Subsystem->JoinFriendSession(FriendIndex);
	}
}

void UFriendButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	JoinButton->OnClicked.AddDynamic(this, &UFriendButtonWidget::JoinButtonOnClicked);
}

void UFriendButtonWidget::NativeDestruct()
{
	Super::NativeConstruct();

	JoinButton->OnClicked.RemoveDynamic(this, &UFriendButtonWidget::JoinButtonOnClicked);
}
