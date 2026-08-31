#include "ECPawnComponent_CharacterParts.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Character.h"

bool FECCharacterPartList::SpawnActorForEntry(FECAppliedCharacterPartEntry& Entry)
{
	bool bCreatedAnyActor = false;

	if (Entry.Part.PartClass != nullptr)
	{
		if (USceneComponent* ComponentToAttachTo = OwnerComponent->GetSceneComponentToAttachTo())
		{
			// ChildActorComponent로 생성해 Owner 파괴 시 자동 정리되게 합니다.
			UChildActorComponent* PartComponent = NewObject<UChildActorComponent>(OwnerComponent->GetOwner());
			PartComponent->SetupAttachment(ComponentToAttachTo, Entry.Part.SocketName);
			PartComponent->SetChildActorClass(Entry.Part.PartClass);
			PartComponent->RegisterComponent();

			// 부모 Transform 확정 후 파츠가 따라붙도록 Tick 선행 조건을 설정합니다.
			if (AActor* SpawnedActor = PartComponent->GetChildActor())
			{
				if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
				{
					SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
				}
			}

			Entry.SpawnedComponent = PartComponent;
			bCreatedAnyActor = true;
		}
	}

	return bCreatedAnyActor;
}

void FECCharacterPartList::DestroyActorForEntry(FECAppliedCharacterPartEntry& Entry)
{
	if (Entry.SpawnedComponent)
	{
		Entry.SpawnedComponent->DestroyComponent();
		Entry.SpawnedComponent = nullptr;
	}
}

FECCharacterPartHandle FECCharacterPartList::AddEntry(FECCharacterPart NewPart)
{
	FECCharacterPartHandle Result;
	Result.PartHandle = PartHandleCounter++;

	// 파츠 스폰은 Authority에서만 수행합니다.
	if (ensure(OwnerComponent && OwnerComponent->GetOwner() && OwnerComponent->GetOwner()->HasAuthority()))
	{
		FECAppliedCharacterPartEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Part = NewPart;
		NewEntry.PartHandle = Result.PartHandle;

		// 스폰에 성공하면 태그를 재수집해 Mesh를 갱신합니다.
		if (SpawnActorForEntry(NewEntry))
		{
			OwnerComponent->BroadcastChanged();
		}
	}

	return Result;
}

void FECCharacterPartList::RemoveEntry(FECCharacterPartHandle Handle)
{
	// 핸들이 일치하는 Entry를 찾아 파괴 후 배열에서 제거합니다.
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FECAppliedCharacterPartEntry& Entry = *EntryIt;

		if (Entry.PartHandle == Handle.PartHandle)
		{
			DestroyActorForEntry(Entry);
			EntryIt.RemoveCurrent();
			break;
		}
	}
}

FGameplayTagContainer FECCharacterPartList::CollectCombinedTags() const
{
	FGameplayTagContainer Result;

	for (const FECAppliedCharacterPartEntry& Entry : Entries)
	{
		if (Entry.SpawnedComponent)
		{
			// IGameplayTagAssetInterface를 구현한 파츠에서만 태그를 수집합니다.
			if (IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Entry.SpawnedComponent->GetChildActor()))
			{
				TagInterface->GetOwnedGameplayTags(Result);
			}
		}
	}

	return Result;
}

UECPawnComponent_CharacterParts::UECPawnComponent_CharacterParts(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CharacterPartList(this)
{
}

USkeletalMeshComponent* UECPawnComponent_CharacterParts::GetParentMeshComponent() const
{
	if (AActor* OwnerActor = GetOwner())
	{
		if (ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor))
		{
			if (USkeletalMeshComponent* MeshComponent = OwningCharacter->GetMesh())
			{
				return MeshComponent;
			}
		}
	}

	return nullptr;
}

USceneComponent* UECPawnComponent_CharacterParts::GetSceneComponentToAttachTo() const
{
	// Mesh가 있으면 소켓 활용을 위해 Mesh에 부착합니다.
	if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
	{
		return MeshComponent;
	}

	// Mesh가 없는 Pawn이면 RootComponent에 부착합니다.
	if (AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetRootComponent();
	}

	return nullptr;
}

FGameplayTagContainer UECPawnComponent_CharacterParts::GetCombinedTags(FGameplayTag RequiredPrefix) const
{
	FGameplayTagContainer Result = CharacterPartList.CollectCombinedTags();

	// Prefix가 유효하면 해당 접두사 태그만 추려 반환합니다.
	if (RequiredPrefix.IsValid())
	{
		return Result.Filter(FGameplayTagContainer(RequiredPrefix));
	}
	else
	{
		return Result;
	}
}

void UECPawnComponent_CharacterParts::BroadcastChanged()
{
	const bool bReinitPose = true;

	if (USkeletalMeshComponent* MeshComponent = GetParentMeshComponent())
	{
		// 장착된 파츠들의 태그를 합산해 최적 Mesh를 선택·적용합니다.
		const FGameplayTagContainer MergedTags = GetCombinedTags(FGameplayTag());

		USkeletalMesh* DesiredMesh = BodyMeshes.SelectBestBodyStyle(MergedTags);

		MeshComponent->SetSkeletalMesh(DesiredMesh, bReinitPose);

		// PhysicsAsset은 모든 Mesh에 공통으로 강제 적용합니다.
		if (UPhysicsAsset* PhysicsAsset = BodyMeshes.ForcedPhysicsAsset)
		{
			MeshComponent->SetPhysicsAsset(PhysicsAsset, bReinitPose);
		}
	}
}

FECCharacterPartHandle UECPawnComponent_CharacterParts::AddCharacterPart(const FECCharacterPart& NewPart)
{
	return CharacterPartList.AddEntry(NewPart);
}

void UECPawnComponent_CharacterParts::RemoveCharacterPart(FECCharacterPartHandle Handle)
{
	return CharacterPartList.RemoveEntry(Handle);
}
