// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/ECEnemyCharacterBase.h"

#include "AbilitySystem/ECAbilitySet.h"
#include "AbilitySystem/Attribute/ECCombatSet.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "AbilitySystem/Attribute/ECMoveSpeedSet.h"
#include "AI/ECEnemyAIController.h"
#include "Character/Enemy/Component/DeathVisualComponent.h"
#include "Component/HitReactionComponent.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Item/ECItemDropComponent.h"
#include "Subsystem/ECEnemyDataSubsystem.h"
#include "Subsystem/ECGameplayMessageSubsystem.h"
#include "Subsystem/ECMessageTypes.h"
#include "UI/ViewModel/ECViewModelFactoryLibrary.h"
#include "UI/ViewModel/HealthViewModel.h"
#include "UI/Widget/ECUserWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogECEnemyDeath, Log, All);

AECEnemyCharacterBase::AECEnemyCharacterBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	ASC = CreateDefaultSubobject<UECAbilitySystemComponent>(TEXT("ASC"));

	HealthSet = CreateDefaultSubobject<UECHealthSet>(TEXT("HealthSet"));
	MoveSpeedSet = CreateDefaultSubobject<UECMoveSpeedSet>(TEXT("MoveSpeedSet"));
	CombatSet = CreateDefaultSubobject<UECCombatSet>(TEXT("CombatSet"));
	ItemDropComponent = CreateDefaultSubobject<UECItemDropComponent>(TEXT("ItemDropComponent"));
	DeathVisualComponent = CreateDefaultSubobject<UDeathVisualComponent>(TEXT("DeathVisualComponent"));

	{
		HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Bar Component"));
		HealthBarComponent->SetupAttachment(RootComponent);
		HealthBarComponent->SetVisibility(false);
	}
}

void AECEnemyCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitAbilities();
}

void AECEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeDefaultAttributes();
	HealthComponent->InitializeWithAbilitySystem(ASC);
	InitializeHealthBarComponent();
	InitializeHitReactionComponent();
}

void AECEnemyCharacterBase::OnDeathStarted(AActor* OwningActor)
{
	GetWorldTimerManager().ClearTimer(DeathAnimationNotifyFallbackTimerHandle);
	bDeathStateFinished = false;
	bDeathVisualStarted = false;
	bDeathVisualFinished = false;
	bDeathCleanupStarted = false;

	if (DeathAnimationNotifyFallbackDelay <= 0.0f)
	{
		GetWorldTimerManager().SetTimerForNextTick(
			this,
			&ThisClass::HandleDeathAnimationNotifyFallback);
	}
	else
	{
		GetWorldTimerManager().SetTimer(
			DeathAnimationNotifyFallbackTimerHandle,
			this,
			&ThisClass::HandleDeathAnimationNotifyFallback,
			DeathAnimationNotifyFallbackDelay,
			false);
	}

	ItemDropComponent->SpawnDrop(LastDamageInstigator.Get());
	HideHealthBar();

	if (AECEnemyAIController* AIController = Cast<AECEnemyAIController>(GetController()))
	{
		AIController->HandlePawnDeath();
	}

	const FGameplayTag EnemyTag = UECEnemyDataSubsystem::Get(this).FindEnemyTag(GetClass());
	if (EnemyTag.IsValid())
	{
		FECNotificationMessage Message;
		Message.TargetChannel = TAG_Notification_Killed;
		Message.PayloadTag = EnemyTag;

		UECGameplayMessageSubsystem::Get(this).BroadcastMessage(TAG_AddNotification_Message, Message);
	}
	
	Super::OnDeathStarted(OwningActor);
}

void AECEnemyCharacterBase::OnDeathFinished(AActor* OwningActor)
{
	bDeathStateFinished = true;

	// Death Ability의 종료까지도 AnimNotify가 오지 않았다면 더 이상 기다리지 않습니다.
	if (!bDeathVisualStarted)
	{
		HandleDeathAnimationNotifyFallback();
	}

	TryFinalizeDeath();
}

void AECEnemyCharacterBase::HandleDeathAnimationFinished()
{
	GetWorldTimerManager().ClearTimer(DeathAnimationNotifyFallbackTimerHandle);

	if (bDeathVisualStarted)
	{
		return;
	}

	bDeathVisualStarted = true;

	if (!DeathVisualComponent)
	{
		HandleDeathVisualFinished();
		return;
	}

	DeathVisualComponent->PlayDeathVisual(
		FSimpleDelegate::CreateUObject(this, &ThisClass::HandleDeathVisualFinished));
}

void AECEnemyCharacterBase::HandleDeathAnimationNotifyFallback()
{
	if (bDeathVisualStarted)
	{
		return;
	}

	UE_LOG(
		LogECEnemyDeath,
		Warning,
		TEXT("[%s] 사망 애니메이션 종료 노티파이가 없어 디졸브를 안전 경로로 시작합니다."),
		*GetNameSafe(this));
	HandleDeathAnimationFinished();
}

void AECEnemyCharacterBase::CompleteDeathAfterExternalVisual()
{
	// 외부 시퀀스가 사망 연출을 담당한 경우 AnimNotify가 실제 보스에서 발생하지 않을 수 있습니다.
	bDeathVisualStarted = true;
	bDeathVisualFinished = true;

	if (HealthComponent && HealthComponent->GetDeathState() == EDeathState::DeathStarted)
	{
		HealthComponent->FinishDeath();
		return;
	}

	bDeathStateFinished = true;
	TryFinalizeDeath();
}

