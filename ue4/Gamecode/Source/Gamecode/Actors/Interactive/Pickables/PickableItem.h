// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interface/Interactive.h"
#include "GameCodeTypes.h"
#include <Components/SphereComponent.h>
#include "PickableItem.generated.h"

class IInteractable;
UCLASS(Blueprintable)
class GAMECODE_API APickableItem : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APickableItem();

	virtual void Interact(AGCBaseCharacter* Character) override;
	virtual FName GetActionEventName() const override;

	void SetItemsCount(int32 ItemsCount_In);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName DataTableID = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EInventoryItemType InventoryItemType = EInventoryItemType::None;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickableItemMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 ItemsCount = 1;
};
