// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "FriendButtonWidget.h"
#include "MultiOnlineSystem.h"
#include "MultiSaveSystem.h"
#include "Components/Spacer.h"

void UMainMenuWidget::RefreshButtonOnClicked()
{
	UE_LOG(LogTemp, Display, TEXT("RefreshButtonOnClicked..."));
	FriendBox->ClearChildren();
	UMultiOnlineSystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiOnlineSystem>();
	if (Subsystem)
	{
		FOnReadFriendsListComplete OnReadFriendsListCompleteDelegate = FOnReadFriendsListComplete::CreateUObject(this, &UMainMenuWidget::OnReadFriendsListComplete);
		Subsystem->ReadFriendsList(OnReadFriendsListCompleteDelegate);
	}
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshButton->OnClicked.AddDynamic(this, &UMainMenuWidget::RefreshButtonOnClicked);
	RefreshButtonOnClicked();

	UMultiSaveSystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
	if (SaveSubsystem)
	{
		if (!SaveSubsystem->SaveFileExists())
		{
			Spacer_Continue->SetVisibility(ESlateVisibility::Collapsed);
			HostButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UMainMenuWidget::NativeDestruct()
{
	Super::NativeDestruct();

	RefreshButton->OnClicked.RemoveDynamic(this, &UMainMenuWidget::RefreshButtonOnClicked);
}

void UMainMenuWidget::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Display, TEXT("OnReadFriendsListComplete..."));
		UMultiOnlineSystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiOnlineSystem>();

		int Index = 0;
		if (Subsystem)
		{
			UE_LOG(LogTemp, Display, TEXT("OnReadFriendsComplete Before Display Join Buttons..."));
			TArray<FString> FriendsNames = Subsystem->GetFriendsListNames();
			for (auto& FriendName : FriendsNames)
			{
				if (FriendButtonClass)
				{
					UFriendButtonWidget* FriendButton = NewObject<UFriendButtonWidget>(this, FriendButtonClass);
					FriendBox->AddChildToVerticalBox(FriendButton);

					UE_LOG(LogTemp, Display, TEXT("OnReadFriendsComplete Creating Join Buttons..."));

					FriendButton->FriendName->SetText(FText::FromString(FriendName));
					FriendButton->FriendIndex = Index;
					Index++;
				}
			}
		}
	}
}
