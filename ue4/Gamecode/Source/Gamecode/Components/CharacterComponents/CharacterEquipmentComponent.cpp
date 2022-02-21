// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterEquipmentComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "GameCodeTypes.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include <Net/UnrealNetwork.h>
#include "Actors/Projectiles/GCProjectile.h"
#include "UI/Widget/Equipment/EquipmentViewWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "Utils/GCDataTableUtils.h"
#include "CharacterInventoryComponent.h"

UCharacterEquipmentComponent::UCharacterEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UCharacterEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentEquippedSlot);
	DOREPLIFETIME(UCharacterEquipmentComponent, AmmunitionArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, ItemsArray);
	DOREPLIFETIME(UCharacterEquipmentComponent, ThrowablesPool);
	DOREPLIFETIME(UCharacterEquipmentComponent, CurrentThrowableIndex);
}

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterEquipmentComponent::BeginPlay() can only be used with AGCBaseCharacter"));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(GetOwner());
	CreateLoadout();
	AutoEquip();
}

EEquipableItemType UCharacterEquipmentComponent::GetCurrentEquippedItemType() const
{
	EEquipableItemType Result = EEquipableItemType::None;
	if (IsValid(CurrentEquippedItem))
	{
		Result = CurrentEquippedItem->GetEquipableItemType();;
	}
	return Result;
}

AMeleeWeaponItem* UCharacterEquipmentComponent::GetCurrentMeleeWeapon() const
{
	return CurrentMeleeWeapon;
}

ARangeWeaponItem* UCharacterEquipmentComponent::GetCurrentRangeWeapon() const
{
	return CurrentEquippedWeapon;
}

void UCharacterEquipmentComponent::ReloadCurrentWeapon()
{
	check(IsValid(CurrentEquippedWeapon));
	int32 AvailableAmmunition = GetAvailableAmmunitionForCurrentWeapon();
	if (AvailableAmmunition <= 0)
	{
		return;
	}

	if (CachedBaseCharacter->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_ReloadCurrentWeapon();
	}

	if (CachedBaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		NetMulticast_ReloadCurrentWeapon();
	}
}

void UCharacterEquipmentComponent::Server_ReloadCurrentWeapon_Implementation()
{
	ReloadCurrentWeapon();
}

void UCharacterEquipmentComponent::NetMulticast_ReloadCurrentWeapon_Implementation()
{
	CurrentEquippedWeapon->StartReload();
}

void UCharacterEquipmentComponent::EquipItemInSlot(EEquipmentSlots Slot)
{
	if (bIsEquipping)
	{
		return;
	}

	if (Slot == EEquipmentSlots::PrimaryItemSlot)
	{
		if (!CanThrowItem())
		{
			return;
		}
	}

	UnEquipCurrentItem();

	AEquipableItem* ItemToEquip = ItemsArray[(int32)Slot];
	CurrentEquippedItem = ItemToEquip;
	CurrentEquippedWeapon = Cast<ARangeWeaponItem>(CurrentEquippedItem);
	CurrentThrowableItem = Cast<AThrowableItem>(CurrentEquippedItem);
	CurrentMeleeWeapon = Cast<AMeleeWeaponItem>(CurrentEquippedItem);

	if (IsValid(CurrentEquippedItem))
	{
		UAnimMontage* EquipMontage = CurrentEquippedItem->GetCharacterEquipAnimMontage();
		if (IsValid(EquipMontage))
		{
			bIsEquipping = true;
			UAnimInstance* CharacterAnimInstance = CachedBaseCharacter->GetMesh()->GetAnimInstance();
			float EquipDuration = CharacterAnimInstance->Montage_Play(EquipMontage, 1.0f, EMontagePlayReturnType::Duration);
			GetWorld()->GetTimerManager().SetTimer(EquipTimer, this, &UCharacterEquipmentComponent::EquipAnimationFinished, EquipDuration, false);
		}
		else
		{
			AttachCurrentItemToEquippedSocket();
		}

		CurrentEquippedItem->Equip();
	}

	if (IsValid(CurrentEquippedWeapon))
	{
		OnCurrentWeaponAmmoChangeHandle = CurrentEquippedWeapon->OnAmmoChanged.AddUFunction(this, FName("OnCurrentWeaponAmmoChanged"));
		OnCurrentWeaponAmmoReloadedHandle = CurrentEquippedWeapon->OnReloadComplete.AddUFunction(this, FName("OnWeaponReloadComplete"));
		CurrentEquippedWeapon->EquipWeapon();
	}

	if (OnEquippedItemChanged.IsBound())
	{
		OnEquippedItemChanged.Broadcast(CurrentEquippedItem);
	}

	CurrentEquippedSlot = Slot;
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_EquipItemInSlot(CurrentEquippedSlot);
	}
}

