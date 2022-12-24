// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MultiPlayerController.h"
#include "MultiCharacter.h"
#include "HUDWidget.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "TagActor.h"
#include "EngineUtils.h"
#include "Components/Image.h"
#include "MultiSaveSystem.h"
#include "MultiSaveGame.h"

void AMultiPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind jump events
	InputComponent->BindAction("Jump", IE_Pressed, this, &AMultiPlayerController::OnJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AMultiPlayerController::OnStopJumping);

	// Bind fire event
	InputComponent->BindAction("PrimaryAction", IE_Pressed, this, &AMultiPlayerController::OnPrimaryAction);

	InputComponent->BindAction("TeamChat", IE_Pressed, this, &AMultiPlayerController::OnTeamChat);
	InputComponent->BindAction("AllChat", IE_Pressed, this, &AMultiPlayerController::OnAllChat);

	InputComponent->BindAction("ApplyTag", IE_Pressed, this, &AMultiPlayerController::OnApplyTag);

	InputComponent->BindAction("Tag1", IE_Pressed, this, &AMultiPlayerController::OnTag1);
	InputComponent->BindAction("Tag2", IE_Pressed, this, &AMultiPlayerController::OnTag2);
	InputComponent->BindAction("Tag3", IE_Pressed, this, &AMultiPlayerController::OnTag3);
	InputComponent->BindAction("Tag4", IE_Pressed, this, &AMultiPlayerController::OnTag4);
	InputComponent->BindAction("Tag5", IE_Pressed, this, &AMultiPlayerController::OnTag5);

	InputComponent->BindAction("Grenade", IE_Pressed, this, &AMultiPlayerController::OnGrenade);

	// Bind movement events
	InputComponent->BindAxis("Move Forward / Backward", this, &AMultiPlayerController::MoveForward);
	InputComponent->BindAxis("Move Right / Left", this, &AMultiPlayerController::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "Mouse" versions handle devices that provide an absolute delta, such as a mouse.
	// "Gamepad" versions are for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn Right / Left Mouse", this, &AMultiPlayerController::AddControllerYawInput);
	InputComponent->BindAxis("Look Up / Down Mouse", this, &AMultiPlayerController::AddControllerPitchInput);
	InputComponent->BindAxis("Turn Right / Left Gamepad", this, &AMultiPlayerController::TurnAtRate);
	InputComponent->BindAxis("Look Up / Down Gamepad", this, &AMultiPlayerController::LookUpAtRate);
}

void AMultiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidgetInstance = NewObject<UHUDWidget>(this, HUDWidgetClass);
		HUDWidgetInstance->AddToViewport();
		HUDWidgetInstance->SetOwningPlayer(this);

		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, HUDWidgetInstance);
	}
}

void AMultiPlayerController::SetAlert(const FString& TextToShow, FLinearColor Color, float Duration)
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->SetAlert(TextToShow, Color, Duration);
	}	
}

void AMultiPlayerController::ActivateRespawnTimer()
{
	if (HUDWidgetInstance)
	{
		HUDWidgetInstance->ActivateRespawnTimer();
	}
}

void AMultiPlayerController::OnJump()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		Char->Jump();
	}
}

void AMultiPlayerController::OnStopJumping()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		Char->StopJumping();
	}
}

void AMultiPlayerController::OnPrimaryAction()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		Char->OnPrimaryAction();
	}
}

void AMultiPlayerController::OnTeamChat()
{
	FocusText(GetPlayerState<AMultiPlayerState>()->Team);
}

void AMultiPlayerController::OnAllChat()
{
	FocusText();
}

void AMultiPlayerController::OnApplyTag()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		ServerApplyTag(CurrTagIndex);
	}
}

void AMultiPlayerController::OnTag1()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		CurrTagIndex = 0;
		HUDWidgetInstance->TagUI->SetBrushFromMaterial(HUDWidgetInstance->TagMaterials[0]);
	}
}

void AMultiPlayerController::OnTag2()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		CurrTagIndex = 1;
		HUDWidgetInstance->TagUI->SetBrushFromMaterial(HUDWidgetInstance->TagMaterials[1]);
	}
}

void AMultiPlayerController::OnTag3()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		CurrTagIndex = 2;
		HUDWidgetInstance->TagUI->SetBrushFromMaterial(HUDWidgetInstance->TagMaterials[2]);
	}
}

void AMultiPlayerController::OnTag4()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		CurrTagIndex = 3;
		HUDWidgetInstance->TagUI->SetBrushFromMaterial(HUDWidgetInstance->TagMaterials[3]);
	}
}

void AMultiPlayerController::OnTag5()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		CurrTagIndex = 4;
		HUDWidgetInstance->TagUI->SetBrushFromMaterial(HUDWidgetInstance->TagMaterials[4]);
	}
}

void AMultiPlayerController::OnGrenade()
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		Char->OnGrenade();
	}
}

