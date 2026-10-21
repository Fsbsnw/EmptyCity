#include "Character/Player/Component/ECFootstepComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Subsystem/ECSoundSubsystem.h"

namespace FootstepSoundRows
{
	const FName Normal(TEXT("Player_Walk_Normal"));
	const FName Concrete(TEXT("Player_Walk_Concrete"));
	const FName Dirt(TEXT("Player_Walk_Dirt"));
	const FName Flat(TEXT("Player_Walk_Flat"));
}

namespace FootstepTraceConstants
{
	constexpr float GroundTraceDistance = 30.0f;
}

UECFootstepComponent::UECFootstepComponent()
{
	// 지상 이동 거리를 매 프레임 누적합니다.
	{
		PrimaryComponentTick.bCanEverTick = true;
		PrimaryComponentTick.bStartWithTickEnabled = true;
	}
}

void UECFootstepComponent::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어 생성 시 함께 생성되므로 소유 캐릭터의 유효성이 보장됩니다.
	OwnerCharacter = CastChecked<ACharacter>(GetOwner());
}

void UECFootstepComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	const float HorizontalSpeed = MovementComponent->Velocity.Size2D();

	// 공중이거나 멈춘 동안에는 다음 이동을 새로운 걸음으로 시작합니다.
	if (!MovementComponent->IsMovingOnGround() || HorizontalSpeed <= UE_KINDA_SMALL_NUMBER)
	{
		AccumulatedDistance = 0.0f;
		return;
	}

	// 실제 이동 속도를 거리로 누적하여 이동 속도에 비례한 발걸음 간격을 만듭니다.
	AccumulatedDistance += HorizontalSpeed * DeltaTime;

	// 설정한 한 걸음 거리에 도달하기 전까지 재생을 기다립니다.
	if (AccumulatedDistance < StepDistance)
	{
		return;
	}

	// 프레임 지연으로 초과한 거리를 보존하여 발걸음 간격이 흔들리지 않게 합니다.
	AccumulatedDistance = FMath::Fmod(AccumulatedDistance, StepDistance);
	PlayFootstep();
}

void UECFootstepComponent::PlayFootstep()
{
	const FVector TraceStart = OwnerCharacter->GetActorLocation();
	const FVector TraceEnd = TraceStart - FVector::UpVector * (OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + FootstepTraceConstants::GroundTraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.bReturnPhysicalMaterial = true;

	// 캡슐 중심에서 발밑까지 검사하며, 바닥을 찾지 못하면 발소리를 생략합니다.
	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return;
	}

	// 지정되지 않은 표면과 Normal 표면에는 기본 발소리를 사용합니다.
	FName SoundRowName = FootstepSoundRows::Normal;
	switch (UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get()))
	{
	case SurfaceType2:
		SoundRowName = FootstepSoundRows::Concrete;
		break;

	case SurfaceType3:
		SoundRowName = FootstepSoundRows::Dirt;
		break;

	case SurfaceType4:
		SoundRowName = FootstepSoundRows::Flat;
		break;

	case SurfaceType_Default:
	case SurfaceType1:
	default:
		break;
	}

	// 기존 사운드 데이터 테이블과 SFX 풀을 통해 충돌 지점에서 재생합니다.
	UECSoundSubsystem::Get(this).PlaySFXAtLocationByName(SoundRowName, Hit.ImpactPoint);
}
