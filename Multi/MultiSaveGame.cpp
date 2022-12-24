// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MultiSaveGame.h"
#include "TagActor.h"

void UMultiSaveGame::AddToTagSave(FTransform CurrTransform, TSubclassOf<class ATagActor> CurrClassType)
{
	FTagActorSaveData Save;
	Save.Transform = CurrTransform;
	Save.ClassType = CurrClassType;

	TagActors.Add(Save);
}
