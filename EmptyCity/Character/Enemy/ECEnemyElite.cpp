// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/ECEnemyElite.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ECEnemyNormal.h"
#include "ECGameplayTags.h"
#include "Engine/OverlapResult.h"

void AECEnemyElite::BeginPlay()
{
	Super::BeginPlay();

	// 부하들을 캐싱합니다.
	CachingManagedMinions();
	// 부하들에게 Buff를 부여합니다.
	GrantEliteBuffToMinions();
}

void AECEnemyElite::OnDeathFinished(AActor* OwningActor)
{
	Super::OnDeathFinished(OwningActor);

	FGameplayTagContainer BuffTagContainer;
	BuffTagContainer.AddTag(ECGameplayTags::Status_Buff_EliteEmpowered);
	
	for (AECEnemyNormal* Minion : ManagedMinions)
	{
		// 1. 부하 액터가 아직 월드에 존재하는지 검사합니다.
		if (IsValid(Minion))
		{
			// 2. 부하의 ASC를 빠르고 안전하게 가져옵니다.
			UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Minion);
           
			if (TargetASC)
			{
				// 3. 해당 태그를 포함하고 있는 모든 활성 GE를 제거합니다.
				TargetASC->RemoveActiveEffectsWithGrantedTags(BuffTagContainer);
			}
		}
	}
}

void AECEnemyElite::CachingManagedMinions()
{
	TArray<FOverlapResult> OverlapResults;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	const FVector Center = GetActorLocation();

	// 자신의 위치를 기준으로 ManagerRadius 반경 내의 Normal Enemy 클래스를 탐색합니다.
	bool bResult = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		Center,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(ManageRadius),
		QueryParams
	);

	if (!bResult)
	{
		return;
	}

	// 버프를 걸어줄 Normal Enemy들을 캐싱해서 가져옵니다.
	for (const FOverlapResult& Result : OverlapResults)
	{
		AECEnemyNormal* NormalEnemy = Cast<AECEnemyNormal>(Result.GetActor());

		if (!IsValid(NormalEnemy))
		{
			continue;
		}
		ManagedMinions.AddUnique(NormalEnemy);
	}
}

void AECEnemyElite::GrantEliteBuffToMinions()
{	
	if (ASC && ManagedMinions.Num() > 0)
	{     
		// 1. 이벤트에 실어 보낼 Payload를 만듭니다.
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = this;

		// 2. 여러 명의 액터를 담을 수 있는 TargetData를 생성합니다.
		FGameplayAbilityTargetData_ActorArray* TargetActors = new FGameplayAbilityTargetData_ActorArray();

		// 3. 캐싱해 둔 부하들을 타겟 데이터 배열에 집어넣습니다.
		for (AECEnemyNormal* Minion : ManagedMinions)
		{
			if (IsValid(Minion))
			{
				TargetActors->TargetActorArray.Add(Minion);
			}
		}

		// 4. Payload에 TargetData를 추가합니다.
		Payload.TargetData.Add(TargetActors);

		// 5. 이벤트를 발송하여 GA를 트리거합니다. (BuffAbilityTag 사용)
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, ECGameplayTags::Ability_Skill_GrantEliteBuff, Payload);
	}
}