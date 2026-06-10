#include "ECAbilitySystemComponent.h"
#include "ECLogChannels.h"
#include "Ability/ECGameplayAbility.h"

UECAbilitySystemComponent::UECAbilitySystemComponent()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
	
	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

void UECAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);
	
	if (UECGameplayAbility* LyraAbility = Cast<UECGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(LyraAbility->ActivationGroup, LyraAbility);
	}
}

void UECAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);
	
	if (UECGameplayAbility* LyraAbility = Cast<UECGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(LyraAbility->ActivationGroup, LyraAbility);
	}
}

void UECAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// 부여된 모든 Ability 중 입력 Tag가 일치하는 것을 "눌림"과 "유지" 목록에 등록
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);	// 이번 프레임에 눌림
				InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);		// 떼기 전까지 계속 유지
			}
		}
	}
}

void UECAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		// 부여된 모든 Ability 중 입력 Tag가 일치하는 것을 "떼짐" 목록으로 이동
		for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
		{
			if (AbilitySpec.Ability && (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag)))
			{
				InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);	// 이번 프레임에 떼짐
				InputHeldSpecHandles.Remove(AbilitySpec.Handle);		// 더 이상 유지 아님
			}
		}
	}
}

void UECAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// 이번 프레임에 발동을 시도할 Ability 모음
	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;

	// 1) 유지 중인 입력: WhileInputActive 정책 Ability는 눌려 있는 동안 계속 발동시킨다.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				const UECGameplayAbility* AbilityCDO = CastChecked<UECGameplayAbility>(AbilitySpec->Ability);

				if (AbilityCDO->ActivationPolicy == EAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// 2) 눌린 입력: 이미 활성화된 Ability에는 "눌림"을 전달하고,
	//    아직 비활성인 OnInputTriggered 정책 Ability는 발동 대상으로 추가한다.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;
				
				if (AbilitySpec->IsActive())
				{
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					const UECGameplayAbility* AbilityCDO = CastChecked<UECGameplayAbility>(AbilitySpec->Ability);

					if (AbilityCDO->ActivationPolicy == EAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}
	
	// 3) 위에서 모은 Ability들을 실제로 발동 시도
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// 4) 떼진 입력: 활성화된 Ability에 "떼짐"을 전달 (예: 차징 종료, 발사 등)
	//    WhileInputActive 정책 Ability는 입력이 떼지면 자동으로 종료시킨다.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;
				if (AbilitySpec->IsActive())
				{
					const UECGameplayAbility* AbilityCDO = CastChecked<UECGameplayAbility>(AbilitySpec->Ability);
					if (AbilityCDO->ActivationPolicy == EAbilityActivationPolicy::WhileInputActive)
					{
						CancelAbilityHandle(AbilitySpec->Handle);
					}
					else
					{
						AbilitySpecInputReleased(*AbilitySpec);
					}
				}
			}
		}
	}

	// 눌림/떼짐은 1프레임용 신호이므로 처리 후 비운다. (유지 목록은 그대로 둔다)
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

bool UECAbilitySystemComponent::IsActivationGroupBlocked(EAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
		// Independent는 다른 어빌리티 발동 여부와 관계없이 발동 가능합니다.
		case EAbilityActivationGroup::Independent:
			bBlocked = false;
			break;

		// Exclusive는 다른 어빌리티가 발동 중인 경우 발동할 수 없습니다.
		case EAbilityActivationGroup::Exclusive_Replaceable:
		case EAbilityActivationGroup::Exclusive_Blocking:
			bBlocked = (ActivationGroupCounts[(uint8)EAbilityActivationGroup::Exclusive_Blocking] > 0);
			break;

		default:
			break;
	}

	return bBlocked;
}

void UECAbilitySystemComponent::AddAbilityToActivationGroup(EAbilityActivationGroup Group, UECGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;
	
	switch (Group)
	{
		// Independent는 다른 어빌리티를 취소하지 않습니다.
		case EAbilityActivationGroup::Independent:
			break;

		// Exclusive는 발동 중인 다른 어빌리티를 취소합니다.
		case EAbilityActivationGroup::Exclusive_Replaceable:
		case EAbilityActivationGroup::Exclusive_Blocking:
			CancelActivationGroupAbilities(EAbilityActivationGroup::Exclusive_Replaceable, Ability);
			break;

		default:
			break;
	}
}

void UECAbilitySystemComponent::RemoveAbilityFromActivationGroup(EAbilityActivationGroup Group, UECGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UECAbilitySystemComponent::CancelActivationGroupAbilities(EAbilityActivationGroup Group, UECGameplayAbility* IgnoreAbility)
{
	// "ActivationGroup이 Group과 일치하고, IgnoreAbility가 아닌 어빌리티"를 취소 대상으로 판단하는 정책 함수.
	// IgnoreAbility를 제외하는 이유: 새로 진입하는 어빌리티 자신이 같은 Group에 이미 등록되어 있을 수 있으므로, 자기 자신은 취소되지 않아야 합니다.
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UECGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		return ((Ability->ActivationGroup == Group) && (Ability != IgnoreAbility));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc);
}

void UECAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc)
{
	// 순회 중 어빌리티 cancel로 ActivatableAbilities가 재정렬/변경되는 것을 막기 위해 락을 건다.
	ABILITYLIST_SCOPE_LOCK();

	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		// 활성 상태가 아닌 spec은 취소할 대상이 없으므로 건너뛴다.
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		// 같은 spec에서 여러 인스턴스가 존재할 수 있으므로 (InstancedPerExecution 등) 각각 검사한다.
		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UECGameplayAbility* ECAbilityInstance = CastChecked<UECGameplayAbility>(AbilityInstance);

			// 호출자가 전달한 정책 함수로 이 인스턴스를 취소할지 결정한다.
			if (ShouldCancelFunc(ECAbilityInstance, AbilitySpec.Handle))
			{
				if (ECAbilityInstance->CanBeCanceled())
				{
					ECAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), ECAbilityInstance->GetCurrentActivationInfo(), false);
				}
				else
				{
					// 취소 불가 어빌리티(예: 끊기면 안 되는 연출, 보호된 상태)는 강제 종료하지 않고 로그만 남긴다.
					LOG_ABILITY(TEXT("[%s] 어빌리티를 취소할 수 없습니다. CanBeCanceled가 false입니다."), *ECAbilityInstance->GetName());
				}
			}
		}
	}
}