void UCharacterEquipmentComponent::AttachCurrentItemToEquippedSocket()
{
	if (IsValid(CurrentEquippedItem))
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentEquippedItem->GetEquippedSocketName());
	}
}

void UCharacterEquipmentComponent::EquipAnimationFinished()
{
	bIsEquipping = false;
	AttachCurrentItemToEquippedSocket();
}

void UCharacterEquipmentComponent::UnEquipCurrentItem()
{
	if (IsValid(CurrentEquippedItem))
	{
		CurrentEquippedItem->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentEquippedItem->GetUnequippedSocketName());
		CurrentEquippedItem->UnEquip();
	}

	if (IsValid(CurrentEquippedWeapon))
	{
		CurrentEquippedWeapon->StopFire();
		CurrentEquippedWeapon->EndReload(false);
		CurrentEquippedWeapon->UnEquipWeapon();
		CurrentEquippedWeapon->OnAmmoChanged.Remove(OnCurrentWeaponAmmoChangeHandle);
		CurrentEquippedWeapon->OnReloadComplete.Remove(OnCurrentWeaponAmmoReloadedHandle);
	}

	PreviousEquippedSlot = CurrentEquippedSlot;
	CurrentEquippedSlot = EEquipmentSlots::None;
}

void UCharacterEquipmentComponent::EquipNextItem()
{
	bool bCanEquip = false;

	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 NextSlotIndex = NextItemsArraySlotIndex(CurrentSlotIndex);

	if (ItemsArray.Num() <= 1)
	{
		return;
	}

	for (NextSlotIndex = NextItemsArraySlotIndex(CurrentSlotIndex); NextSlotIndex != CurrentSlotIndex; NextSlotIndex = NextItemsArraySlotIndex(NextSlotIndex))
	{
		if (!IsValid(ItemsArray[NextSlotIndex]) || IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)NextSlotIndex))
		{
			continue;
		}

		bCanEquip = true;
		break;
	}

	if (bCanEquip)
	{
		EquipItemInSlot((EEquipmentSlots)NextSlotIndex);
	}

}

void UCharacterEquipmentComponent::EquipPrevItem()
{
	bool bCanEquip = false;

	uint32 CurrentSlotIndex = (uint32)CurrentEquippedSlot;
	uint32 PrevSlotIndex = PrevItemsArraySlotIndex(CurrentSlotIndex);

	if (ItemsArray.Num() <= 1)
	{
		return;
	}

	for (PrevSlotIndex = PrevItemsArraySlotIndex(CurrentSlotIndex); PrevSlotIndex != CurrentSlotIndex; PrevSlotIndex = PrevItemsArraySlotIndex(PrevSlotIndex))
	{
		if (!IsValid(ItemsArray[PrevSlotIndex]) || IgnoreSlotsWhileSwitching.Contains((EEquipmentSlots)PrevSlotIndex))
		{
			continue;
		}

		bCanEquip = true;
		break;
	}

	if (bCanEquip)
	{
		EquipItemInSlot((EEquipmentSlots)PrevSlotIndex);
	}
}