void AMultiPlayerController::MoveForward(float Val)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Val != 0.0f && Char)
	{
		// add movement in that direction
		Char->AddMovementInput(Char->GetActorForwardVector(), Val);
	}
}

void AMultiPlayerController::MoveRight(float Val)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Val != 0.0f && Char)
	{
		// add movement in that direction
		Char->AddMovementInput(Char->GetActorRightVector(), Val);
	}
}

void AMultiPlayerController::AddControllerYawInput(float Val)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Val != 0.0f && Char && Char->IsAlive())
	{
		Char->AddControllerYawInput(Val);
	}
}

void AMultiPlayerController::AddControllerPitchInput(float Val)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Val != 0.0f && Char && Char->IsAlive())
	{
		Char->AddControllerPitchInput(Val);
	}
}

void AMultiPlayerController::TurnAtRate(float Val)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char && Char->IsAlive())
	{
		// calculate delta for this frame from the rate information
		Char->AddControllerYawInput(Val * Char->TurnRateGamepad * GetWorld()->GetDeltaSeconds());
	}	
}

void AMultiPlayerController::LookUpAtRate(float Val)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char && Char->IsAlive())
	{
		// calculate delta for this frame from the rate information
		AddControllerPitchInput(Val * Char->TurnRateGamepad * GetWorld()->GetDeltaSeconds());
	}	
}

void AMultiPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (PlayerState)
	{
		if (AMultiCharacter* Char = GetPawn<AMultiCharacter>())
		{
			Char->SetTeam(Cast<AMultiPlayerState>(PlayerState)->GetTeam());
		}
	}
	
	ClientOnPossess();
}

void AMultiPlayerController::FocusText(EMultiTeam Team)
{
	HUDWidgetInstance->ChatEntry->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, HUDWidgetInstance);

	if (Team == EMultiTeam::Red)
	{
		HUDWidgetInstance->ChatTeam->SetText(FText::FromString(FString::Printf(TEXT("[Red]"))));
		HUDWidgetInstance->ChatTeam->SetColorAndOpacity(FLinearColor::Red);
	}
	else if (Team == EMultiTeam::Blue)
	{
		HUDWidgetInstance->ChatTeam->SetText(FText::FromString(FString::Printf(TEXT("[Blue]"))));
		HUDWidgetInstance->ChatTeam->SetColorAndOpacity(FLinearColor::Blue);
	}
	else
	{
		HUDWidgetInstance->ChatTeam->SetText(FText::FromString(FString::Printf(TEXT("[All]"))));
		HUDWidgetInstance->ChatTeam->SetColorAndOpacity(FLinearColor::White);
	}

	HUDWidgetInstance->ChatTextBox->SetFocus();
}

void AMultiPlayerController::ServerApplyTag_Implementation(int TagIndexCurr)
{
	AMultiCharacter* Char = GetPawn<AMultiCharacter>();
	if (Char)
	{
		FHitResult HitResult;
		FVector StartPoint = Char->GetActorLocation();
		FVector EndPoint = StartPoint + ControlRotation.Vector() * 500.0f;
		FCollisionQueryParams QueryParams;

		// Ignore all characters in the world
		for (TActorIterator<ACharacter> Iter(GetWorld()); Iter; ++Iter)
		{
			QueryParams.AddIgnoredActor(*Iter);
		}
		QueryParams.AddIgnoredActor(Char);
		GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, ECC_Camera, QueryParams);

		if (HitResult.bBlockingHit)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ATagActor* TagActor = GetWorld()->SpawnActor<ATagActor>(TagActorClass[TagIndexCurr], HitResult.Location, FRotator::ZeroRotator, SpawnParameters);	
			
			if (TagActor)
			{
				FVector Axis = FVector::CrossProduct(FVector::XAxisVector, -HitResult.Normal);
				Axis.Normalize();
				float Dot = FVector::DotProduct(FVector::XAxisVector, -HitResult.Normal);
				float Angle = FMath::Acos(Dot);
				FQuat SpawnQuat(Axis, Angle);

				if (Dot < -0.99f)
				{
					SpawnQuat = FQuat(FVector::ZAxisVector, PI);
				}
				FRotator SpawnRotator(SpawnQuat);

				// If we're on the ground, yaw based on the player control rotation yaw
				if (SpawnRotator.Pitch < -45.0f)
				{
					SpawnRotator.Yaw = ControlRotation.Yaw - 90.0f;
				}
				else
				{
					SpawnRotator.Roll += 90.0f;
				}

				TagActor->SetActorRotation(SpawnRotator);

				// Set the Tag Save
				UMultiSaveSystem* SaveSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMultiSaveSystem>();
				if (SaveSubsystem)
				{
					SaveSubsystem->GroundTruth->AddToTagSave(TagActor->GetTransform(), TagActorClass[TagIndexCurr]);
					SaveSubsystem->SaveGame();
				}
			}
		}
	}
}

void AMultiPlayerController::ClientOnPossess_Implementation()
{
	UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
}
