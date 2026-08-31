// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/ECEnemyCharacterBase.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "AbilitySystem/ECAbilitySet.h"
#include "AbilitySystem/Attribute/ECCombatSet.h"
#include "AbilitySystem/Attribute/ECHealthSet.h"
#include "AbilitySystem/Attribute/ECMoveSpeedSet.h"
#include "AI/ECEnemyAIController.h"
#include "Component/HitReactionComponent.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/ECItemDropComponent.h"
#include "UI/ViewModel/ECViewModelFactoryLibrary.h"
#include "UI/ViewModel/HealthViewModel.h"
#include "UI/Widget/ECUserWidget.h"

AECEnemyCharacterBase::AECEnemyCharacterBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	ASC = CreateDefaultSubobject<UECAbilitySystemComponent>(TEXT("ASC"));

	HealthSet = CreateDefaultSubobject<UECHealthSet>(TEXT("HealthSet"));
	MoveSpeedSet = CreateDefaultSubobject<UECMoveSpeedSet>(TEXT("MoveSpeedSet"));
	CombatSet = CreateDefaultSubobject<UECCombatSet>(TEXT("CombatSet"));
	ItemDropComponent = CreateDefaultSubobject<UECItemDropComponent>(TEXT("ItemDropComponent"));

	{
		HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Bar Component"));
		HealthBarComponent->SetupAttachment(RootComponent);
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
	InitializeHealthBarComponent();
	InitializeHitReactionComponent();

	HealthComponent->InitializeWithAbilitySystem(ASC);
}

void AECEnemyCharacterBase::OnDeathStarted(AActor* OwningActor)
{
	ItemDropComponent->SpawnDrop();

	if (AECEnemyAIController* AIController = Cast<AECEnemyAIController>(GetController()))
	{
		AIController->HandlePawnDeath();
	}
	
	Super::OnDeathStarted(OwningActor);
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
	if (HealthBarComponent)
	{
		UECUserWidget* HealthBarWidget = Cast<UECUserWidget>(HealthBarComponent->GetUserWidgetObject());

		if (HealthBarWidget)
		{
			UHealthViewModel* HealthViewModel = UECViewModelFactoryLibrary::CreateViewModel<UHealthViewModel>(this, HealthBarWidget);

			HealthBarWidget->InjectViewModel(HealthViewModel);
		}
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
