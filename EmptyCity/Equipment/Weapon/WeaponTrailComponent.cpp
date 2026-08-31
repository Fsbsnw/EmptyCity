#include "Equipment/Weapon/WeaponTrailComponent.h"

#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UWeaponTrailComponent::UWeaponTrailComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UWeaponTrailComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UWeaponTrailComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ForceEndTrail();
	Super::EndPlay(EndPlayReason);
}

void UWeaponTrailComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsTrailActive() && !UpdateTrailParameters())
	{
		// Trail이 활성화 됐지만, Ribbon 파라미터에 값을 전달하는 과정에서 문제가 발생하면 강제로 종료합니다.
		ForceEndTrail();
	}
}

bool UWeaponTrailComponent::BeginTrail()
{
	if (IsTrailActive())
	{
		++ActiveTrailRequestCount;
		return true;
	}

	if (!ResolveSourceMesh() || !EnsureTrailEffectComponent() || !UpdateTrailParameters())
	{
		return false;
	}

	ActiveTrailRequestCount = 1;
	TrailEffectComponent->Activate(true);
	SetComponentTickEnabled(true);
	return true;
}

void UWeaponTrailComponent::EndTrail()
{
	if (!IsTrailActive())
	{
		return;
	}

	--ActiveTrailRequestCount;
	if (ActiveTrailRequestCount == 0)
	{
		StopTrailEffect();
	}
}

void UWeaponTrailComponent::ForceEndTrail()
{
	ActiveTrailRequestCount = 0;
	StopTrailEffect();
}

bool UWeaponTrailComponent::ResolveSourceMesh()
{
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	if (UMeshComponent* ReferencedMesh = Cast<UMeshComponent>(SourceMeshComponent.GetComponent(OwnerActor)))
	{
		if (ReferencedMesh->DoesSocketExist(StartSocket) && ReferencedMesh->DoesSocketExist(EndSocket))
		{
			CachedSourceMesh = ReferencedMesh;
			return true;
		}
	}

	// FComponentReference를 찾지 못 한 경우, 해당 소켓들을 보유하고 있는 모든 Mesh Component를 탐색합니다.
	TArray<UMeshComponent*> MeshComponents;
	OwnerActor->GetComponents(MeshComponents);
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (IsValid(MeshComponent) && MeshComponent->DoesSocketExist(StartSocket) && MeshComponent->DoesSocketExist(EndSocket))
		{
			CachedSourceMesh = MeshComponent;
			return true;
		}
	}

	UE_LOG(LogTemp,	Warning, TEXT("[%s] WeaponTrail 소켓을 가진 MeshComponent를 찾지 못했습니다."), *GetNameSafe(OwnerActor));
	return false;
}

bool UWeaponTrailComponent::EnsureTrailEffectComponent()
{
	if (IsValid(TrailEffectComponent))
	{
		return true;
	}

	UMeshComponent* SourceMesh = CachedSourceMesh.Get();
	if (!IsValid(TrailSystem) || !IsValid(SourceMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] WeaponTrail NiagaraSystem 또는 SourceMesh가 설정되지 않았습니다."), *GetNameSafe(GetOwner()));
		return false;
	}

	TrailEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		TrailSystem,
		SourceMesh,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		false,
		false,
		ENCPoolMethod::None,
		false);

	return IsValid(TrailEffectComponent);
}

bool UWeaponTrailComponent::UpdateTrailParameters()
{
	UMeshComponent* SourceMesh = CachedSourceMesh.Get();
	if (!IsValid(SourceMesh) || !IsValid(TrailEffectComponent))
	{
		return false;
	}

	if (!SourceMesh->DoesSocketExist(StartSocket) || !SourceMesh->DoesSocketExist(EndSocket))
	{
		return false;
	}

	// Ribbon 파라미터에 무기 소켓 위치값을 전달합니다.
	TrailEffectComponent->SetVariablePosition(TrailStartParameter, SourceMesh->GetSocketLocation(StartSocket));
	TrailEffectComponent->SetVariablePosition(TrailEndParameter, SourceMesh->GetSocketLocation(EndSocket));
	return true;
}

void UWeaponTrailComponent::StopTrailEffect()
{
	SetComponentTickEnabled(false);

	if (IsValid(TrailEffectComponent))
	{
		TrailEffectComponent->Deactivate();
	}
}