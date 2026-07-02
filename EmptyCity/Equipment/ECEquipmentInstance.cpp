#include "ECEquipmentInstance.h"
#include "ECEquipmentDefinition.h"
#include "GameFramework/Character.h"

APawn* UECEquipmentInstance::GetPawn() const
{
	// NewObject 호출 시 OwnerComponent->GetOwner()(= Pawn)을 Outer로 지정했기 때문에 GetOuter()로 소유 Pawn을 얻는다.
	return Cast<APawn>(GetOuter());
}

APawn* UECEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{	
		// 소유 Pawn(Outer)이 요청한 타입과 같거나 그 파생 타입일 때만 반환한다. (타입이 맞지 않으면 nullptr)
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}
	return Result;
}

void UECEquipmentInstance::SpawnEquipmentActors(const TArray<FEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			// 일반 Pawn은 RootComponent에, Character는 Mesh 소켓에 부착해야 애니메이션이 올바르게 연동된다.
			AttachTarget = Char->GetMesh();
		}

		for (const FEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			// SpawnActorDeferred를 사용해 FinishSpawning 전에 Owner/Transform을 먼저 설정한다.
			AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			NewActor->FinishSpawning(FTransform::Identity, true);

			NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			NewActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
			SpawnedActors.Add(NewActor);
		}
	}
}

void UECEquipmentInstance::DestroyEquipmentActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
}

void UECEquipmentInstance::OnEquipped()
{

}

void UECEquipmentInstance::OnUnequipped()
{

}