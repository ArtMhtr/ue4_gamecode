// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "Controllers/GCPlayerController.h"
#include <Camera/CameraComponent.h>

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsFPMontagePlaying() && GCPlayerController.IsValid())
	{
		float BlendSpeed = 300.0f;
		FRotator TargetControlRotation = GCPlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.0f;
		TargetControlRotation = FMath::RInterpTo(GCPlayerController->GetControlRotation(), TargetControlRotation, DeltaSeconds, BlendSpeed);
		GCPlayerController->SetControlRotation(TargetControlRotation);
	}
}

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	FirstPersonMeshComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -86.0f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = 1;

	CameraComponent->bAutoActivate = false;

	SpringArmComponent->bAutoActivate = false;
	SpringArmComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	const AFPPlayerCharacter* DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	GCPlayerController = Cast<AGCPlayerController>(NewController);
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode /*= 0*/)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if (GetBaseCharacterMovementComponent()->IsOnLadder() || GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		OnCustomMoveModeStarted(GetBaseCharacterMovementComponent()->CustomMovementMode);
	}
	else
	{
		OnCustomMoveModeStopped();
	}
}

void AFPPlayerCharacter::OnCustomMoveModeStopped()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(false);
		bUseControllerRotationYaw = true;

		APlayerCameraManager* CameraManager = GCPlayerController->PlayerCameraManager;
		APlayerCameraManager* DefaultCameraManager = CameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();

		CameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
		CameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
		CameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
		CameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
	}
}

void AFPPlayerCharacter::OnCustomMoveModeStarted(uint8 CustomMovementMode)
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreCameraPitch(true);
		bUseControllerRotationYaw = false;

		APlayerCameraManager* CameraManager = GCPlayerController->PlayerCameraManager;
		float CurrentCharacterYawRotation = GetActorRotation().Yaw;
		if (CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
		{
			CameraManager->ViewPitchMin = LadderCameraMinPitch;
			CameraManager->ViewPitchMax = LadderCameraMaxPitch;
			CameraManager->ViewYawMin = CurrentCharacterYawRotation + LadderCameraMinYaw;
			CameraManager->ViewYawMax = CurrentCharacterYawRotation + LadderCameraMaxYaw;
		}
		else
		{
			CameraManager->ViewPitchMin = ZiplineCameraMinPitch;
			CameraManager->ViewPitchMax = ZiplineCameraMaxPitch;
			CameraManager->ViewYawMin = CurrentCharacterYawRotation + ZiplineCameraMinYaw;
			CameraManager->ViewYawMax = CurrentCharacterYawRotation + ZiplineCameraMaxYaw;
		}
	}
}

void AFPPlayerCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float StartTime)
{
	Super::OnMantle(MantlingSettings, StartTime);
	UAnimInstance* FPAnimInstanse = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstanse) && MantlingSettings.FPMantlingMontage)
	{
		if (GCPlayerController.IsValid())
		{
			GCPlayerController->SetIgnoreLookInput(true);
			GCPlayerController->SetIgnoreMoveInput(true);
		}
		float MontageDuration = FPAnimInstanse->Montage_Play(MantlingSettings.FPMantlingMontage, 1.0f, EMontagePlayReturnType::Duration, StartTime);
		GetWorld()->GetTimerManager().SetTimer(FPMontageTimer, this, &AFPPlayerCharacter::OnFPMontageTimerElapsed, MontageDuration, false);
	}
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	UAnimInstance* FPAnimInstanse = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(FPAnimInstanse) && FPAnimInstanse->IsAnyMontagePlaying();
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();
	if (IsFPMontagePlaying())
	{
		FRotator SocketRotation = FirstPersonMeshComponent->GetSocketRotation(SocketFPCamera);
		Result.Yaw = SocketRotation.Yaw;
		Result.Roll = SocketRotation.Roll;
		Result.Pitch += SocketRotation.Pitch;
	}

	return Result;
}

void AFPPlayerCharacter::OnFPMontageTimerElapsed()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreLookInput(false);
		GCPlayerController->SetIgnoreMoveInput(false);
	}
}

void AFPPlayerCharacter::OnStartHardLanding()
{
	UAnimInstance* FPAnimInstanse = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(FPAnimInstanse) && FPHardLandingMontage)
	{
		if (GCPlayerController.IsValid())
		{
			GCPlayerController->SetIgnoreLookInput(true);
			GCPlayerController->SetIgnoreMoveInput(true);
		}

		float MontageDuration = FPAnimInstanse->Montage_Play(FPHardLandingMontage, 1.0f, EMontagePlayReturnType::Duration);
		GetWorld()->GetTimerManager().SetTimer(HardLandingTimer, this, &AFPPlayerCharacter::OnHardLandingTimerElapsed, MontageDuration, false);
	}
}

void AFPPlayerCharacter::OnHardLandingTimerElapsed()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreLookInput(false);
		GCPlayerController->SetIgnoreMoveInput(false);
	}
}
