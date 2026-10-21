#include "Character/Enemy/Component/DeathVisualComponent.h"

#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

UDeathVisualComponent::UDeathVisualComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
}

void UDeathVisualComponent::PlayDeathVisual(FSimpleDelegate InFinishedDelegate)
{
	if (bIsPlaying)
	{
		return;
	}

	bIsPlaying = true;
	bDissolveFinished = false;
	bNiagaraFinished = true;
	DeathDissolveComponent = nullptr;
	FinishedDelegate = MoveTemp(InFinishedDelegate);

	if (!GetWorld() || DissolveStartDelay <= 0.0f)
	{
		StartDissolve();
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		DissolveStartTimerHandle,
		this,
		&ThisClass::StartDissolve,
		DissolveStartDelay,
		false);
}

void UDeathVisualComponent::StartDissolve()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner) || !GetWorld())
	{
		bDissolveFinished = true;
		FinishDeathVisual();
		return;
	}

	DissolveMaterialInstances.Reset();

	TInlineComponentArray<UMeshComponent*> MeshComponents;
	Owner->GetComponents(MeshComponents);

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
		{
			if (UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(MaterialIndex))
			{
				DynamicMaterial->SetScalarParameterValue(DissolveParameterName, DissolveStartValue);
				DissolveMaterialInstances.Add(DynamicMaterial);
			}
		}
	}

	if (DeathDissolveVFX)
	{
		if (USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>())
		{
			DeathDissolveComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				DeathDissolveVFX,
				OwnerMesh,
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true,
				false);

			if (DeathDissolveComponent)
			{
				bNiagaraFinished = false;
				DeathDissolveComponent->OnSystemFinished.AddDynamic(
					this,
					&ThisClass::HandleNiagaraSystemFinished);
				DeathDissolveComponent->SetVariableFloat(
					NiagaraDissolveParameterName,
					DissolveStartValue);
				DeathDissolveComponent->Activate(true);
			}
		}
	}

	if (DissolveDuration <= KINDA_SMALL_NUMBER)
	{
		CompleteDissolve();
		return;
	}

	DissolveStartedAt = GetWorld()->GetTimeSeconds();
	UpdateDissolve();

	constexpr float DissolveUpdateInterval = 0.02f;
	GetWorld()->GetTimerManager().SetTimer(
		DissolveUpdateTimerHandle,
		this,
		&ThisClass::UpdateDissolve,
		DissolveUpdateInterval,
		true);
}

void UDeathVisualComponent::UpdateDissolve()
{
	if (!GetWorld())
	{
		FinishDeathVisual();
		return;
	}

	const float ElapsedTime = GetWorld()->GetTimeSeconds() - DissolveStartedAt;
	const float Alpha = FMath::Clamp(ElapsedTime / DissolveDuration, 0.0f, 1.0f);
	const float CurveValue = DissolveCurve ? DissolveCurve->GetFloatValue(Alpha) : Alpha;
	const float DissolveValue = FMath::Lerp(DissolveStartValue, DissolveEndValue, CurveValue);

	for (UMaterialInstanceDynamic* DynamicMaterial : DissolveMaterialInstances)
	{
		if (IsValid(DynamicMaterial))
		{
			DynamicMaterial->SetScalarParameterValue(DissolveParameterName, DissolveValue);
		}
	}

	if (DeathDissolveComponent)
	{
		DeathDissolveComponent->SetVariableFloat(NiagaraDissolveParameterName, DissolveValue);
	}

	if (Alpha >= 1.0f)
	{
		CompleteDissolve();
	}
}

void UDeathVisualComponent::CompleteDissolve()
{
	if (!bIsPlaying || bDissolveFinished)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DissolveUpdateTimerHandle);
	}

	for (UMaterialInstanceDynamic* DynamicMaterial : DissolveMaterialInstances)
	{
		if (IsValid(DynamicMaterial))
		{
			DynamicMaterial->SetScalarParameterValue(DissolveParameterName, DissolveEndValue);
		}
	}

	if (DeathDissolveComponent)
	{
		DeathDissolveComponent->SetVariableFloat(NiagaraDissolveParameterName, DissolveEndValue);
	}

	bDissolveFinished = true;

	if (!bNiagaraFinished && GetWorld())
	{
		if (NiagaraCompletionTimeout <= 0.0f)
		{
			HandleNiagaraCompletionTimeout();
			return;
		}

		GetWorld()->GetTimerManager().SetTimer(
			NiagaraCompletionTimerHandle,
			this,
			&ThisClass::HandleNiagaraCompletionTimeout,
			NiagaraCompletionTimeout,
			false);
	}

	TryFinishDeathVisual();
}

void UDeathVisualComponent::HandleNiagaraSystemFinished(UNiagaraComponent* FinishedComponent)
{
	if (FinishedComponent != DeathDissolveComponent)
	{
		return;
	}

	bNiagaraFinished = true;
	TryFinishDeathVisual();
}

void UDeathVisualComponent::HandleNiagaraCompletionTimeout()
{
	bNiagaraFinished = true;

	if (DeathDissolveComponent)
	{
		DeathDissolveComponent->DeactivateImmediate();
	}

	TryFinishDeathVisual();
}

void UDeathVisualComponent::TryFinishDeathVisual()
{
	if (bDissolveFinished && bNiagaraFinished)
	{
		FinishDeathVisual();
	}
}

void UDeathVisualComponent::FinishDeathVisual()
{
	if (!bIsPlaying)
	{
		return;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DissolveStartTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DissolveUpdateTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(NiagaraCompletionTimerHandle);
	}

	if (DeathDissolveComponent)
	{
		DeathDissolveComponent->OnSystemFinished.RemoveDynamic(
			this,
			&ThisClass::HandleNiagaraSystemFinished);
	}

	bIsPlaying = false;
	FinishedDelegate.ExecuteIfBound();
	FinishedDelegate.Unbind();
}
