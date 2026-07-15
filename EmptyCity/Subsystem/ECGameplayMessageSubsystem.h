// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ECMessageTypes.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ECGameplayMessageSubsystem.generated.h"

class UECGameplayMessageSubsystem;

/**
 * 등록된 메시지 리스너를 식별하고 해제하기 위한 핸들입니다.
 */
USTRUCT(BlueprintType)
struct FECGameplayMessageListenerHandle
{
public:
	GENERATED_BODY()

	FECGameplayMessageListenerHandle() {}
	
	/** 등록된 리스너를 메시지 서브시스템에서 제거합니다. */
	void Unregister();

	/** 유효한 리스너 핸들인지 확인합니다. */
	bool IsValid() const { return ID != 0; }

private:
	friend class UECGameplayMessageSubsystem;

	TWeakObjectPtr<UECGameplayMessageSubsystem> Subsystem;
	FGameplayTag Channel;
	int32 ID = 0;

	friend UECGameplayMessageSubsystem;

	FECGameplayMessageListenerHandle(UECGameplayMessageSubsystem* InSubsystem, FGameplayTag InChannel, int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID) {}
};

/**
 * 리스너 하나의 정보
 */
USTRUCT()
struct FGameplayMessageListenerData
{
	GENERATED_BODY()
	
	/** 메시지를 받았을 때 실행할 함수 */
	TFunction<void(FGameplayTag, const void*)> ReceivedCallback;
	
	/** 리스너가 기대하는 메시지 구조체 타입 */
	TWeakObjectPtr<const UScriptStruct> ListenerStructType = nullptr;

	int32 HandleID;
};

UCLASS()
class EMPTYCITY_API UECGameplayMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** WorldContextObject가 속한 메시지 서브시스템을 반환하는 헬퍼 함수입니다. */
	static UECGameplayMessageSubsystem& Get(const UObject* WorldContextObject);

	/** 지정한 채널로 메시지를 전달합니다. */
	template <typename TMessage>
	void BroadcastMessage(FGameplayTag Channel,	const TMessage& Message)
	{
		const UScriptStruct* StructType = TBaseStructure<TMessage>::Get();

		BroadcastMessageInternal(Channel, StructType, &Message);
	}

	/** 지정한 메시지 채널에 객체의 멤버 함수를 등록합니다. */
	template <typename FMessageStructType, typename TOwner = UObject>
	FECGameplayMessageListenerHandle RegisterListener(FGameplayTag Channel, TOwner* Object, void (TOwner::*Function)(FGameplayTag, const FMessageStructType&))
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);

		auto Callback = [WeakObject, Function](FGameplayTag ActualChannel, const void* MessageData)
			{
				TOwner* StrongObject = WeakObject.Get();

				if (!StrongObject || !MessageData)
				{
					return;
				}

				const FMessageStructType* TypedMessage = static_cast<const FMessageStructType*>(MessageData);

				(StrongObject->*Function)(ActualChannel, *TypedMessage);
			};

		const UScriptStruct* StructType = TBaseStructure<FMessageStructType>::Get();
		
		return RegisterListenerInternal(Channel, StructType, MoveTemp(Callback));
	}

	void UnregisterListener(FECGameplayMessageListenerHandle Handle);

private:
	/**
	 * 하나의 태그를 구독하는 리스너 목록
	 */
	struct FChannelListenerList
	{
		TArray<FGameplayMessageListenerData> Listeners;
		int32 HandleID = 0;
	};

private:
	void BroadcastMessageInternal(
		FGameplayTag Channel,
		const UScriptStruct* StructType,
		const void* MessageData);

	FECGameplayMessageListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		const UScriptStruct* StructType,
		TFunction<void(FGameplayTag, const void*)>&& Callback);

	void UnregisterListenerInternal(FGameplayTag Channel, int32 HandleID);

private:
	/** 태그별 리스너 목록을 관리합니다. */
	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};