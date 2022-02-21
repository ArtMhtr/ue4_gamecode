// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "CharacterEquipmentComponent.generated.h"

class AThrowableItem;
class AMeleeWeaponItem;
class UEquipmentViewWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCurrentWeaponAmmoChangedEvent, int32, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEquippedItemChanged, const AEquipableItem*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnThrowableAmmoChanged, int32);

typedef TArray<int32, TInlineAllocator<(int32)EAmmunitionType::MAX>> TAmmunitionArray;
typedef TArray<class AEquipableItem*, TInlineAllocator<(int32)EEquipmentSlots::MAX>> TItemsArray;

USTRUCT(BlueprintType)
struct FThrowItemParams
{
	GENERATED_BODY()

	UPROPERTY()
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector Direction = FVector::ZeroVector;

	UPROPERTY()
	int32 CurrentThrowableItemIndex = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCharacterEquipmentComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	EEquipableItemType GetCurrentEquippedItemType() const;

	AMeleeWeaponItem* GetCurrentMeleeWeapon() const;
	ARangeWeaponItem* GetCurrentRangeWeapon() const;

	FOnCurrentWeaponAmmoChangedEvent OnCurrentWeaponAmmoChangedEvent;
	FOnEquippedItemChanged OnEquippedItemChanged;
	FOnThrowableAmmoChanged OnThrowableAmmoChanged;

	void ReloadCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void Server_ReloadCurrentWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_ReloadCurrentWeapon();

	void EquipItemInSlot(EEquipmentSlots Slot);

	void AttachCurrentItemToEquippedSocket();
	void EquipAnimationFinished();
	void UnEquipCurrentItem();
	void EquipNextItem();
	void EquipPrevItem();
	bool IsEquipping() const { return bIsEquipping; }
	void ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo = 0, bool bCheckIsFull = false);
	void LaunchCurrentThrowableItem();
	void UpdateThrowableIndex();
	void ChangeAmmoType();
	void OnThrowableAmmoChangedBroadcast(int32 AvailableAmmunition);
	bool CanThrowItem() const;

	bool AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> EquipableItemClass, int32 SlotIndex);
	void RemoveItemFromSlot(int32 SlotIndex);

	void OpenViewEquipment(APlayerController* PlayerContoroller);
	void CloseViewEquipment();
	bool IsViewVisible() const;

	void AddAmmunition(EAmmunitionType AmmunitionType, int32 Amount);
	void RemoveAmmunition(EAmmunitionType AmmunitionType, int32 Amount);

	const TArray<AEquipableItem*>& GetItems() const;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EAmmunitionType, int32> MaxAmmunitionAmount;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TMap<EEquipmentSlots, TSubclassOf<class AEquipableItem>> ItemsLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TSet<EEquipmentSlots> IgnoreSlotsWhileSwitching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	EEquipmentSlots AutoEquipItemInSlot = EEquipmentSlots::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
	TSubclassOf<UEquipmentViewWidget> ViewWidgetClass;

	void CreateViewWidget(APlayerController* PlayerController);

private:

	UPROPERTY(Replicated)
	TArray<AGCProjectile*> ThrowablesPool;

	UPROPERTY(Replicated)
	int32 CurrentThrowableIndex;

	const FVector ThrowablesPoolLocation = FVector(0.0f, 0.0f, -100);
	const int32 ThrowablesPoolSize = 10;

	void FillThrowablesPool(TSubclassOf<class AGCProjectile> ProjectileClass);
	void ThrowItem(FThrowItemParams ThrowItemParams);

	UFUNCTION(Server, Reliable)
	void Server_LaunchCurrentThrowableItem();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_ThrowItem(const FThrowItemParams& ThrowParams);

	UFUNCTION(Server, Reliable)
	void Server_EquipItemInSlot(EEquipmentSlots Slot);

	FThrowItemParams FillThrowItemParams();

	uint32 NextItemsArraySlotIndex(uint32 CurrentSlotIndex);
	uint32 PrevItemsArraySlotIndex(uint32 CurrentSlotIndex);

	UPROPERTY(Replicated)
	TArray<int32> AmmunitionArray;

	UPROPERTY(ReplicatedUsing=OnRep_ItemsArray)
	TArray<AEquipableItem*> ItemsArray;

	UFUNCTION()
	void OnRep_ItemsArray();

	void CreateLoadout();

	void AutoEquip();

	UFUNCTION()
	void OnCurrentWeaponAmmoChanged(int32 Ammo);

	UFUNCTION()
	void OnWeaponReloadComplete(int32 Ammo);
	int32 GetAvailableAmmunitionForCurrentWeapon();

	EEquipmentSlots PreviousEquippedSlot;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentEquippedSlot)
	EEquipmentSlots CurrentEquippedSlot;

	UFUNCTION()
	void OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old);

	AEquipableItem* CurrentEquippedItem;
	ARangeWeaponItem* CurrentEquippedWeapon;
	AThrowableItem* CurrentThrowableItem;
	AMeleeWeaponItem* CurrentMeleeWeapon;

	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;
	FDelegateHandle OnCurrentWeaponAmmoChangeHandle;
	FDelegateHandle OnCurrentWeaponAmmoReloadedHandle;
	bool bIsEquipping = false;
	FTimerHandle EquipTimer;

	TWeakObjectPtr<UEquipmentViewWidget> ViewWidget;
};