void AECEnemyCharacterBase::HandleDeathVisualFinished()
{
	bDeathVisualFinished = true;

	if (HealthComponent && HealthComponent->GetDeathState() == EDeathState::DeathStarted)
	{
		HealthComponent->FinishDeath();
		return;
	}

	bDeathStateFinished = true;
	TryFinalizeDeath();
}

void AECEnemyCharacterBase::TryFinalizeDeath()
{
	if (bDeathCleanupStarted || !bDeathStateFinished || !bDeathVisualFinished)
	{
		return;
	}

	bDeathCleanupStarted = true;
	GetWorldTimerManager().ClearTimer(DeathAnimationNotifyFallbackTimerHandle);
	Super::OnDeathFinished(this);
}

void AECEnemyCharacterBase::InitializeDefaultAttributes()
{
	// Enemy Level에 해당하는 Attributes를 적용합니다. 임시로 1.f
	ApplyAttributeEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyAttributeEffectToSelf(DefaultMaxVitalAttributes, 1.f);

	// 최대 체력만큼 현재 체력을 초기화시킵니다.
	HealthSet->SetHealth(HealthSet->GetMaxHealth());

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// 이동 속도가 변할 때 Movement Component의 MaxWalkSpeed에 반영하는 함수를 바인딩합니다.
		ASC->GetGameplayAttributeValueChangeDelegate(MoveSpeedSet->GetMoveSpeedAttribute()).AddUObject(this, &ThisClass::OnMoveSpeedChanged);
		// 이동 속도 Attribute 값을 적용합니다.
		MovementComponent->MaxWalkSpeed = MoveSpeedSet->GetMoveSpeed();
	}
}

void AECEnemyCharacterBase::InitializeHealthBarComponent()
{
	if (!HealthBarComponent)
	{
		return;
	}

	HideHealthBar();

	HealthComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
	HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);

	UECUserWidget* HealthBarWidget = Cast<UECUserWidget>(HealthBarComponent->GetUserWidgetObject());
	if (HealthBarWidget)
	{
		UHealthViewModel* HealthViewModel =	UECViewModelFactoryLibrary::CreateViewModel<UHealthViewModel>(this, HealthBarWidget);
		if (!HealthViewModel)
		{
			return;
		}

		HealthBarWidget->InjectViewModel(HealthViewModel);

		// 새 공용 체력바는 MVVM 바인딩이 없어도 이름을 즉시 표시합니다.
		if (UTextBlock* EnemyNameText =	Cast<UTextBlock>(HealthBarWidget->GetWidgetFromName(TEXT("Text_EnemyName"))))
		{
			EnemyNameText->SetText(HealthViewModel->GetEnemyName());
		}
	}
}

void AECEnemyCharacterBase::HandleHealthChanged(
	UECHealthComponent* ChangedHealthComponent,
	const float OldValue,
	const float NewValue,
	AActor* DamageInstigator)
{
	if (NewValue < OldValue && IsValid(DamageInstigator))
	{
		LastDamageInstigator = DamageInstigator;

		// 시야 밖에서 먼저 피격되더라도 즉시 공격자를 전투 대상으로 설정합니다.
		// 체력이 0이 된 경우에는 곧바로 사망 흐름이 시작되므로 Engage 이벤트를 보내지 않습니다.
		if (NewValue > 0.0f)
		{
			if (AECEnemyAIController* AIController = Cast<AECEnemyAIController>(GetController()))
			{
				AIController->EngageTarget(DamageInstigator);
			}
		}
	}

	if (!HealthBarComponent || NewValue >= OldValue)
	{
		return;
	}

	HealthBarComponent->SetVisibility(true);

	if (HealthBarVisibleDuration <= 0.0f)
	{
		HideHealthBar();
		return;
	}

	GetWorldTimerManager().SetTimer(
		HealthBarHideTimerHandle,
		this,
		&ThisClass::HideHealthBar,
		HealthBarVisibleDuration,
		false);
}

void AECEnemyCharacterBase::HideHealthBar()
{
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}
}

void AECEnemyCharacterBase::InitializeHitReactionComponent()
{
	UHitReactionComponent* HitReactionComponent = FindComponentByClass<UHitReactionComponent>();

	if (!HitReactionComponent)
	{
		return;
	}

	HitReactionComponent->InitializeWithAbilitySystem(ASC);
}

void AECEnemyCharacterBase::ApplyAttributeEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass, float Level)
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	
	// Level에 해당하는 SpecHandle을 만듭니다.
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);

	// 최종 SpecHandle을 Enemy에게 적용합니다.
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AECEnemyCharacterBase::InitAbilities()
{
	ASC->InitAbilityActorInfo(this, this);

	for (UECAbilitySet* AbilitySet : DefaultAbilitySets)
	{
		if (AbilitySet)
		{
			// OutGrantedHandle을 nullptr로 지정한다는 것은 영구 부여한다는 의미입니다.
			AbilitySet->GiveToAbilitySystem(ASC, nullptr);
		}
	}
}

void AECEnemyCharacterBase::OnMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = Data.NewValue;
	}
}
