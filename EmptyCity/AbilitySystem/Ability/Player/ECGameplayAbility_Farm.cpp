#include "AbilitySystem/Ability/Player/ECGameplayAbility_Farm.h"

#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Actor/Farming/ECFarmingResourceActor.h"
#include "Character/Player/ECPlayer.h"
#include "Character/Player/State/ECPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/Farming/ECFarmingTableRow.h"
#include "Data/Workbench/ECWorkbenchUpgradeDataAsset.h"
#include "Equipment/Weapon/ECWeaponInstance.h"

UECGameplayAbility_Farm::UECGameplayAbility_Farm()
{
	// 파밍 Ability의 인스턴스, 실행 정책 및 동시 실행 정책을 설정합니다.
	{
		InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
		ActivationPolicy = EAbilityActivationPolicy::ByGameplayEvent;
		ActivationGroup = EAbilityActivationGroup::Exclusive_Blocking;
	}
}

void UECGameplayAbility_Farm::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 상호작용 이벤트에서 파밍 대상과 파밍 데이터를 가져옵니다.
	FarmingTarget = Cast<AECFarmingResourceActor>(const_cast<AActor*>(ToRawPtr(TriggerEventData->Target)));
	const FECFarmingTableRow& FarmingData = FarmingTarget->GetFarmingData();

	AECPlayer* Player = Cast<AECPlayer>(GetAvatarActorFromActorInfo());
	EquippedWeapon = Player->GetEquippedWeapon();

	// 작업대 도구 강화 단계에 설정된 실제 파밍 소요 시간을 적용합니다.
	// 데이터가 없거나 Axe/Pickaxe 대상이 아니면 DT_Farming의 기본 시간을 사용합니다.
	float EffectiveInteractionDuration = FarmingData.InteractionDuration;
	float FarmingMontagePlayRate = 1.0f;
	const bool bUsesUpgradeableTool =
		FarmingData.ToolUpgradeType == EECWorkbenchUpgradeType::Axe ||
		FarmingData.ToolUpgradeType == EECWorkbenchUpgradeType::Pickaxe;
	if (bUsesUpgradeableTool)
	{
		if (const AECPlayerState* PlayerState = Player->GetPlayerState<AECPlayerState>())
		{
			float ConfiguredDuration = 0.0f;
			if (PlayerState->GetWorkbenchUpgradeValue(
				FarmingData.ToolUpgradeType,
				TEXT("InteractionDuration"),
				ConfiguredDuration) && ConfiguredDuration > UE_KINDA_SMALL_NUMBER)
			{
				EffectiveInteractionDuration = ConfiguredDuration;

				// 몽타주 배속은 파밍 대상의 InteractionDuration과 무관하게,
				// 해당 도구 1단계 작업 시간 대비 현재 단계 작업 시간만으로 계산합니다.
				if (const UECWorkbenchUpgradeDataAsset* UpgradeData = PlayerState->GetWorkbenchUpgradeData())
				{
					if (const FECWorkbenchUpgradeLevelData* LevelOneData =
						UpgradeData->FindLevelData(FarmingData.ToolUpgradeType, 1))
					{
						const FECWorkbenchUpgradeValue* LevelOneDuration = LevelOneData->Values.FindByPredicate(
							[](const FECWorkbenchUpgradeValue& Value)
							{
								return Value.ValueId == TEXT("InteractionDuration");
							});

						if (LevelOneDuration && LevelOneDuration->Value > UE_KINDA_SMALL_NUMBER)
						{
							FarmingMontagePlayRate = LevelOneDuration->Value / ConfiguredDuration;
						}
					}
				}
			}
		}
	}
	EffectiveInteractionDuration = FMath::Max(0.1f, EffectiveInteractionDuration);
	FarmingMontagePlayRate = FMath::Max(UE_KINDA_SMALL_NUMBER, FarmingMontagePlayRate);

	// 파밍 애니메이션은 항상 플레이어의 1인칭 Mesh에서 재생합니다.
	ActiveFarmingMesh = Player->GetFirstPersonMesh();
	ActiveFarmingMontage = FarmingData.FarmingMontage.LoadSynchronous();
	Player->SetActiveFarmingSound(FarmingData.FarmingSound);

	// 현재 방망이를 숨기고 파밍 대상에 맞는 도끼/곡괭이를 1인칭 Mesh에 표시합니다.
	if (EquippedWeapon)
	{
		for (AActor* WeaponActor : EquippedWeapon->GetSpawnedActors())
		{
			WeaponActor->SetActorHiddenInGame(true);
		}
	}
	Player->SetFirstPersonFarmingToolActive(true, FarmingData.ToolUpgradeType);

	if (ActiveFarmingMontage)
	{
		if (UAnimInstance* FarmingAnimInstance = ActiveFarmingMesh
			? ActiveFarmingMesh->GetAnimInstance()
			: nullptr)
		{
			FarmingAnimInstance->Montage_Play(ActiveFarmingMontage, FarmingMontagePlayRate);
		}
	}

	// 강화 단계에 설정된 정확한 초가 지나면 파밍을 완료합니다.
	UAbilityTask_WaitDelay* FarmingTimer = UAbilityTask_WaitDelay::WaitDelay(
		this,
		EffectiveInteractionDuration);
	FarmingTimer->OnFinish.AddDynamic(this, &ThisClass::HandleFarmingCompleted);
	FarmingTimer->ReadyForActivation();
}

void UECGameplayAbility_Farm::HandleFarmingCompleted()
{
	// 아이템을 드롭하고 파밍 대상을 영구 제거합니다.
	FarmingTarget->CompleteFarming();
	K2_EndAbility();
}

void UECGameplayAbility_Farm::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 종료와 취소 모두에서 1인칭 파밍 몽타주를 멈춥니다.
	if (ActiveFarmingMesh)
	{
		if (UAnimInstance* FarmingAnimInstance = ActiveFarmingMesh->GetAnimInstance())
		{
			FarmingAnimInstance->Montage_Stop(0.15f, ActiveFarmingMontage);
		}
	}

	if (AECPlayer* Player = Cast<AECPlayer>(GetAvatarActorFromActorInfo()))
	{
		Player->SetFirstPersonFarmingToolActive(false);
		Player->SetActiveFarmingSound(FDataTableRowHandle());
	}

	// 파밍 중 숨긴 기존 방망이를 다시 표시합니다.
	if (EquippedWeapon)
	{
		for (AActor* WeaponActor : EquippedWeapon->GetSpawnedActors())
		{
			WeaponActor->SetActorHiddenInGame(false);
		}
	}

	ActiveFarmingMontage = nullptr;
	ActiveFarmingMesh = nullptr;
	EquippedWeapon = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
