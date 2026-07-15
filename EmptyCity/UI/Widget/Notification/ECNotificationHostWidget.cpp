// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/Notification/ECNotificationHostWidget.h"

#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECMessageTypes.h"

void UECNotificationHostWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// MessageSubsystem에 알림을 구독할 채널을 등록합니다.
	UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);
	ListenerHandle = MessageSubsystem.RegisterListener(TAG_AddNotification_Message, this, &ThisClass::OnNotification);
}

void UECNotificationHostWidget::NativeDestruct()
{
	UECGameplayMessageSubsystem& MessageSubsystem = UECGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(ListenerHandle);
	
	Super::NativeDestruct();
}

void UECNotificationHostWidget::OnNotification(FGameplayTag Channel, const FECNotificationMessage& Notification)
{	
	if (Notification.TargetChannel == NotificationType)
	{
		// 플레이어의 수신 대상이 일치하지 않는 경우에 종료합니다.
		if (Notification.TargetPlayer != nullptr)
		{
			APlayerController* PC = GetOwningPlayer();
			if ((PC == nullptr) || (PC->PlayerState != Notification.TargetPlayer))
			{
				return;
			}
		}

		// PayloadTag를 이용하여 알림 표시 정보를 찾습니다.
		const FName RowName = Notification.PayloadTag.GetTagName();
		
		FNotificationEntry Entry;
		
		const FNotificationDefinitionRow* FoundRow =
			NotificationDefinitionTable->FindRow<FNotificationDefinitionRow>(RowName,TEXT(""));

		if (FoundRow)
		{
			Entry.Row = *FoundRow;
			Entry.Icon = FoundRow->Icon.LoadSynchronous();
			Entry.Sound = FoundRow->Sound.LoadSynchronous();
		}
		else
		{
			return;
		}

		Entry.Message = Notification.PayloadMessage;
		Entry.Value = Notification.PayloadValue;
		Entry.ItemBrush = Notification.PayloadItemBrush;
		
		/** 실제 위젯은 블루프린트에 미리 추가된 상태이며,
		 * CreateNotificationWidget 함수를 통해 위젯의 정보를 설정합니다. */
		Entry.AllocatedWidget = CreateNotificationWidget(Entry);

		UUserWidget* CreatedWidget = Entry.AllocatedWidget;

		ActiveEntries.Add(MoveTemp(Entry));

		FTimerHandle RemoveTimerHandle;

		// FoundRow->DisplayDuration 시간 경과 후에 Entry 객체를 ActiveEntries 관리에서 제거합니다.
		GetWorld()->GetTimerManager().SetTimer(
			RemoveTimerHandle,
			FTimerDelegate::CreateWeakLambda(
				this,
				[this, CreatedWidget]()
				{
					const int32 EntryIndex = ActiveEntries.IndexOfByPredicate(
						[CreatedWidget](const FNotificationEntry& ActiveEntry)
						{
							return ActiveEntry.AllocatedWidget == CreatedWidget;
						});

					if (EntryIndex == INDEX_NONE)
					{
						return;
					}

					if (ActiveEntries[EntryIndex].AllocatedWidget)
					{
						DestroyNotificationWidget(
							ActiveEntries[EntryIndex].AllocatedWidget
						);
					}

					ActiveEntries.RemoveAt(EntryIndex);
				}),
			FoundRow->DisplayDuration,
			false
		);
	}
}