#include "ECPlayerState.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/ECAbilitySet.h"
#include "AbilitySystem/ECAbilitySystemComponent.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "GameplayAbilitiesModule.h"
#include "AbilitySystem/Attribute/ECStaminaSet.h"
#include "AbilitySystem/Attribute/ECMoveSpeedSet.h"

AECPlayerState::AECPlayerState()
{
	ASC = CreateDefaultSubobject<UECAbilitySystemComponent>(TEXT("ASC"));

	CreateDefaultSubobject<UECHealthSet>(TEXT("HealthSet"));
	CreateDefaultSubobject<UECStaminaSet>(TEXT("StaminaSet"));
	CreateDefaultSubobject<UECMoveSpeedSet>(TEXT("MoveSpeedSet"));

	StatLevels.Emplace(EPlayerStatType::Health,  1);
	StatLevels.Emplace(EPlayerStatType::Stamina, 1);
	StatLevels.Emplace(EPlayerStatType::MoveSpeed, 1);
}

void AECPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	InitAbilities();
	InitStats();
}

void AECPlayerState::InitStats()
{
	if (!ASC) return;

	// bInitialInit = true → 최초 1회. 이후 LevelUp 시에는 false로 호출됩니다.
	for (auto& [StatType, Level] : StatLevels)
	{
		ApplyStatInit(StatType, Level, true);
	}
	bIsStatsInitialized = true;
	// 초기화가 완료된 후, HUD 초기화를 위해 이벤트를 전달합니다.(PlayerState -> PlayerController)
	OnStatsInitializedDelegate.Broadcast();
}

void AECPlayerState::ApplyStatInit(EPlayerStatType StatType, int32 Level, bool bInitialInit)
{
	if (!ASC) return;
	
	UAbilitySystemGlobals* ASG = IGameplayAbilitiesModule::Get().GetAbilitySystemGlobals();
	if (!ASG) return;

	FAttributeSetInitter* Initter = ASG->GetAttributeSetInitter();
	if (!Initter) return;

	// Enum을 FName으로 변환합니다.
	FName StatGroupName = FName(StaticEnum<EPlayerStatType>()->GetNameStringByValue((int32)StatType));
	
	// Global Data Table에서 Group을 찾아 Level에 맞게 값을 수정합니다.
	Initter->InitAttributeSetDefaults(ASC, StatGroupName, Level, bInitialInit);

	// Max 값이 갱신되었으니 현재 값도 Max에 맞춥니다. (초기화/레벨업 모두 풀회복)
	SyncCurrentToMax(StatType);
}

void AECPlayerState::SyncCurrentToMax(EPlayerStatType StatType)
{
	if (!ASC) return;

	switch (StatType)
	{
		case EPlayerStatType::Health:
			if (const UECHealthSet* HealthSet = ASC->GetSet<UECHealthSet>())
			{
				ASC->SetNumericAttributeBase(UECHealthSet::GetHealthAttribute(), HealthSet->GetMaxHealth());
			}
			break;

		case EPlayerStatType::Stamina:
			if (const UECStaminaSet* StaminaSet = ASC->GetSet<UECStaminaSet>())
			{
				ASC->SetNumericAttributeBase(UECStaminaSet::GetStaminaAttribute(), StaminaSet->GetMaxStamina());
			}
			break;

		default:
			break;
	}
}

void AECPlayerState::LevelUpStat(EPlayerStatType StatType, int32 InLevel)
{
	int32* Level = StatLevels.Find(StatType);
	if (!Level) return;
	(*Level) += InLevel;
	ApplyStatInit(StatType, *Level, false);
}

void AECPlayerState::InitAbilities()
{
	ASC->InitAbilityActorInfo(this, GetPawn());
	
	for (UECAbilitySet* AbilitySet : DefaultAbilitySets)
	{
		if (AbilitySet)
		{
			// OutGrantedHandle을 nullptr로 지정한다는 것은 영구 부여한다는 의미입니다.
			AbilitySet->GiveToAbilitySystem(ASC, nullptr);
		}
	}
}
