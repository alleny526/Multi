// Copyright 2022 Hanfei Yang - alleny@usc.edu


#include "MultiOnlineSystem.h"
#include "OnlineSessionSettings.h"
#include <Kismet/GameplayStatics.h>
#include <Interfaces/OnlinePresenceInterface.h>
#include <Interfaces/OnlineIdentityInterface.h>

void UMultiOnlineSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Get the OnlineSubsystem
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	// Get the session interface, if it exists
	SessionInterface = OSS ? OSS->GetSessionInterface() : nullptr;
	if (SessionInterface)
	{
		// We got a session interface, do something with it!
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiOnlineSystem::OnCreateSessionComplete);
		SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &UMultiOnlineSystem::OnStartSessionComplete);
		SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &UMultiOnlineSystem::OnEndSessionComplete);
		SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UMultiOnlineSystem::OnSessionUserInviteAccepted);
		SessionInterface->OnFindFriendSessionCompleteDelegates[0].AddUObject(this, &UMultiOnlineSystem::OnFindFriendSessionComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiOnlineSystem::OnJoinSessionComplete);
		UE_LOG(LogTemp, Display, TEXT("In the if SessionInterface, Read Friends List..."));
	}

	FriendsInterface = OSS ? OSS->GetFriendsInterface() : nullptr;
	if (FriendsInterface)
	{
		// We got a friends interface, do something with it!
		ReadFriendsList();
	}

	SetPresenceJoinable(false);
}

void UMultiOnlineSystem::Deinitialize()
{
	if (SessionInterface)
	{
		// Do any cleanup as needed!
		SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnStartSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnEndSessionCompleteDelegates.RemoveAll(this);
		SessionInterface->OnSessionUserInviteAcceptedDelegates.RemoveAll(this);
		SessionInterface->OnFindFriendSessionCompleteDelegates[0].RemoveAll(this);
		SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
	}

	// We no longer need to hold a strong reference to the interfaces
	SessionInterface = nullptr;
	FriendsInterface = nullptr;
}

void UMultiOnlineSystem::HostSession()
{
	FOnlineSessionSettings Settings;

	// Whether this match is on a LAN or not
	Settings.bIsLANMatch = false;
	// Allow up to 4 players
	Settings.NumPublicConnections = 4;
	// Make this match advertised
	Settings.bShouldAdvertise = true;
	// This will announce the server using Steam's "presence"
	Settings.bUsesPresence = true;
	// Allow users to join if the game is already in-progress
	Settings.bAllowJoinInProgress = true;
	// Allow users to join via Steam presence
	Settings.bAllowJoinViaPresence = true;
	// Don't require only friends to join from presence
	Settings.bAllowJoinViaPresenceFriendsOnly = false;
	// Allow game invites
	Settings.bAllowInvites = true;
	// Use lobbies
	Settings.bUseLobbiesIfAvailable = true;

	// Specify the map name as your USC username (so you won't clash with others)
	Settings.Set(SETTING_MAPNAME, FString(TEXT("alleny")), EOnlineDataAdvertisementType::ViaOnlineService);

	// Create the session - use the default "GameSession"
	SessionInterface->CreateSession(0, "GameSession", Settings);
}

void UMultiOnlineSystem::FindAndJoinSession()
{
	JoinFriendSession(0);
}

void UMultiOnlineSystem::EndSession()
{
}

void UMultiOnlineSystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SessionInterface->StartSession("GameSession");
	}
}

void UMultiOnlineSystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UGameplayStatics::OpenLevel(GetWorld(), "FirstPersonMap", true, "listen");
	SetPresenceJoinable(true);
}

void UMultiOnlineSystem::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiOnlineSystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
}

void UMultiOnlineSystem::OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult)
{
	if (bWasSuccessful)
	{
		SessionInterface->JoinSession(0, "GameSession", SearchResult[0]);
	}
}

void UMultiOnlineSystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface && Result == EOnJoinSessionCompleteResult::Success)
	{
		// When we join a session, we need to get the correct travel URL from the session interface
		FString URL;
		if (SessionInterface->GetResolvedConnectString("GameSession", URL))
		{
			// Now use ClientTravel to tell our local PlayerController to join the match
			auto PC = UGameplayStatics::GetPlayerController(GetGameInstance(), 0);
			PC->ClientTravel(URL, ETravelType::TRAVEL_Absolute);
		}
	}

	SetPresenceJoinable(true);
}

void UMultiOnlineSystem::ReadFriendsList(const FOnReadFriendsListComplete& Delegate)
{
	// Get the list of friends from Steam who're playing this game and are in a session
	FriendsInterface->ReadFriendsList(0, TEXT("inGameAndSessionPlayers"), Delegate);
}

TArray<FString> UMultiOnlineSystem::GetFriendsListNames()
{
	TArray<FString> FriendsName;

	TArray<TSharedRef<FOnlineFriend>> FriendsList;
	FriendsInterface->GetFriendsList(0, TEXT("inGameAndSessionPlayers"), FriendsList);

	for (auto& CurrentFriend : FriendsList)
	{
		FriendsName.Add(CurrentFriend->GetDisplayName());
		FString Name = CurrentFriend->GetDisplayName();
		UE_LOG(LogTemp, Display, TEXT("Displaying Player's name: %s"), *Name);
	}

	return FriendsName;
}

void UMultiOnlineSystem::JoinFriendSession(int FriendNum)
{
	if (FriendsInterface && SessionInterface)
	{
		// Get the list of friends who are in-game and in progress
		// This will return immediately since we already did ReadFriendsList
		TArray<TSharedRef<FOnlineFriend>> FriendsList;
		FriendsInterface->GetFriendsList(0, TEXT("inGameAndSessionPlayers"), FriendsList);
		if (FriendsList.Num() > 0)
		{
			if (FriendsList.IsValidIndex(FriendNum))
			{
				SessionInterface->FindFriendSession(0, FriendsList[FriendNum].Get().GetUserId().Get());
			}
		}
	}
}

void UMultiOnlineSystem::SetPresenceJoinable(bool bIsJoinable)
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	// Query our presence and set the game to not joinable
	IOnlinePresencePtr PresenceInterface = OSS ? OSS->GetPresenceInterface() : nullptr;
	IOnlineIdentityPtr IdentityInterface = OSS ? OSS->GetIdentityInterface() : nullptr;
	if (PresenceInterface && IdentityInterface)
	{
		// Get the UniqueNetID for this player
		auto UserId = IdentityInterface->GetUniquePlayerId(0);
		// Query the presence (this will return instantly for the local player on Steam)
		PresenceInterface->QueryPresence(*UserId);

		// Now get the cached presence
		TSharedPtr<FOnlineUserPresence> Presence;
		if (PresenceInterface->GetCachedPresence(*UserId, Presence) == EOnlineCachedResult::Success)
		{
			// Set our new presence to the cached presence and update whether we're joinable
			FOnlineUserPresenceStatus Status = Presence->Status;
			Status.Properties.Add(TEXT("Joinable"), bIsJoinable);
			PresenceInterface->SetPresence(*UserId, Status);
		}
	}
}