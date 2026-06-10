#include "ECCharacterBase.h"
#include "ECGameplayTags.h"
#include "ECCharacterMovementComponent.h"
#include "Component/Attribute/ECHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AECCharacterBase::AECCharacterBase(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UECCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// HealthComponent를 생성합니다.
	{
		HealthComponent = CreateDefaultSubobject<UECHealthComponent>(TEXT("HealthComponent"));
		HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
		HealthComponent->OnDeathFinished.AddDynamic(this, &ThisClass::OnDeathFinished);
	}
}

void AECCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateMovingTag();
}

void AECCharacterBase::UpdateMovingTag()
{
	// possess 이전에는 ASC가 아직 준비되지 않았으므로 건너뜁니다.
	if (!ASC)
	{
		return;
	}

	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	// 이동 입력(가속도)이 있거나, 데드존을 넘는 실제 속도가 있으면 '이동 중'으로 판단합니다.
	const bool bHasInput = !MoveComp->GetCurrentAcceleration().IsNearlyZero();
	const bool bHasSpeed = MoveComp->Velocity.Size2D() > 3.0f;
	const bool bIsMovingNow = bHasInput || bHasSpeed;

	// 상태가 바뀐 프레임에만 태그를 갱신해 불필요한 태그 이벤트를 줄입니다.
	if (bIsMovingNow == bIsMoving)
	{
		return;
	}

	bIsMoving = bIsMovingNow;
	ASC->SetLooseGameplayTagCount(ECGameplayTags::Status_Moving, bIsMoving ? 1 : 0);
}

void AECCharacterBase::OnDeathStarted(AActor* OwningActor)
{
	StartDeathProcess();
	DisableMovementAndCollision();
}

void AECCharacterBase::OnDeathFinished(AActor* OwningActor)
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::DestroyDueToDeath);
}

void AECCharacterBase::DisableMovementAndCollision()
{
	if (GetController())
	{
		GetController()->SetIgnoreMoveInput(true);
	}

	// CapsuleComponent의 충돌을 비활성화 합니다.
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Character의 MovementMode를 MOVE_NONE으로 설정합니다.
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->StopMovementImmediately();
	MoveComp->DisableMovement();
}

void AECCharacterBase::StartDeathProcess()
{
	// @TODO : 사망 애니메이션 몽타주 재생 같은 기능을 추가하면 될 것 같습니다.
	
	// 따로 사망 연출이 없을까봐 임시로 추가해둔 래그돌 코드입니다.
	/*// Mesh의 Ragdoll 연출을 출력합니다.
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetAllBodiesBelowSimulatePhysics(FName(TEXT("pelvis")), true, true);
	GetMesh()->SetVisibility(false);
	
	// 마지막으로 이동하던 방향으로 힘을 가합니다.
	const FVector LastUpdateVelocity = GetCharacterMovement()->GetLastUpdateVelocity();
	const FVector RagdollImpulseVector = LastUpdateVelocity.GetSafeNormal(0.0001f) * 200.0f;
	GetMesh()->AddImpulse(LastUpdateVelocity + RagdollImpulseVector, FName(TEXT("pelvis")), true);*/
}

void AECCharacterBase::DestroyDueToDeath()
{
	// 액터와 컨트롤러를 분리합니다.
	DetachFromControllerPendingDestroy();

	// 액터의 모습을 비활성화 합니다.
	SetActorHiddenInGame(true);
	
	// 0.1초 후 파괴합니다.
	SetLifeSpan(0.1f);
}
