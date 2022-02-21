// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include <GameFramework/SpringArmComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <Camera/CameraComponent.h>
#include <GameFramework/Character.h>

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	Team = ETeams::Player;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimFOVTimeline.TickTimeline(DeltaTime);	
}


void APlayerCharacter::UpdateFOV(float Alpha)
{
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}

	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
	ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (!IsValid(CameraManager) || !IsValid(RangeWeaponItem))
	{
		return;
	}

	float UpdatedFOV = FMath::Lerp(CameraManager->DefaultFOV, RangeWeaponItem->GetAimFOV(), Alpha);
	CameraManager->SetFOV(UpdatedFOV);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(AimFOVCurve))
	{
		FOnTimelineFloat FOVTimelineUpdate;
		FOVTimelineUpdate.BindUFunction(this, FName("UpdateFOV"));
		AimFOVTimeline.AddInterpFloat(AimFOVCurve, FOVTimelineUpdate);
	}
}

void APlayerCharacter::MoveForward(float Value)
{
	if ((GetCharacterMovement()->IsWalking() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = YawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
		//AddMovementInput(GetActorForwardVector(), Value);
	}
	
	ForwardAxisValue = Value;
}

void APlayerCharacter::MoveRight(float Value)
{
	if ((GetCharacterMovement()->IsWalking() || GetCharacterMovement()->IsFalling()) && !FMath::IsNearlyZero(Value))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);

		//AddMovementInput(GetActorRightVector(), Value);
	}
	RightAxisValue = Value;
}

void APlayerCharacter::LookUp(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (IsValid(RangeWeaponItem) && IsAiming())
		{
			Value *= RangeWeaponItem->GetAimLookUpModifier();
		}
		AddControllerPitchInput(Value);
	}
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (IsValid(RangeWeaponItem) && IsAiming())
		{
			Value *= RangeWeaponItem->GetAimLookUpModifier();
		}
		AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	}
}

void APlayerCharacter::SwimForward(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FRotator PitchYawRotator(GetControlRotation().Pitch, GetControlRotation().Yaw, 0.0f);
		FVector ForwardVector = PitchYawRotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);
	}
}

void APlayerCharacter::SwimRight(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		FRotator YawRotator(0.0f, GetControlRotation().Yaw, 0.0f);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);
	}
}

void APlayerCharacter::SwimUp(float Value)
{
	if (GetCharacterMovement()->IsSwimming() && !FMath::IsNearlyZero(Value))
	{
		AddMovementInput(FVector::UpVector, Value);
	}

}

void APlayerCharacter::OnStartSlide(float HalfHeightAdjust)
{
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndSlide(float HalfHeightAdjust)
{
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnStartAimingInternal()
{
	Super::OnStartAimingInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}
	
	if (IsValid(AimFOVCurve))
	{
		ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		AimFOVTimeline.SetPlayRate(AimFOVTimeline.GetTimelineLength() / RangeWeaponItem->GetChangeAimFOVDuration());
		AimFOVTimeline.Play();
	}
	else
	{
		// if timer is not set - change FOV instantly
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		if (IsValid(CameraManager))
		{
			ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
			CameraManager->SetFOV(RangeWeaponItem->GetAimFOV());
		}
	}
}

void APlayerCharacter::OnStopAimingInternal()
{
	Super::OnStopAimingInternal();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (IsValid(AimFOVCurve))
	{
		AimFOVTimeline.Reverse();
	}
	else
	{
		// if timer is not set - change FOV instantly
		APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
		if (IsValid(CameraManager))
		{
			ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
			CameraManager->UnlockFOV();
		}
	}
}

void APlayerCharacter::OnStartProne(float HalfHeightAdjust)
{
	SpringArmComponent->TargetOffset += FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnEndProne(float HalfHeightAdjust)
{
	SpringArmComponent->TargetOffset -= FVector(0.0f, 0.0f, HalfHeightAdjust);
}

void APlayerCharacter::OnJumped_Implementation()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

bool APlayerCharacter::CanJumpInternal_Implementation() const
{
	bool Result;

	if (IsOutOfStamina() || bIsProned || GetBaseCharacterMovementComponent()->IsMantling() || GetBaseCharacterMovementComponent()->IsSliding() ||
		IsHardLanding())
	{
		Result = false;
	}
	else
	{
		Result = bIsCrouched || Super::CanJumpInternal_Implementation();
	}

	return Result;
}

void APlayerCharacter::Turn(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (IsValid(RangeWeaponItem) && IsAiming())
		{
			Value *= RangeWeaponItem->GetAimTurnModifier();
		}
		AddControllerYawInput(Value);
	}
}

void APlayerCharacter::TurnAtRate(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		ARangeWeaponItem* RangeWeaponItem = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (IsValid(RangeWeaponItem) && IsAiming())
		{
			Value *= RangeWeaponItem->GetAimTurnModifier();
		}
		AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	}
}
