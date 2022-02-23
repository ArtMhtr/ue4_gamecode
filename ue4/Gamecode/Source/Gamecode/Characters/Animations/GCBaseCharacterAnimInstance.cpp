// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterAnimInstance.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include <GameFramework/CharacterMovementComponent.h>

void UGCBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGCBaseCharacter>(), TEXT("UGCBaseCharacterAnimInstance::NativeBeginPlay() can be used only with AGCBaseCharacter"));
	CachedBaseCharacter = StaticCast<AGCBaseCharacter*>(TryGetPawnOwner());
}

void UGCBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedBaseCharacter.IsValid())
	{
		return;
	}

	UGCBaseCharacterMovementComponent* CharacterMovement = CachedBaseCharacter->GetBaseCharacterMovementComponent();
	Speed = CharacterMovement->Velocity.Size();
	bIsFalling = CharacterMovement->IsFalling();
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprinting = CharacterMovement->IsSprinting();
	bIsProning = CharacterMovement->IsProning();
	bIsSwimming = CharacterMovement->IsSwimming();
	bIsOnZipline = CharacterMovement->IsOnZipline();
	bIsOnLadder = CharacterMovement->IsOnLadder();
	bIsOnWallRun = CharacterMovement->IsOnWallRun();
	bIsStrafing = !CharacterMovement->bOrientRotationToMovement;
	Direction = CalculateDirection(CharacterMovement->Velocity, CachedBaseCharacter->GetActorRotation());
	AimRotation = CachedBaseCharacter->GetAimOffset();
	bIsAiming = CachedBaseCharacter->IsAiming();

	if (bIsOnLadder)
	{
		LadderSpeedRatio = CharacterMovement->GetLadderSpeedRation();
	}

	if (bIsOnWallRun)
	{
		WallRunSide = CharacterMovement->GetCurrentWallRunSide();
	}

	RightFootEffectorLocation = FVector(CachedBaseCharacter->GetIKRightFootOffset(), 0.0f, 0.0f);
	LeftFootEffectorLocation = FVector(-CachedBaseCharacter->GetIKLeftFootOffset(), 0.0f, 0.0f);
	bIsOutOfStamina = CachedBaseCharacter->IsOutOfStamina();

	const UCharacterEquipmentComponent* EquipmentComponent = CachedBaseCharacter->GetCharacterEquipmentComponent();
	CurrentEquppedItemType = EquipmentComponent->GetCurrentEquippedItemType();
	ARangeWeaponItem* CurrentRangeWeapon = EquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		ForeGripSocketTransform = CurrentRangeWeapon->GetForeGripTransform();
		BowStringSocketTransform = CurrentRangeWeapon->GetBowStringTransform();
	}
}