void UCharacterEquipmentComponent::Server_EquipItemInSlot_Implementation(EEquipmentSlots Slot)
{
	EquipItemInSlot(Slot);
}

uint32 UCharacterEquipmentComponent::NextItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if (CurrentSlotIndex == ItemsArray.Num() - 1)
	{
		return 0;
	}
	else
	{
		return CurrentSlotIndex + 1;
	}
}

uint32 UCharacterEquipmentComponent::PrevItemsArraySlotIndex(uint32 CurrentSlotIndex)
{
	if (CurrentSlotIndex == 0)
	{
		return ItemsArray.Num() - 1;
	}
	else
	{
		return CurrentSlotIndex - 1;
	}
}

void UCharacterEquipmentComponent::OnRep_ItemsArray()
{
	for (AEquipableItem* EquipableItem : ItemsArray)
	{
		if (IsValid(EquipableItem))
		{
			EquipableItem->UnEquip();
		}
	}
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	AmmunitionArray.AddZeroed((uint32)EAmmunitionType::MAX);
	for (const TPair<EAmmunitionType, int32>& AmmoPair : MaxAmmunitionAmount)
	{
		AddAmmunition(AmmoPair.Key, FMath::Max(AmmoPair.Value, 0));

		if (AmmoPair.Key == EAmmunitionType::FragGrenades)
		{
			OnThrowableAmmoChangedBroadcast(AmmoPair.Value);
		}
	}

	ItemsArray.AddZeroed((uint32)EEquipmentSlots::MAX);
	for (const TPair<EEquipmentSlots, TSubclassOf<AEquipableItem>>& ItemPair : ItemsLoadout)
	{
		if (!IsValid(ItemPair.Value))
		{
			continue;
		}

		AddEquipmentItemToSlot(ItemPair.Value, (int32)ItemPair.Key);
		if (ItemPair.Value->GetDefaultObject()->IsA<AThrowableItem>())
		{
			FillThrowablesPool(StaticCast<AThrowableItem*>(ItemPair.Value->GetDefaultObject())->GetProjectileClass());
		}
	}
}

void UCharacterEquipmentComponent::AutoEquip()
{
	if (AutoEquipItemInSlot != EEquipmentSlots::None)
	{
		EquipItemInSlot(AutoEquipItemInSlot);
	}
}

void UCharacterEquipmentComponent::OnCurrentWeaponAmmoChanged(int32 Ammo)
{
	if (OnCurrentWeaponAmmoChangedEvent.IsBound())
	{
		OnCurrentWeaponAmmoChangedEvent.Broadcast(Ammo, GetAvailableAmmunitionForCurrentWeapon());
	}
}

void UCharacterEquipmentComponent::OnWeaponReloadComplete(int32 Ammo)
{
	ReloadAmmoInCurrentWeapon();
}

void UCharacterEquipmentComponent::ReloadAmmoInCurrentWeapon(int32 NumberOfAmmo /*= 0*/, bool bCheckIsFull /*= false*/)
{
	int32 AvailableAmmunition = GetAvailableAmmunitionForCurrentWeapon();
	if (AvailableAmmunition <= 0)
	{
		return;
	}

	int32 CurrentAmmo = CurrentEquippedWeapon->GetAmmo();
	int32 AmmoToReload = CurrentEquippedWeapon->GetMaxAmmo() - CurrentAmmo;
	int32 ReloadedAmmo = FMath::Min(AvailableAmmunition, AmmoToReload);
	if (NumberOfAmmo > 0)
	{
		ReloadedAmmo = FMath::Min(ReloadedAmmo, NumberOfAmmo);
	}

	RemoveAmmunition(GetCurrentRangeWeapon()->GetAmmoType(), ReloadedAmmo);
	CurrentEquippedWeapon->SetAmmo(ReloadedAmmo + CurrentAmmo);
	if (GetOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		CurrentEquippedWeapon->Server_SetAmmo(CurrentAmmo);
	}

	if (bCheckIsFull)
	{
		AvailableAmmunition = AmmunitionArray[(uint32)GetCurrentRangeWeapon()->GetAmmoType()];
		bool bIsFullyLoaded = (CurrentEquippedWeapon->GetAmmo() == CurrentEquippedWeapon->GetMaxAmmo());
		if (AvailableAmmunition == 0 || bIsFullyLoaded)
		{
			CurrentEquippedWeapon->EndReload(true);
		}
	}
}

