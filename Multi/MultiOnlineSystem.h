// Copyright 2022 Hanfei Yang - alleny@usc.edu

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "MultiOnlineSystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTI_API UMultiOnlineSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection);
	virtual void Deinitialize();

	UFUNCTION(BlueprintCallable)
	void HostSession();

	UFUNCTION(BlueprintCallable)
	void FindAndJoinSession();

	UFUNCTION(BlueprintCallable)
	void EndSession();

	// Used to bind to FOnCreateSessionComplete delegate
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	void OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	// Reads the friends list and triggers the delegate when complete
	void ReadFriendsList(const FOnReadFriendsListComplete& Delegate = FOnReadFriendsListComplete());

	// Returns the names of the friends which were recieved on the last successful ReadFriendsList
	TArray<FString> GetFriendsListNames();

	// Try to join the friend session of FriendNum, if it's a valid index in the friends list
	void JoinFriendSession(int FriendNum);

	void SetPresenceJoinable(bool bIsJoinable);

private:
	IOnlineSessionPtr SessionInterface;
	IOnlineFriendsPtr FriendsInterface;

};
