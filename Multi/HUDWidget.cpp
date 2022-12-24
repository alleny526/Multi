// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "HUDWidget.h"
#include "MultiCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "MultiGameStateBase.h"
#include "ChatMessageWidget.h"
#include "GameFramework/GameMode.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "MultiPlayerController.h"
#include "Components/HorizontalBox.h"
#include "Components/CheckBox.h"
#include "Components/CircularThrobber.h"

void UHUDWidget::SetAlert(const FString& TextToShow, FLinearColor Color, float Duration)
{
	Alert->SetText(FText::FromString(TextToShow));
	Alert->SetColorAndOpacity(Color);
	AlertDuration = Duration;
	bIsAlertAppear = true;

	FTimerHandle TimerHandle;
	GetOwningPlayer()->GetWorldTimerManager().SetTimer(TimerHandle, this, &UHUDWidget::HideAlert, Duration);
}

void UHUDWidget::HideAlert()
{	
	bIsAlertAppear = false;
	if (Alert->GetVisibility() != ESlateVisibility::Hidden)
	{
		Alert->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHUDWidget::AddChatMessage(EMultiTeam Team, const FString& Sender, const FString& Message)
{
	if (ChatMessageWidgetClass)
	{
		ChatMessageWidgetInstance = NewObject<UChatMessageWidget>(this, ChatMessageWidgetClass);
		ChatMessages->AddChildToVerticalBox(ChatMessageWidgetInstance);
		if (Team == EMultiTeam::None)
		{
			ChatMessageWidgetInstance->Channel->SetText(FText::FromString(FString::Printf(TEXT("[All]"))));
			ChatMessageWidgetInstance->Channel->SetColorAndOpacity(FLinearColor::White);
		}
		else if (Team == EMultiTeam::Red)
		{
			ChatMessageWidgetInstance->Channel->SetText(FText::FromString(FString::Printf(TEXT("[Red]"))));
			ChatMessageWidgetInstance->Channel->SetColorAndOpacity(FLinearColor::Red);
		}
		else if (Team == EMultiTeam::Blue)
		{
			ChatMessageWidgetInstance->Channel->SetText(FText::FromString(FString::Printf(TEXT("[Blue]"))));
			ChatMessageWidgetInstance->Channel->SetColorAndOpacity(FLinearColor::Blue);
		}

		ChatMessageWidgetInstance->Sender->SetText(FText::FromString(Sender));
		ChatMessageWidgetInstance->Message->SetText(FText::FromString(Message));
		
		ChatScrollBox->ScrollToEnd();
	}
}

void UHUDWidget::SetTextChat(const FText& Text, ETextCommit::Type CommitMethod)
{
	AMultiCharacter* Char = Cast<AMultiCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	AMultiPlayerState* PlayerState = Char->GetPlayerState<AMultiPlayerState>();
	AMultiGameStateBase* GameStateBase = GetWorld()->GetGameState<AMultiGameStateBase>();

	if (CommitMethod == ETextCommit::OnEnter)
	{
		ChatTextBox->SetText(FText::GetEmpty());
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (ChatTeam->GetText().EqualTo(FText::FromString(FString::Printf(TEXT("[All]")))))
		{
			PlayerState->ServerSendMessage(EMultiTeam::None, PlayerState->GetPlayerName(), Text.ToString());
		}
		else
		{
			PlayerState->ServerSendMessage(PlayerState->Team, PlayerState->GetPlayerName(), Text.ToString());
		}

		ChatEntry->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHUDWidget::SendReadyStat(bool bStatus)
{
	AMultiPlayerState* PlayerState = Cast<AMultiPlayerController>(UGameplayStatics::GetPlayerController(this, 0))->GetPlayerState<AMultiPlayerState>();

	if (bStatus)
	{
		PlayerState->ServerSendReadyInfo(true);
	}
	else
	{
		PlayerState->ServerSendReadyInfo(false);
	}
}

void UHUDWidget::HideAutoSaveThrobber()
{
	AutoSaveThrobber->SetVisibility(ESlateVisibility::Hidden);
}

void UHUDWidget::NativeConstruct()
{	
	Super::NativeConstruct();

	ChatTextBox->OnTextCommitted.AddDynamic(this, &UHUDWidget::SetTextChat);
	ReadyCheckBox->OnCheckStateChanged.AddDynamic(this, &UHUDWidget::SendReadyStat);
}

void UHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	ChatTextBox->OnTextCommitted.RemoveDynamic(this, &UHUDWidget::SetTextChat);
	ReadyCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UHUDWidget::SendReadyStat);
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AMultiGameStateBase* GameStateBase = GetWorld()->GetGameState<AMultiGameStateBase>();

	if (GameStateBase && GameStateBase->GetMatchState() == MatchState::WaitingToStart)
	{
		ReadyCheckBox->SetVisibility(ESlateVisibility::Visible);

		if (Timer->GetVisibility() != ESlateVisibility::HitTestInvisible)
		{
			Timer->SetVisibility(ESlateVisibility::HitTestInvisible);
		}

		FString TimerText = FString::Printf(TEXT("%f"), GameStateBase->WaitingToStartTime);
		Timer->SetText(FText::FromString(TimerText));
	}
	else
	{
		ReadyCheckBox->SetVisibility(ESlateVisibility::Hidden);

		if (Timer->GetVisibility() != ESlateVisibility::Hidden)
		{
			Timer->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (bIsAlertAppear && (GameStateBase->IsMatchInProgress() || GameStateBase->GetMatchState() == MatchState::WaitingPostMatch))
	{
		Alert->SetVisibility(ESlateVisibility::Visible);
		AlertDuration -= InDeltaTime;
		if (AlertDuration <= 0.0f)
		{
			AlertDuration = 0.0f;
		}
	}

	if (bIsRespawnTimerActive && GameStateBase->IsMatchInProgress())
	{
		RespawnTimer->SetVisibility(ESlateVisibility::Visible);
		RespawnDuration -= InDeltaTime;
		FString TimerText = FString::Printf(TEXT("%f"), RespawnDuration);
		RespawnTimer->SetText(FText::FromString(TimerText));
		if (RespawnDuration <= 0.0f)
		{
			RespawnDuration = 5.0f;
			bIsRespawnTimerActive = false;
			if (RespawnTimer->GetVisibility() != ESlateVisibility::Hidden)
			{
				RespawnTimer->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	// Update stuff
	if (auto Char = Cast<AMultiCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		FString AmmoString = FString::Printf(TEXT("%d AMMO"), Char->GetAmmo());
		Ammo->SetText(FText::FromString(AmmoString));

		if (GameStateBase)
		{
			if (AMultiPlayerState* PlayerState = Char->GetPlayerState<AMultiPlayerState>())
			{
				if (Char->Team == EMultiTeam::Red)
				{
					FString RedScoreString = FString::Printf(TEXT("RED %d(%d)"), GameStateBase->GetRedScore(), PlayerState->GetScore());
					RedScore->SetText(FText::FromString(RedScoreString));

					FString BlueScoreString = FString::Printf(TEXT("%d BLUE"), GameStateBase->GetBlueScore());
					BlueScore->SetText(FText::FromString(BlueScoreString));
				}
				else
				{
					FString RedScoreString = FString::Printf(TEXT("RED %d"), GameStateBase->GetRedScore());
					RedScore->SetText(FText::FromString(RedScoreString));

					FString BlueScoreString = FString::Printf(TEXT("%d(%d) BLUE"), GameStateBase->GetBlueScore(), PlayerState->GetScore());
					BlueScore->SetText(FText::FromString(BlueScoreString));
				}
			}
		}
	}
}