// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiPlayerState.h"
#include "HUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> RedScore;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> BlueScore;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Ammo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Timer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Alert;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> RespawnTimer;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> ChatBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UVerticalBox> ChatMessages;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UHorizontalBox> ChatEntry;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> ChatTeam;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox> ChatTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCheckBox> ReadyCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> TagUI;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UCircularThrobber> AutoSaveThrobber;

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UMaterialInterface>> TagMaterials;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> Grenade;

	float AlertDuration;
	void SetAlert(const FString& TextToShow, FLinearColor Color, float Duration);
	bool bIsAlertAppear = false;

	void HideAlert();

	float RespawnDuration = 5.0f;
	bool bIsRespawnTimerActive = false;
	void ActivateRespawnTimer() { bIsRespawnTimerActive = true; };

	void AddChatMessage(EMultiTeam Team, const FString& Sender, const FString& Message);

	UFUNCTION()
	virtual void SetTextChat(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	virtual void SendReadyStat(bool bStatus);
	
	void HideAutoSaveThrobber();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UChatMessageWidget> ChatMessageWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<class UChatMessageWidget> ChatMessageWidgetInstance;

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
