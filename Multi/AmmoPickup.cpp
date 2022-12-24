// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "AmmoPickup.h"
#include "MultiCharacter.h"
#include <Components/SphereComponent.h>
#include <Components/TextRenderComponent.h>

// Sets default values
AAmmoPickup::AAmmoPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	TextComp = CreateDefaultSubobject<UTextRenderComponent>("TextComp");

	SetRootComponent(SphereComp);
	SphereComp->SetSphereRadius(25.0f);
	TextComp->SetupAttachment(RootComponent);

	FString AmmoString = FString::Printf(TEXT("Ammo"));
	TextComp->SetText(FText::FromString(AmmoString));
	TextComp->SetHorizontalAlignment(EHTA_Center);
	TextComp->SetVerticalAlignment(EVRTA_TextCenter);
	TextComp->SetTextRenderColor(FColor::Green);
}

// Called when the game starts or when spawned
void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAmmoPickup::HandleOnBeginOverlap);
	}
}

void AAmmoPickup::HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != nullptr) && (OtherActor->IsA<AMultiCharacter>()))
	{
		if (auto Char = Cast<AMultiCharacter>(OtherActor))
		{
			Char->AddAmmo(AmmoAmount);
			Destroy();
		}
	}
}

