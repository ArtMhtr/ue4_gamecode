// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../Characters/GCBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../LedgeDetectorComponent.h"
#include "../../Actors/Interactive/Environment/Zipline.h"
#include "../../GameCodeTypes.h"
#include <Curves/CurveVector.h>
#include <Components/TimelineComponent.h>
#include "GCBaseCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_Ladder UMETA(DisplayName = "Ladder"),
	CMOVE_Zipline UMETA(DisplayName = "Zipline"),
	CMOVE_Wallrun UMETA(DisplayName = "Wallrun"),
	CMOVE_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};

UENUM(BlueprintType)
enum class EDetachFromZiplineMethod : uint8
{
	ReachingTheBottom = 0,
	JumpOff
};

struct FMantlingMovementParameters
{
	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	FVector InitialAnimationLocation = FVector::ZeroVector;

	float Duration = 1.0f;
	float StartTime = 0.0f;

	UCurveVector* MantlingCurve;

	// if component is moving, we should compute its shift to apply to target location later
	FVector LedgeInitialLocation;
	TWeakObjectPtr<UPrimitiveComponent> Ledge;
};

UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FSavedMove_GC;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	virtual void PhysicsRotation(float DeltaTime) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual float GetMaxSpeed() const override;

	/* Sprinting */
	FORCEINLINE bool IsSprinting() { return bIsSprinting; };
	void StartSprint();
	void StopSprint();

	/* Crouch */
	virtual void UnCrouch(bool bClientSimulation = false) override;

	/* Prone */
	FORCEINLINE virtual bool CanEverProne() const { return true; }
	bool IsProning() const { return GetBaseCharacterOwner()->bIsProned; }
	float GetProneCapsuleHalfHeight() const { return ProneCapsuleHalfHeight; }

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Character Movement: Prone")
	uint8 bWantsToProne : 1;

	/* Mantling */
	bool IsMantling() const;
	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();

	/* Ladder */
	bool IsOnLadder() const;
	void AttachToLadder(const class ALadder* Ladder);
	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	float GetLadderSpeedRation() const;
	float GetActorToCurrentLadderProjection(const FVector&) const;
	const ALadder* GetCurrentLadder() const { return CurrentLadder; }

	/* Zipline */
	void AttachToZipline(const class AZipline* Zipline);
	void DetachFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod = EDetachFromZiplineMethod::JumpOff);
	bool IsOnZipline() const;
	bool CanAttachToZipline(const class AZipline* Zipline);
	bool IsAtZiplineBottom(const class AZipline* Zipline, FVector Location);
	const AZipline* GetCurrentZipline() const { return CurrentZipline; }

	/* Wallrun */
	bool CanStartWallRun(FVector ImpactNormal, EWallRunSide WallSide);
	bool IsProperAngleForWallRunToAttach(FVector ImpactNormal);
	bool IsOnWallRun() const;
	bool IsSurfaceRunnable(const FVector& SurfaceNormal) const;
	void StartWallRun(EWallRunSide WallSide, FVector AttachToWallPoint, FVector WallNormal);
	void StopWallRun();
	FVector GetWallRunDirection(FVector SurfaceNormal, EWallRunSide WallRunSide) const;
	EWallRunSide GetWallRunSide(const FVector& SurfaceNormal) const;
	EWallRunSide GetCurrentWallRunSide() const;
	void JumpOffTheWall();

	/* Slide */
	bool IsSliding() const { return GetBaseCharacterOwner()->bIsSliding; }
	void StartSlide();
	void StopSlide();

protected:

	AGCBaseCharacter* GetBaseCharacterOwner() const;

	virtual void PhysCustom(float DeltaTime, int32 Iteration) override;

	void PhysWallRun(float DeltaTime, int32 Iteration);
	void PhysZipline(float DeltaTime, int32 Iteration);
	void PhysLadder(float DeltaTime, int32 Iteration);
	void PhysMantling(float DeltaTime, int32 Iteration);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ProneSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Crouch", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float CrouchingSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Stamina", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OutOfStaminaSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: General", meta = (ClampMin = "0", UIMin = "0"))
	float ProneCapsuleRadius = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: General", meta = (ClampMin = "0", UIMin = "0"))
	float ProneCapsuleHalfHeight = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Swimming", meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleRadius = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Swimming", meta = (ClampMin = "0", UIMin = "0"))
	float SwimmingCapsuleHalfHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = "0", UIMin = "0"))
	float ClimbingOnLadderMaxSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = "0", UIMin = "0"))
	float LadderToCharacterOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = "0", UIMin = "0"))
	float ClimbingOnLadderBreakingDeceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = "0", UIMin = "0"))
	float MaxLadderTopOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = "0", UIMin = "0"))
	float MinLadderBottomOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Ladder", meta = (ClampMin = "0", UIMin = "0"))
	float JumpOffFromLadderSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineMaxSpeed = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineBaseSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineMaxAcceleration = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineAcceleration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineToCharacterForwardOffset = 80.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineToCharacterDownOffset = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Zipline", meta = (ClampMin = "0", UIMin = "0"))
	float ZiplineBottomToCharacterMinPossibleDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float MaxWallRunTime = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float MaxWallRunSpeed = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float WallRunLineTraceDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float WallRunOffset = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float WallRunLauchNormalVelocity = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float JumpOffWallForwardVelocity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "0", UIMin = "0"))
	float JumpOffWallSideVelocity = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Wallrun", meta = (ClampMin = "-1", UIMin = "-1", ClampMax = "-0.001", UIMax = "-0.001"))
	float WallRunProperCosAngleToAttach = -0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Wallrun")
	UCurveFloat* WallRunCameraTiltCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Wallrun")
	UCurveFloat* WallRunVerticalVelocityCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (ClampMin = "0", UIMin = "0"))
	float SlideSpeed = 1400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Slide", meta = (ClampMin = "0", UIMin = "0"))
	float SlideCapsuleHalfHeight = 60.0f;

private:

	bool CanCapsuleFit(FVector CapsuleLocation, float CapsuleRadius, float CapsuleHalfHeight);

	/* States */
	bool bIsSprinting = false;

	/* Zipline */
	const AZipline* CurrentZipline = nullptr;

	/* Prone */
	bool CanProneInCurrentState() const;

	void Prone(bool bClientSimulation = false);
	void UnProne(bool bClientSimulation = false);

	/* Mantling */

	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle MantlingTimer;

	/* Ladder */
	const ALadder* CurrentLadder = nullptr;
	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;

	/* Wallrun */
	FORCEINLINE void BeginCameraTilt() { WallRunCameraTiltTimeline.Play(); }
	FORCEINLINE void StopCameraTilt() { WallRunCameraTiltTimeline.Reverse(); }

	UFUNCTION()
	void UpdateWallRunCameraTilt(float Value);

	FTimeline WallRunCameraTiltTimeline;
	FTimerHandle WallRunTimer;

	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	FVector CurrentWallRunVector = FVector::ZeroVector;
};

class FSavedMove_GC : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

public:
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

private:
	uint8 bSavedIsSprinting : 1;
	uint8 bSavedIsMantling : 1;
	uint8 bSavedIsPressingSlide : 1;
};

class FNetworkPredictionData_Client_Character_GC : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

public:
	FNetworkPredictionData_Client_Character_GC(const UCharacterMovementComponent& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override;
};
