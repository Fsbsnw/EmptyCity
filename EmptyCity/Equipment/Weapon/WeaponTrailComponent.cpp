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
		if (ActiveSourceMode != ETrailSourceMode::ComponentDefault)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%s] 이미 다른 소스의 WeaponTrail이 활성화되어 새 요청을 시작하지 않았습니다."),
				*GetNameSafe(GetOwner()));
			return false;
		}

		++ActiveTrailRequestCount;
		return true;
	}

	UMeshComponent* DefaultSourceMesh = nullptr;
	if (!ResolveDefaultSourceMesh(DefaultSourceMesh))
	{
		return false;
	}

	return BeginTrailInternal(ETrailSourceMode::ComponentDefault, DefaultSourceMesh, StartSocket, EndSocket);
}

bool UWeaponTrailComponent::BeginTrailFromMesh(UMeshComponent* InSourceMesh, FName InStartSocket, FName InEndSocket)
{
	return BeginTrailInternal(ETrailSourceMode::ExplicitMesh, InSourceMesh, InStartSocket, InEndSocket);
}

bool UWeaponTrailComponent::BeginTrailInternal(ETrailSourceMode InSourceMode, UMeshComponent* InSourceMesh,
	FName InStartSocket, FName InEndSocket)
{
	if (!IsValid(InSourceMesh) || InStartSocket.IsNone() || InEndSocket.IsNone() ||
		!InSourceMesh->DoesSocketExist(InStartSocket) || !InSourceMesh->DoesSocketExist(InEndSocket))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] WeaponTrail 소스로 지정한 Mesh 또는 본/소켓이 유효하지 않습니다."),
			*GetNameSafe(GetOwner()));
		return false;
	}

	if (IsTrailActive())
	{
		if (!IsActiveSource(InSourceMode, InSourceMesh, InStartSocket, InEndSocket))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[%s] 이미 다른 소스의 WeaponTrail이 활성화되어 새 요청을 시작하지 않았습니다."),
				*GetNameSafe(GetOwner()));
			return false;
		}

		++ActiveTrailRequestCount;
		return true;
	}

	CachedSourceMesh = InSourceMesh;
	ActiveSourceMode = InSourceMode;
	ActiveStartSocket = InStartSocket;
	ActiveEndSocket = InEndSocket;

	if (!EnsureTrailEffectComponent() || !UpdateTrailParameters())
	{
		CachedSourceMesh.Reset();
		ActiveSourceMode = ETrailSourceMode::None;
		ActiveStartSocket = NAME_None;
		ActiveEndSocket = NAME_None;
		return false;
	}

	ActiveTrailRequestCount = 1;
	TrailEffectComponent->Activate(true);
	SetComponentTickEnabled(true);
	return true;
}

void UWeaponTrailComponent::EndTrail()
{
	if (!IsTrailActive() || ActiveSourceMode != ETrailSourceMode::ComponentDefault)
	{
		return;
	}

	EndActiveTrailRequest();
}

void UWeaponTrailComponent::EndTrailFromMesh(UMeshComponent* InSourceMesh, FName InStartSocket, FName InEndSocket)
{
	EndTrailInternal(ETrailSourceMode::ExplicitMesh, InSourceMesh, InStartSocket, InEndSocket);
}

void UWeaponTrailComponent::EndTrailInternal(ETrailSourceMode InSourceMode, UMeshComponent* InSourceMesh,
	FName InStartSocket, FName InEndSocket)
{
	if (!IsTrailActive() || !IsActiveSource(InSourceMode, InSourceMesh, InStartSocket, InEndSocket))
	{
		return;
	}

	EndActiveTrailRequest();
}

void UWeaponTrailComponent::EndActiveTrailRequest()
{
	--ActiveTrailRequestCount;
	if (!IsTrailActive())
	{
		StopTrailEffect();
	}
}

void UWeaponTrailComponent::ForceEndTrail()
{
	ActiveTrailRequestCount = 0;
	StopTrailEffect();
}

bool UWeaponTrailComponent::ResolveDefaultSourceMesh(UMeshComponent*& OutSourceMesh) const
{
	OutSourceMesh = nullptr;

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	if (UMeshComponent* ReferencedMesh = Cast<UMeshComponent>(SourceMeshComponent.GetComponent(OwnerActor)))
	{
		if (ReferencedMesh->DoesSocketExist(StartSocket) && ReferencedMesh->DoesSocketExist(EndSocket))
		{
			OutSourceMesh = ReferencedMesh;
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
			OutSourceMesh = MeshComponent;
			return true;
		}
	}

	UE_LOG(LogTemp,	Warning, TEXT("[%s] WeaponTrail 소켓을 가진 MeshComponent를 찾지 못했습니다."), *GetNameSafe(OwnerActor));
	return false;
}

UNiagaraSystem* UWeaponTrailComponent::GetTrailSystemForSource(ETrailSourceMode InSourceMode) const
{
	switch (InSourceMode)
	{
	case ETrailSourceMode::ComponentDefault:
		return WeaponTrailSystem;

	case ETrailSourceMode::ExplicitMesh:
		return KickTrailSystem;

	default:
		return nullptr;
	}
}

bool UWeaponTrailComponent::EnsureTrailEffectComponent()
{
	UMeshComponent* SourceMesh = CachedSourceMesh.Get();
	UNiagaraSystem* DesiredTrailSystem = GetTrailSystemForSource(ActiveSourceMode);
	if (!IsValid(DesiredTrailSystem) || !IsValid(SourceMesh))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[%s] 현재 트레일 소스에 사용할 NiagaraSystem 또는 SourceMesh가 설정되지 않았습니다."),
			*GetNameSafe(GetOwner()));
		return false;
	}

	if (IsValid(TrailEffectComponent))
	{
		if (TrailEffectComponent->GetAsset() != DesiredTrailSystem)
		{
			TrailEffectComponent->Deactivate();
			TrailEffectComponent->SetAsset(DesiredTrailSystem);
		}

		if (TrailEffectComponent->GetAttachParent() != SourceMesh)
		{
			TrailEffectComponent->AttachToComponent(SourceMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		return true;
	}

	TrailEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		DesiredTrailSystem,
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

	if (!SourceMesh->DoesSocketExist(ActiveStartSocket) || !SourceMesh->DoesSocketExist(ActiveEndSocket))
	{
		return false;
	}

	// Ribbon 파라미터에 선택한 메시의 본/소켓 위치값을 전달합니다.
	TrailEffectComponent->SetVariablePosition(TrailStartParameter, SourceMesh->GetSocketLocation(ActiveStartSocket));
	TrailEffectComponent->SetVariablePosition(TrailEndParameter, SourceMesh->GetSocketLocation(ActiveEndSocket));
	return true;
}

bool UWeaponTrailComponent::IsActiveSource(ETrailSourceMode InSourceMode, UMeshComponent* InSourceMesh,
	FName InStartSocket, FName InEndSocket) const
{
	return ActiveSourceMode == InSourceMode &&
		CachedSourceMesh.Get() == InSourceMesh &&
		ActiveStartSocket == InStartSocket &&
		ActiveEndSocket == InEndSocket;
}

void UWeaponTrailComponent::StopTrailEffect()
{
	SetComponentTickEnabled(false);

	if (IsValid(TrailEffectComponent))
	{
		TrailEffectComponent->Deactivate();
	}

	CachedSourceMesh.Reset();
	ActiveSourceMode = ETrailSourceMode::None;
	ActiveStartSocket = NAME_None;
	ActiveEndSocket = NAME_None;
}