FThrowItemParams UCharacterEquipmentComponent::FillThrowItemParams()
{
	FVector PlayerViewPoint;
	FRotator PlayerViewRotation;

	if (CachedBaseCharacter->IsPlayerControlled())
	{
		APlayerController* Controller = CachedBaseCharacter->GetController<APlayerController>();
		Controller->GetPlayerViewPoint(PlayerViewPoint, PlayerViewRotation);
	}
	else
	{
		PlayerViewPoint = CachedBaseCharacter->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
		PlayerViewRotation = CachedBaseCharacter->GetBaseAimRotation();
	}

	FTransform PlayerViewTransform(PlayerViewRotation, PlayerViewPoint);

	FVector ViewDirection = PlayerViewRotation.RotateVector(FVector::ForwardVector);
	FVector ViewUpVector = PlayerViewRotation.RotateVector(FVector::UpVector);

	FVector LaunchDirection = ViewDirection + FMath::Tan(FMath::DegreesToRadians(CurrentThrowableItem->GetThrowAngle())) * ViewUpVector;
	FVector ThrowableSocketLocation = CachedBaseCharacter->GetMesh()->GetSocketLocation(SocketCharacterThrowable);
	FVector SocketInViewSpace = PlayerViewTransform.InverseTransformPosition(ThrowableSocketLocation);

	FVector SpawnLocation = PlayerViewPoint + ViewDirection * SocketInViewSpace.X;

	FThrowItemParams ThrowItemParams;
	ThrowItemParams.StartLocation = SpawnLocation;
	ThrowItemParams.Direction = LaunchDirection;
	ThrowItemParams.CurrentThrowableItemIndex = CurrentThrowableIndex;

	return ThrowItemParams;
}

void UCharacterEquipmentComponent::LaunchCurrentThrowableItem()
{
	if (CachedBaseCharacter->GetLocalRole() == ROLE_Authority)
	{
		if (!IsValid(CurrentThrowableItem))
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("UCharacterEquipmentComponent::LaunchCurrentThrowableItem item: %d by %d"), CurrentThrowableIndex, CachedBaseCharacter->GetLocalRole());

		int32& AvailableAmmunition = AmmunitionArray[(int32)(CurrentThrowableItem->GetAmmoType())];
		if (0 == AvailableAmmunition)
		{
			return;
		}

		FThrowItemParams ThrowItemParams = FillThrowItemParams();
		NetMulticast_ThrowItem(ThrowItemParams);
		UpdateThrowableIndex();

		AvailableAmmunition--;
		OnThrowableAmmoChangedBroadcast(AvailableAmmunition);

		bIsEquipping = false;
		EquipItemInSlot(PreviousEquippedSlot);
	}
	else
	{
		if (CachedBaseCharacter->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_LaunchCurrentThrowableItem();
		}
	}
}

void UCharacterEquipmentComponent::UpdateThrowableIndex()
{
	CurrentThrowableIndex++;
	if (CurrentThrowableIndex == ThrowablesPool.Num())
	{
		CurrentThrowableIndex = 0;
	}
}

void UCharacterEquipmentComponent::NetMulticast_ThrowItem_Implementation(const FThrowItemParams& ThrowParams)
{
	ThrowItem(ThrowParams);
}

void UCharacterEquipmentComponent::Server_LaunchCurrentThrowableItem_Implementation()
{
	LaunchCurrentThrowableItem();
}

void UCharacterEquipmentComponent::ChangeAmmoType()
{
	if (!IsValid(CurrentEquippedWeapon))
	{
		return;
	}

	CurrentEquippedWeapon->ChangeAmmoType();
}

