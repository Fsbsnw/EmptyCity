// Copyright Epic Games, Inc. All Rights Reserved.

#include "Subsystem/ECGameplayMessageSubsystem.h"

#include "ECMessageTypes.h"

void FECGameplayMessageListenerHandle::Unregister()
{
	if (UECGameplayMessageSubsystem* StrongSubsystem =	Subsystem.Get())
	{
		StrongSubsystem->UnregisterListener(*this);
		Subsystem.Reset();
		Channel = FGameplayTag();
		ID = 0;
	}
}

UECGameplayMessageSubsystem& UECGameplayMessageSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UECGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UECGameplayMessageSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

FECGameplayMessageListenerHandle  UECGameplayMessageSubsystem::RegisterListenerInternal(FGameplayTag Channel, const UScriptStruct* StructType, TFunction<void(FGameplayTag, const void*)>&& Callback)
{
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);

	FGameplayMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.ListenerStructType = StructType;
	Entry.HandleID = ++List.HandleID;

	return FECGameplayMessageListenerHandle(this, Channel, Entry.HandleID);
}

void UECGameplayMessageSubsystem::UnregisterListener(FECGameplayMessageListenerHandle Handle)
{
	if (Handle.IsValid())
	{
		check(Handle.Subsystem == this);

		UnregisterListenerInternal(Handle.Channel, Handle.ID);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

void UECGameplayMessageSubsystem::UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID)
{
	if (FChannelListenerList* pList = ListenerMap.Find(Channel))
	{
		int32 MatchIndex = pList->Listeners.IndexOfByPredicate([ID = HandleID](const FGameplayMessageListenerData& Other) { return Other.HandleID == ID; });
		if (MatchIndex != INDEX_NONE)
		{
			pList->Listeners.RemoveAtSwap(MatchIndex);
		}

		if (pList->Listeners.Num() == 0)
		{
			ListenerMap.Remove(Channel);
		}
	}
}

void UECGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageData)
{
	// 부모 태그로 순차적인 순회를 통해 알림 전파
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			TArray<FGameplayMessageListenerData> ListenerArray(pList->Listeners);
			
			for (const FGameplayMessageListenerData& Listener : ListenerArray)
			{
				// 처음 호출된 태그는 무조건 실행
				if (bOnInitialTag)
				{
					if (!Listener.ListenerStructType.IsValid())
					{
						UE_LOG(LogTemp, Warning, TEXT("Listener struct type has gone invalid on Channel %s. Removing listener from list"), *Tag.ToString());
						UnregisterListenerInternal(Tag, Listener.HandleID);
						continue;
					}
					
					if (StructType->IsChildOf(Listener.ListenerStructType.Get()))
					{
						Listener.ReceivedCallback(Tag, MessageData);
					}
					else
					{
						UE_LOG(LogTemp, Warning,
							TEXT("Message type mismatch. Channel: %s, "	"Broadcast: %s, Listener: %s"),
							*Channel.ToString(),
							*StructType->GetPathName(),
							*Listener.ListenerStructType->GetPathName()
						);
					}
				}
			}
		}
	}
}