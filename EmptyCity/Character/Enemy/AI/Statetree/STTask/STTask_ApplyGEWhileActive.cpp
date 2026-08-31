#include "STTask_ApplyGEWhileActive.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"

#include "AIController.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

FSTTask_ApplyGEWhileActive::FSTTask_ApplyGEWhileActive()
{
    // Tick은 필요하지 않습니다.
    // GE 적용은 EnterState, 제거는 ExitState에서만 수행합니다.
    bShouldCallTick = false;

    /*
     * 부모 상태가 계속 활성화된 상태에서 자식 상태만 바뀌는 경우,
     * 부모 Task의 ExitState/EnterState가 반복 호출되는 것을 막습니다.
     *
     * 예:
     * Alerted
     * ├─ Chase
     * └─ Attack
     *
     * Chase → Attack 전환 시 Alerted Task는 계속 유지됩니다.
     */
    bShouldStateChangeOnReselect = false;

    /*
     * 사용하는 UE 5.5 엔진 소스에 이 옵션이 존재한다면 아래도 사용할 수 있습니다.
     * 이 Task가 상태 완료 판정에 관여하지 않도록 합니다.
     *
     * bConsideredForCompletion = false;
     *
     * 프로젝트의 UE 5.5 브랜치에서 해당 멤버가 없으면 작성하지 않아도 됩니다.
     * 이 Task는 Running을 유지하고, 상태 전환은 이벤트/조건으로 처리하면 됩니다.
     */
}

EStateTreeRunStatus
FSTTask_ApplyGEWhileActive::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    /*
     * 부모 상태가 계속 선택된 상태에서 자식 상태만 변경되는 Sustained 전환이면
     * GE를 다시 적용하지 않습니다.
     *
     * bShouldStateChangeOnReselect가 false이므로 일반적으로 호출되지 않지만,
     * 방어적으로 한 번 더 확인합니다.
     */
    if (Transition.ChangeType == EStateTreeStateChangeType::Sustained)
    {
        return EStateTreeRunStatus::Running;
    }

    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.GameplayEffectClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyGameplayEffectWhileActive: GameplayEffectClass가 설정되지 않았습니다."));
        return EStateTreeRunStatus::Failed;
    }

    /*
     * 이미 정상적으로 적용된 상태라면 중복 적용하지 않습니다.
     */
    if (InstanceData.ActiveEffectHandle.IsValid() && IsValid(InstanceData.AppliedAbilitySystemComponent))
    {
        return EStateTreeRunStatus::Running;
    }

    // 이전에 불완전하게 남은 실행 정보를 정리합니다.
    InstanceData.ActiveEffectHandle = FActiveGameplayEffectHandle();

    InstanceData.AppliedAbilitySystemComponent = nullptr;

    AActor* TargetActor = ResolveTargetActor(Context);

    if (!IsValid(TargetActor))
    {
        UE_LOG(LogTemp, Error, TEXT("ApplyGameplayEffectWhileActive: 적용 대상 Actor를 찾지 못했습니다."));
        return EStateTreeRunStatus::Failed;
    }

    /*
     * AI StateTree는 일반적으로 서버에서 실행합니다.
     * 클라이언트에서 실행됐다면 서버 권한 GE를 임의로 적용하지 않습니다.
     */

    UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor,true);

    FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

    EffectContext.AddSourceObject(TargetActor);

    const FGameplayEffectSpecHandle SpecHandle =
        AbilitySystemComponent->MakeOutgoingSpec(
            InstanceData.GameplayEffectClass,
            InstanceData.EffectLevel,
            EffectContext);

    const FActiveGameplayEffectHandle AppliedHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

    InstanceData.AppliedAbilitySystemComponent = AbilitySystemComponent;

    InstanceData.ActiveEffectHandle = AppliedHandle;

    /*
     * 이 Task는 상태가 활성화되어 있는 동안 계속 살아 있어야 하므로
     * Running을 반환합니다.
     */
    return EStateTreeRunStatus::Running;
}

void FSTTask_ApplyGEWhileActive::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    /*
     * 부모 상태는 계속 활성화되어 있고 자식 상태만 바뀐 경우에는
     * GE를 제거하지 않습니다.
     */
    if (Transition.ChangeType == EStateTreeStateChangeType::Sustained)
    {
        return;
    }

    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    UAbilitySystemComponent* AbilitySystemComponent = InstanceData.AppliedAbilitySystemComponent;

    if (IsValid(AbilitySystemComponent) && InstanceData.ActiveEffectHandle.IsValid())
    {
        AbilitySystemComponent->RemoveActiveGameplayEffect(InstanceData.ActiveEffectHandle,-1);
    }

    InstanceData.ActiveEffectHandle = FActiveGameplayEffectHandle();
    InstanceData.AppliedAbilitySystemComponent = nullptr;
}


AActor* FSTTask_ApplyGEWhileActive::ResolveTargetActor(FStateTreeExecutionContext& Context) const
{
    UObject* OwnerObject = Context.GetOwner();

    if (!IsValid(OwnerObject))
    {
        return nullptr;
    }

    /*
     * StateTreeComponent가 AIController를 Owner로 사용한다면
     * 실제 GE 대상은 AIController가 아니라 Controlled Pawn입니다.
     */
    if (AAIController* AIController = Cast<AAIController>(OwnerObject))
    {
        return AIController->GetPawn();
    }

    /*
     * 실행 Owner가 ActorComponent인 구성까지 대응합니다.
     */
    if (UActorComponent* OwnerComponent = Cast<UActorComponent>(OwnerObject))
    {
        AActor* ComponentOwner = OwnerComponent->GetOwner();

        if (AAIController* AIController = Cast<AAIController>(ComponentOwner))
        {
            return AIController->GetPawn();
        }

        return ComponentOwner;
    }

    /*
     * StateTree Owner가 Character, Pawn 또는 일반 Actor라면
     * 해당 Actor 자신에게 적용합니다.
     */
    return Cast<AActor>(OwnerObject);
}