void UCharacterEquipmentComponent::OnThrowableAmmoChangedBroadcast(int32 AvailableAmmunition)
{
	if (OnThrowableAmmoChanged.IsBound())
	{
		OnThrowableAmmoChanged.Broadcast(AvailableAmmunition);
	}
}

bool UCharacterEquipmentComponent::CanThrowItem() const
{
	AThrowableItem* ItemToThrow = Cast<AThrowableItem>(ItemsArray[(uint32)EEquipmentSlots::PrimaryItemSlot]);
	if (!ItemToThrow)
	{
		return false;
	}

	return AmmunitionArray[(int32)ItemToThrow->GetAmmoType()] > 0;
}

bool UCharacterEquipmentComponent::AddEquipmentItemToSlot(const TSubclassOf<AEquipableItem> EquipableItemClass, int32 SlotIndex)
{
	if (!IsValid(EquipableItemClass))
	{
		return false;
	}

	AEquipableItem* DefaultItemObject = EquipableItemClass->GetDefaultObject<AEquipableItem>();
	if (!DefaultItemObject->IsSlotCompatible((EEquipmentSlots)SlotIndex))
	{
		return false;
	}

	if (!IsValid(ItemsArray[SlotIndex]))
	{
		AEquipableItem* Item = GetWorld()->SpawnActor<AEquipableItem>(EquipableItemClass);
		Item->AttachToComponent(CachedBaseCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Item->GetUnequippedSocketName());
		Item->SetOwner(CachedBaseCharacter.Get());
		Item->UnEquip();
		ItemsArray[SlotIndex] = Item;
	}
	else if (DefaultItemObject->IsA<ARangeWeaponItem>())
	{
		ARangeWeaponItem* RangeWeaponItem = StaticCast<ARangeWeaponItem*>(DefaultItemObject);
		int32 AmmoSlotIndex = (int32)RangeWeaponItem->GetAmmoType();
		AmmunitionArray[SlotIndex] += RangeWeaponItem->GetMaxAmmo();
	}

	return true;
}

void UCharacterEquipmentComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	if ((uint32)(CurrentEquippedSlot) == SlotIndex)
	{
		UnEquipCurrentItem();
	}

	ItemsArray[SlotIndex]->Destroy();
	ItemsArray[SlotIndex] = nullptr;
}

void UCharacterEquipmentComponent::OpenViewEquipment(APlayerController* PlayerContoroller)
{
	if (!ViewWidget.IsValid())
	{
		CreateViewWidget(PlayerContoroller);
	}

	if (!ViewWidget->IsVisible())
	{
		ViewWidget->AddToViewport();
	}
}

void UCharacterEquipmentComponent::CloseViewEquipment()
{
	if (ViewWidget->IsVisible())
	{
		ViewWidget->RemoveFromParent();
	}
}

bool UCharacterEquipmentComponent::IsViewVisible() const
{
	bool bResult = false;
	if (ViewWidget.IsValid())
	{
		bResult = ViewWidget->IsVisible();
	}
	return bResult;
}

const TArray<AEquipableItem*>& UCharacterEquipmentComponent::GetItems() const
{
	return ItemsArray;
}

int32 UCharacterEquipmentComponent::GetAvailableAmmunitionForCurrentWeapon()
{
	check(IsValid(GetCurrentRangeWeapon()));
	return AmmunitionArray[(uint32)GetCurrentRangeWeapon()->GetAmmoType()];
}

