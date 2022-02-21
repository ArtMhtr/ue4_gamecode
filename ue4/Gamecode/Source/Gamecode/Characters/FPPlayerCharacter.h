// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "FPPlayerCharacter.generated.h"

UCLASS()
class GAMECODE_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

protected:

	virtual void OnStartHardLanding() override;
	virtual void OnHardLandingTimerElapsed() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | FirstPerson")
	USkeletalMeshComponent* FirstPersonMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | FirstPerson")
	UCameraComponent* FirstPersonCameraComponent;

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float StartTime) override;
	virtual FRotator GetViewRotation() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Ladder | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float LadderCameraMinPitch  = -60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Ladder | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float LadderCameraMaxPitch = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Ladder | Yaw", meta = (UIMin = -90.0f, UIMax = 90.0f))
	float LadderCameraMinYaw = -60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Ladder | Yaw", meta = (UIMin = -90.0f, UIMax = 90.0f))
	float LadderCameraMaxYaw = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Zipline | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float ZiplineCameraMinPitch = -30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Zipline | Pitch", meta = (UIMin = -89.0f, UIMax = 89.0f))
	float ZiplineCameraMaxPitch = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Zipline | Yaw", meta = (UIMin = -90.0f, UIMax = 90.0f))
	float ZiplineCameraMinYaw = -60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | FirstPerson | Camera | Zipline | Yaw", meta = (UIMin = -90.0f, UIMax = 90.0f))
	float ZiplineCameraMaxYaw = 60.0f;

private:

	void OnCustomMoveModeStarted(uint8 CustomMovementMode);
	void OnCustomMoveModeStopped();

	FTimerHandle FPMontageTimer;
	void OnFPMontageTimerElapsed();
	bool IsFPMontagePlaying() const;

	TWeakObjectPtr<class AGCPlayerController> GCPlayerController = nullptr;
};