void UCharacterEquipmentComponent::AddAmmunition(EAmmunitionType AmmunitionType, int32 Amount)
{
	AmmunitionArray[(uint32)AmmunitionType] += Amount;

	if (CachedBaseCharacter->IsPlayerControlled())
	{
		// update inventory
		TTuple<FName, FAmmoTableRowBase*> AmmoInfo = GCDataTableUtils::FindAmmoItemDataInfoByAmmunitionType(AmmunitionType);
		if (AmmoInfo.Key == NAME_None || AmmoInfo.Value == nullptr)
		{
			return;
		}

		CachedBaseCharacter->GetCharacterInventoryComponent_Mutable()->AddItems(AmmoInfo.Key, AmmoInfo.Value->InventoryItemType, Amount);

		// update current widget if needed
		ARangeWeaponItem* CurrentRangeWeapon = GetCurrentRangeWeapon();
		if (IsValid(CurrentRangeWeapon) && CurrentRangeWeapon->GetAmmoType() == AmmunitionType)
		{
			if (OnCurrentWeaponAmmoChangedEvent.IsBound())
			{
				OnCurrentWeaponAmmoChangedEvent.Broadcast(CurrentRangeWeapon->GetAmmo(), AmmunitionArray[(uint32)AmmunitionType]);
			}
		}
	}
}

void UCharacterEquipmentComponent::RemoveAmmunition(EAmmunitionType AmmunitionType, int32 Amount)
{
	AmmunitionArray[(uint32)AmmunitionType] -= Amount;

	if (CachedBaseCharacter->IsPlayerControlled())
	{
		// update inventory
		TTuple<FName, FAmmoTableRowBase*> AmmoInfo = GCDataTableUtils::FindAmmoItemDataInfoByAmmunitionType(AmmunitionType);
		if (AmmoInfo.Key == NAME_None || AmmoInfo.Value == nullptr)
		{
			return;
		}

		CachedBaseCharacter->GetCharacterInventoryComponent_Mutable()->RemoveItems(AmmoInfo.Key, AmmoInfo.Value->InventoryItemType, Amount);

		// update current widget if needed
		ARangeWeaponItem* CurrentRangeWeapon = GetCurrentRangeWeapon();
		if (IsValid(CurrentRangeWeapon) && CurrentRangeWeapon->GetAmmoType() == AmmunitionType)
		{
			if (OnCurrentWeaponAmmoChangedEvent.IsBound())
			{
				OnCurrentWeaponAmmoChangedEvent.Broadcast(CurrentRangeWeapon->GetAmmo(), AmmunitionArray[(uint32)AmmunitionType]);
			}
		}
	}
}

void UCharacterEquipmentComponent::OnRep_CurrentEquippedSlot(EEquipmentSlots CurrentEquippedSlot_Old)
{
	EquipItemInSlot(CurrentEquippedSlot);
}

void UCharacterEquipmentComponent::CreateViewWidget(APlayerController* PlayerController)
{
	check(IsValid(ViewWidgetClass));

	if (!IsValid(PlayerController))
	{
		return;
	}

	ViewWidget = CreateWidget<UEquipmentViewWidget>(PlayerController, ViewWidgetClass);
	ViewWidget->InitializeEquipmentWidget(this);
}

void UCharacterEquipmentComponent::FillThrowablesPool(TSubclassOf<class AGCProjectile> ProjectileClass)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	ThrowablesPool.Empty();
	ThrowablesPool.Reserve(ThrowablesPoolSize);

	for (int32 i = 0; i < ThrowablesPoolSize; ++i)
	{
		AGCProjectile* Projectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, ThrowablesPoolLocation, FRotator::ZeroRotator);
		if (IsValid(Projectile))
		{
			Projectile->SetOwner(GetOwner());
			Projectile->SetProjectileActive(false);
			ThrowablesPool.Add(Projectile);
		}
	}
}

void UCharacterEquipmentComponent::ThrowItem(FThrowItemParams CurrentThrowItemParams)
{
	//UE_LOG(LogTemp, Warning, TEXT("UCharacterEquipmentComponent::ThrowItem with index %d by: %d"), CurrentThrowItemParams.CurrentThrowableItemIndex, GetOwner()->GetLocalRole());

	CurrentThrowableItem->Throw(
		ThrowablesPool[CurrentThrowItemParams.CurrentThrowableItemIndex],
		CurrentThrowItemParams.StartLocation,
		CurrentThrowItemParams.Direction);
}
