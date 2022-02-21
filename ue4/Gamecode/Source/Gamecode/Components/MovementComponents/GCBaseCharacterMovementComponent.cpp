// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/LedgeDetectorComponent.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include <Kismet/KismetMathLibrary.h>
#include "GameCodeTypes.h"
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>
#include "Characters/PlayerCharacter.h"
#include <Net/UnrealNetwork.h>

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;

			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}

		return;
	}

	if (IsOnLadder())
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	AGCBaseCharacter* GCBaseCharacter = GetBaseCharacterOwner();

	if (bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if (IsProning())
	{
		Result = ProneSpeed;
	}
	else if (IsCrouching())
	{
		Result = CrouchingSpeed;
	}
	else if (IsOnLadder())
	{
		Result = ClimbingOnLadderMaxSpeed;
	}
	else if (IsOnZipline())
	{
		Result = ZiplineMaxSpeed;
	}
	else if (IsOnWallRun())
	{
		Result = MaxWallRunSpeed;
	}
	else if (GCBaseCharacter->IsAiming())
	{
		Result = GCBaseCharacter->GetAimingMovementSpeed();
	}
	else if (GCBaseCharacter->IsOutOfStamina())
	{
		Result = OutOfStaminaSpeed;
	}

	return Result;
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

void UGCBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)(ECustomMovementMode::CMOVE_Mantling));
	Velocity = FVector::ZeroVector;

	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UGCBaseCharacterMovementComponent::StartMantle() only a GCBaseCharacter support mantling"));
	if (GetBaseCharacterOwner()->bIsCrouched)
	{
		GetBaseCharacterOwner()->UnCrouch();
	}
}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	GetBaseCharacterOwner()->bIsMantling = false;
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CMOVE_Mantling);
}

void UGCBaseCharacterMovementComponent::AttachToLadder(const class ALadder* Ladder)
{
	CurrentLadder = Ladder;

	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorForwardVector();
	float Projection = GetActorToCurrentLadderProjection(GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector + LadderToCharacterOffset * LadderForwardVector;
	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}
	GetOwner()->SetActorRotation(TargetOrientationRotation);
	GetOwner()->SetActorLocation(NewCharacterLocation);
	Velocity = FVector::ZeroVector;

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Ladder);
}

float UGCBaseCharacterMovementComponent::GetActorToCurrentLadderProjection(const FVector& Location) const
{
	checkf(IsValid(CurrentLadder), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToCurrentLadderProjection cannot be invoked, current ladder in null"));
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderToCharacterDistance = Location - CurrentLadder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterDistance);
}

bool UGCBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CMOVE_Zipline);
}

void UGCBaseCharacterMovementComponent::AttachToZipline(const class AZipline* Zipline)
{
	if (!CanAttachToZipline(Zipline))
	{
		return;
	}

 	CurrentZipline = Zipline;

	FVector CableNormalVector = CurrentZipline->GetCableNormalVector();
	FVector TopZiplineLocation = CurrentZipline->GetActorLocation() + CurrentZipline->GetZiplineTopPillarRelativeLocation() + FVector::UpVector * 0.5f * CurrentZipline->GetZiplineTopPillarHeight();
	FVector ClosestPointOnZipline = UKismetMathLibrary::FindClosestPointOnLine(GetActorLocation(), TopZiplineLocation, CableNormalVector);

	FVector NewCharacterLocation = ClosestPointOnZipline;
	NewCharacterLocation += CableNormalVector * ZiplineToCharacterForwardOffset;
	NewCharacterLocation += FVector::DownVector * ZiplineToCharacterDownOffset;

	GetOwner()->SetActorRotation(CableNormalVector.ToOrientationRotator());
	GetOwner()->SetActorLocation(NewCharacterLocation);

	Velocity = CableNormalVector * ZiplineBaseSpeed;
	ZiplineAcceleration = ZiplineMaxAcceleration * FVector::DotProduct(FVector::DownVector, CableNormalVector);
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Zipline);
}

bool UGCBaseCharacterMovementComponent::IsAtZiplineBottom(const class AZipline* Zipline, FVector Location)
{
	FVector ZiplineBottomLocation = Zipline->GetActorLocation() + Zipline->GetZiplineBottomPillarRelativeLocation() + FVector::UpVector * 0.5f * Zipline->GetZiplineBottomPillarHeight();
	float ZiplineBottomToCharacterSquaredDistance = (Location - ZiplineBottomLocation).SizeSquared();
	return (ZiplineBottomToCharacterSquaredDistance < FMath::Square(ZiplineBottomToCharacterMinPossibleDistance));
}

FVector UGCBaseCharacterMovementComponent::GetWallRunDirection(FVector HitNormal, EWallRunSide WallRunSide) const
{
	if (WallRunSide == EWallRunSide::Left)
	{
		return FVector::CrossProduct(HitNormal, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		return FVector::CrossProduct(FVector::UpVector, HitNormal).GetSafeNormal();
	}
}

bool UGCBaseCharacterMovementComponent::IsProperAngleForWallRunToAttach(FVector ImpactNormal)
{
	AGCBaseCharacter* Character = GetBaseCharacterOwner();
	if (IsValid(Character))
	{
		float XYCosAngle = Character->GetActorForwardVector().CosineAngle2D(ImpactNormal);
		if (XYCosAngle < 0 && XYCosAngle > WallRunProperCosAngleToAttach)
		{
			return true;
		}
	}
	return false;
}

bool UGCBaseCharacterMovementComponent::CanStartWallRun(FVector ImpactNormal, EWallRunSide WallSide)
{
	return !IsOnWallRun() && IsFalling() && IsProperAngleForWallRunToAttach(ImpactNormal) && IsSurfaceRunnable(ImpactNormal) && GetBaseCharacterOwner()->areRequiredKeysDown(WallSide);
}

FNetworkPredictionData_Client* UGCBaseCharacterMovementComponent::GetPredictionData_Client() const 
{
	if (ClientPredictionData == nullptr)
	{
		UGCBaseCharacterMovementComponent* MutableThis = const_cast<UGCBaseCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_GC(*this);
	}
	return ClientPredictionData;
}

void UGCBaseCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bIsSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0);

	bool bWasMantling = GetBaseCharacterOwner()->bIsMantling;
	bool bIsManting = (Flags & FSavedMove_Character::FLAG_Custom_1);

	bool bWasSliding = GetBaseCharacterOwner()->bIsSliding;
	bool bIsRequestedToSlide = (Flags & FSavedMove_Character::FLAG_Custom_2);

	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_Authority)
	{
		if (!bWasMantling && bIsManting)
		{
			GetBaseCharacterOwner()->Mantle(true);
		}

		if (!bWasSliding && bIsRequestedToSlide)
		{
			GetBaseCharacterOwner()->Slide();
		}
	}
}

void UGCBaseCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	WallRunCameraTiltTimeline.TickTimeline(DeltaTime);
}

void UGCBaseCharacterMovementComponent::OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	EWallRunSide WallSide = GetWallRunSide(Hit.ImpactNormal);
	if (!CanStartWallRun(Hit.ImpactNormal, WallSide))
	{
		return;
	}

	FVector AttachToWallPoint = Hit.ImpactPoint + Hit.ImpactNormal.GetSafeNormal() * WallRunOffset;
	StartWallRun(WallSide, AttachToWallPoint, Hit.ImpactNormal);
}

EWallRunSide UGCBaseCharacterMovementComponent::GetWallRunSide(const FVector& HitNormal) const
{
	AGCBaseCharacter* Character = GetBaseCharacterOwner();
	return (FVector::DotProduct(HitNormal, Character->GetActorRightVector()) > 0 ? EWallRunSide::Left : EWallRunSide::Right);
}

bool UGCBaseCharacterMovementComponent::IsOnWallRun() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CMOVE_Wallrun);
}

EWallRunSide UGCBaseCharacterMovementComponent::GetCurrentWallRunSide() const
{
	EWallRunSide WallRunSide = EWallRunSide::None;
	if (IsOnWallRun())
	{
		WallRunSide = CurrentWallRunSide;
	}

	return WallRunSide;
}

void UGCBaseCharacterMovementComponent::JumpOffTheWall()
{
	FVector JumpDirection = FVector::ZeroVector;
	if (CurrentWallRunSide == EWallRunSide::Left)
	{
		JumpDirection = FVector::CrossProduct(FVector::UpVector, CurrentWallRunVector).GetSafeNormal();
	}
	else
	{
		JumpDirection = FVector::CrossProduct(CurrentWallRunVector, FVector::UpVector).GetSafeNormal();
	}

	JumpDirection *= JumpOffWallSideVelocity;
	JumpDirection += CurrentWallRunVector * JumpOffWallForwardVelocity;
	JumpDirection += FVector::UpVector * JumpZVelocity;

	StopWallRun();
	GetBaseCharacterOwner()->SetActorRotation(JumpDirection.ToOrientationRotator());
	Launch(JumpDirection);
}

void UGCBaseCharacterMovementComponent::StartSlide()
{
	AGCBaseCharacter* Character = GetBaseCharacterOwner();
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	if (IsValid(Character) && IsValid(DefaultCharacter) && IsValid(Character->GetSlidingMontage()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Start Slide by %d"), Character->GetLocalRole());

		const float DefaultUnscaledHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		const float DefaultUnscaledRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
		const float HalfHeightAdjust = DefaultUnscaledHalfHeight - SlideCapsuleHalfHeight;

		Character->GetCapsuleComponent()->SetCapsuleSize(DefaultUnscaledRadius, SlideCapsuleHalfHeight);
		Character->GetCapsuleComponent()->SetRelativeLocation(Character->GetCapsuleComponent()->GetRelativeLocation() - HalfHeightAdjust * FVector::UpVector);
		Character->OnCapsuleSizeAdjustDown(HalfHeightAdjust);

		if (Character->IsA<APlayerCharacter>())
		{
			StaticCast<APlayerCharacter*>(Character)->OnStartSlide(HalfHeightAdjust);
		}

		Character->bIsSliding = true;
		if (Character->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Character->Server_StartSlide();
		}
		Character->PlayAnimMontage(Character->GetSlidingMontage());
	}
}

void UGCBaseCharacterMovementComponent::StopSlide()
{
	AGCBaseCharacter* Character = GetBaseCharacterOwner();
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	UE_LOG(LogTemp, Warning, TEXT("Stop Slide by %d"), Character->GetLocalRole());

	if (IsValid(Character) && IsValid(DefaultCharacter))
	{
		const float DefaultUnscaledHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
		const float DefaultUnscaledRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
		const float HalfHeightAdjust = DefaultUnscaledHalfHeight - SlideCapsuleHalfHeight;

		// if player won't fit standing at current location
		FVector NewCapsuleLocation = Character->GetActorLocation() - SlideCapsuleHalfHeight + DefaultUnscaledHalfHeight;
		if (!CanCapsuleFit(NewCapsuleLocation, DefaultUnscaledRadius, DefaultUnscaledHalfHeight))
		{
			Character->StopAnimMontage(Character->GetSlidingMontage());
			Character->Crouch();
		}

		const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
		Character->GetCapsuleComponent()->SetCapsuleSize(DefaultUnscaledRadius, DefaultUnscaledHalfHeight, true);
		Character->GetCapsuleComponent()->SetRelativeLocation(PawnLocation + HalfHeightAdjust * FVector::UpVector);
		Character->OnCapsuleSizeAdjustUp(HalfHeightAdjust);

		if (Character->IsA<APlayerCharacter>())
		{
			StaticCast<APlayerCharacter*>(Character)->OnEndSlide(HalfHeightAdjust);
		}

		Character->bIsSliding = false;
		if (Character->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Character->Server_StopSlide();
		}
	}
}

bool UGCBaseCharacterMovementComponent::IsSurfaceRunnable(const FVector& SurfaceNormal) const
{
	return (!(SurfaceNormal.Z > GetWalkableFloorZ()) && !(SurfaceNormal.Z < -0.005f));
}

void UGCBaseCharacterMovementComponent::StartWallRun(EWallRunSide WallSide, FVector AttachToWallPoint, FVector WallNormal)
{
	// we cannot hit the same wall side twice
	if (WallSide == CurrentWallRunSide)
	{
		return;
	}

	if (IsValid(WallRunCameraTiltCurve))
	{
		FOnTimelineFloat TimelineCallback;
		TimelineCallback.BindUFunction(this, FName("UpdateWallRunCameraTilt"));
		WallRunCameraTiltTimeline.AddInterpFloat(WallRunCameraTiltCurve, TimelineCallback);
	}

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Wallrun);
	SetPlaneConstraintNormal(FVector::ZeroVector);

	CurrentWallRunSide = WallSide;
	CurrentWallRunVector = GetWallRunDirection(WallNormal, WallSide);

	AGCBaseCharacter* BaseCharacter = GetBaseCharacterOwner();
	BaseCharacter->SetActorRotation(CurrentWallRunVector.ToOrientationRotator());
	BaseCharacter->SetActorLocation(AttachToWallPoint);
	BeginCameraTilt();
}

void UGCBaseCharacterMovementComponent::StopWallRun()
{
	SetMovementMode(MOVE_Falling);
	CurrentWallRunVector = FVector::ZeroVector;
	SetPlaneConstraintNormal(FVector::ZeroVector);
	StopCameraTilt();
}

bool UGCBaseCharacterMovementComponent::CanAttachToZipline(const class AZipline* Zipline)
{
	return (MovementMode == MOVE_Falling || MovementMode == MOVE_Walking) && !IsAtZiplineBottom(Zipline, GetActorLocation()) && !IsSliding();
}

void UGCBaseCharacterMovementComponent::DetachFromZipline(EDetachFromZiplineMethod DetachFromZiplineMethod /*= Fall*/)
{
	switch (DetachFromZiplineMethod)
	{
	case EDetachFromZiplineMethod::JumpOff:
	{
		FVector JumpDirection = CurrentZipline->GetCableNormalVector();
		SetMovementMode(MOVE_Falling);

		FVector JumpVelocity = JumpDirection * Velocity.Size();

		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;

		Launch(JumpVelocity);
		break;
	}
	case EDetachFromZiplineMethod::ReachingTheBottom:
	default:
		SetMovementMode(MOVE_Walking);
		break;
	}
}

void UGCBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod /*= Fall*/)
{
	switch (DetachFromLadderMethod)
	{
	case EDetachFromLadderMethod::Fall:
	default:
		SetMovementMode(MOVE_Falling);
		break;
	case EDetachFromLadderMethod::JumpOff:
	{

		FVector JumpDirection = CurrentLadder->GetActorForwardVector();
		SetMovementMode(MOVE_Falling);

		FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;

		ForceTargetRotation = JumpDirection.ToOrientationRotator();
		bForceRotation = true;

		Launch(JumpVelocity);
		break;
	}
	case EDetachFromLadderMethod::ReachingTheBottom:
		SetMovementMode(MOVE_Walking);
		break;
	case EDetachFromLadderMethod::ReachingTheTop:
		GetBaseCharacterOwner()->Mantle(true);
		break;
	}
}

bool UGCBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMovementMode::CMOVE_Ladder);
}

float UGCBaseCharacterMovementComponent::GetLadderSpeedRation() const
{
	checkf(IsValid(CurrentLadder), TEXT("UGCBaseCharacterMovementComponent::GetLadderSpeedRation cannot be invoked, current ladder in null"));
	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

AGCBaseCharacter* UGCBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AGCBaseCharacter*>(CharacterOwner);
}

void UGCBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iteration)
{
	if (GetBaseCharacterOwner()->GetLocalRole() == ROLE_SimulatedProxy)
	{
		return;
	}
	
	switch (CustomMovementMode)
	{
	case (uint8)ECustomMovementMode::CMOVE_Mantling:
		PhysMantling(DeltaTime, Iteration);
		break;
	case (uint8)ECustomMovementMode::CMOVE_Ladder:
		PhysLadder(DeltaTime, Iteration);
		break;
	case (uint8)ECustomMovementMode::CMOVE_Zipline:
		PhysZipline(DeltaTime, Iteration);
		break;
	case (uint8)ECustomMovementMode::CMOVE_Wallrun:
		PhysWallRun(DeltaTime, Iteration);
		break;
	default:
		break;
	}

	Super::PhysCustom(DeltaTime, Iteration);
}


void UGCBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iteration)
{
	AGCBaseCharacter* GCBaseCharacter = GetBaseCharacterOwner();
	if (!IsValid(GCBaseCharacter))
	{
		return;
	}

	if (!GCBaseCharacter->areRequiredKeysDown(CurrentWallRunSide))
	{
		StopWallRun();
		return;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GCBaseCharacter);

	FVector LineTraceDirection = (CurrentWallRunSide == EWallRunSide::Right ? GCBaseCharacter->GetActorRightVector() : -GCBaseCharacter->GetActorRightVector());
	FHitResult HitResult;
	if (!GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), GetActorLocation() + WallRunLineTraceDistance * LineTraceDirection,
		ECC_WallRunnable, QueryParams))
	{
		StopWallRun();
		return;
	}

	CurrentWallRunVector = GetWallRunDirection(HitResult.ImpactNormal, CurrentWallRunSide);
	Velocity = GetMaxSpeed() * CurrentWallRunVector;

	if (IsValid(WallRunVerticalVelocityCurve))
	{
		float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(WallRunTimer);
		float CurveRatio = ElapsedTime / MaxWallRunTime;
		float VerticalVelocity = WallRunVerticalVelocityCurve->GetFloatValue(CurveRatio);
		Velocity += VerticalVelocity * FVector::UpVector;
	}

	SafeMoveUpdatedComponent(Velocity * DeltaTime, Velocity.ToOrientationRotator(), true, HitResult);
}

void UGCBaseCharacterMovementComponent::UpdateWallRunCameraTilt(float Value)
{
	AGCBaseCharacter* Character = GetBaseCharacterOwner();
	FRotator CurrentControlRotation = Character->GetControlRotation();
	CurrentControlRotation.Roll = CurrentWallRunSide == EWallRunSide::Left ? Value : -Value;
	Character->GetController()->SetControlRotation(CurrentControlRotation);
}

void UGCBaseCharacterMovementComponent::PhysZipline(float DeltaTime, int32 Iteration)
{
	const AZipline* Zipline = GetCurrentZipline();
	if (Velocity.Size() < ZiplineMaxSpeed)
	{
		Velocity += Zipline->GetCableNormalVector() * ZiplineAcceleration;
	}

	FVector Delta = Velocity * DeltaTime;
	FVector NewPosition = GetActorLocation() + Delta;

	FVector ZiplineBottomLocation = Zipline->GetActorLocation() + Zipline->GetZiplineBottomPillarRelativeLocation() + FVector::UpVector * 0.5f * CurrentZipline->GetZiplineBottomPillarHeight();
	float ZiplineBottomToCharacterSquaredDistance = (NewPosition - ZiplineBottomLocation).SizeSquared();

	if (ZiplineBottomToCharacterSquaredDistance < FMath::Square(ZiplineBottomToCharacterMinPossibleDistance))
	{
		DetachFromZipline(EDetachFromZiplineMethod::ReachingTheBottom);
	}

	FHitResult HitResult;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, HitResult);
}

void UGCBaseCharacterMovementComponent::PhysLadder(float DeltaTime, int32 Iteration)
{
	CalcVelocity(DeltaTime, 1, false, ClimbingOnLadderBreakingDeceleration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult HitResult;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, HitResult);
		return;
	}

	FVector NewPosition = GetActorLocation() + Delta;
	float NewPosProjection = GetActorToCurrentLadderProjection(NewPosition);

	if (NewPosProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
	}
	else if (NewPosProjection > CurrentLadder->GetLadderHeight() - MaxLadderTopOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
		return;
	}

	FHitResult HitResult;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, HitResult);
}

void UGCBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 Iteration)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;

	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);
	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialLocation, CurrentMantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialLocation.Z, CurrentMantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);

	FVector Shift = CurrentMantlingParameters.Ledge->GetComponentLocation() - CurrentMantlingParameters.LedgeInitialLocation;

	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, CurrentMantlingParameters.TargetLocation + Shift, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialRotation, CurrentMantlingParameters.TargetRotation, PositionAlpha);

	FVector Delta = NewLocation - GetActorLocation();
	Velocity = Delta / DeltaTime;
	FHitResult HitResult;

	SafeMoveUpdatedComponent(Delta, NewRotation, false, HitResult);
}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (IsSprinting())
	{
		GetBaseCharacterOwner()->StopSprint();
	}

	if (MovementMode == MOVE_Swimming)
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
			DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);
	}

	if (MovementMode == MOVE_Walking)
	{
		// we can forget about previous wall side only when touching the ground
		CurrentWallRunSide = EWallRunSide::None;
	}

	if (PreviousMovementMode == MOVE_Custom)
	{
		if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Ladder)
		{
			CurrentLadder = nullptr;
		}
		else if (PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_Zipline)
		{
			CurrentZipline = nullptr;
		}
	}

	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
			GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UGCBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
			break;
		case (uint8)ECustomMovementMode::CMOVE_Wallrun:
			GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &UGCBaseCharacterMovementComponent::StopWallRun, MaxWallRunTime, false);
			break;
		case (uint8)ECustomMovementMode::CMOVE_None:
		case (uint8)ECustomMovementMode::CMOVE_Max:
		default:
			break;
		}
	}
}

bool UGCBaseCharacterMovementComponent::CanProneInCurrentState() const
{
	if (!CanEverProne())
	{
		return false;
	}
	return (IsFalling() || IsMovingOnGround()) && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

bool UGCBaseCharacterMovementComponent::CanCapsuleFit(FVector Location, float RequiredCapsuleRadius, float RequiredCapsuleHalfHeight)
{
	bool Result = false;
	if (HasValidData())
	{
		AGCBaseCharacter* GCBaseCharacter = GetBaseCharacterOwner();
		if (IsValid(GCBaseCharacter))
		{
			// Compensate for the difference between current capsule size and crouch size
			FCollisionShape CapsuleShape;
			const float HeightSweepInflation = KINDA_SMALL_NUMBER * 10.0f;
			CapsuleShape.SetCapsule(RequiredCapsuleRadius, RequiredCapsuleHalfHeight + HeightSweepInflation);

			// Expand in place
			FCollisionQueryParams CapsuleParams;
			CapsuleParams.AddIgnoredActor(GCBaseCharacter);
			CapsuleParams.bIgnoreTouches = true;

			Result = !GetWorld()->OverlapBlockingTestByChannel(Location + HeightSweepInflation, FQuat::Identity, ECollisionChannel::ECC_Pawn, CapsuleShape, CapsuleParams);
		}
	}

	return Result;
}

void UGCBaseCharacterMovementComponent::UnProne(bool bClientSimulation)
{
	if (HasValidData())
	{
		AGCBaseCharacter* GCBaseCharacter = GetBaseCharacterOwner();
		ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

		if (IsValid(GCBaseCharacter) && IsValid(DefaultCharacter))
		{
			GCBaseCharacter->CachedPreviousCharacterState = EGCBaseCharacterCachedPrevState::Prone;

			float CrouchedCapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
			FVector NewCapsuleLocation = GCBaseCharacter->GetActorLocation() - GetProneCapsuleHalfHeight() + CrouchedHalfHeight;

			if (CanCapsuleFit(NewCapsuleLocation, CrouchedCapsuleRadius, CrouchedHalfHeight))
			{
				GCBaseCharacter->bIsProned = false;
				GCBaseCharacter->bIsCrouched = true;

				const float CapsuleHeightAdjust = CrouchedHalfHeight - ProneCapsuleHalfHeight;

				CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(CrouchedCapsuleRadius, CrouchedHalfHeight);
				CharacterOwner->GetCapsuleComponent()->SetRelativeLocation(CharacterOwner->GetCapsuleComponent()->GetRelativeLocation() + CapsuleHeightAdjust * FVector::UpVector);
				GCBaseCharacter->OnCapsuleSizeAdjustUp(CapsuleHeightAdjust);

				if (GCBaseCharacter->IsA<APlayerCharacter>())
				{
					StaticCast<APlayerCharacter*>(GCBaseCharacter)->OnEndProne(CapsuleHeightAdjust);
				}
			}
		}
	}
}

void UGCBaseCharacterMovementComponent::Prone(bool bClientSimulation)
{
	if (HasValidData())
	{
		AGCBaseCharacter* GCBaseCharacter = GetBaseCharacterOwner();
		if (GCBaseCharacter)
		{
			if (CanProneInCurrentState())
			{
				GCBaseCharacter->bIsProned = true;
				GCBaseCharacter->bIsCrouched = false;

				const float CapsuleHeightAdjust = CrouchedHalfHeight - ProneCapsuleHalfHeight;

				CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(ProneCapsuleRadius, GetProneCapsuleHalfHeight());
				CharacterOwner->GetCapsuleComponent()->SetRelativeLocation(CharacterOwner->GetCapsuleComponent()->GetRelativeLocation() - CapsuleHeightAdjust * FVector::UpVector);
				GCBaseCharacter->OnCapsuleSizeAdjustDown(CapsuleHeightAdjust);
				
				if (GCBaseCharacter->IsA<APlayerCharacter>())
				{
					StaticCast<APlayerCharacter*>(GCBaseCharacter)->OnStartProne(CapsuleHeightAdjust);
				}
			}
		}
	}
}

void UGCBaseCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
	GetBaseCharacterOwner()->CachedPreviousCharacterState = EGCBaseCharacterCachedPrevState::Stand;
}

void UGCBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Proxies get replicated crouch and prone states.
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		const bool bIsCrouching = IsCrouching();
		const bool bIsProning = IsProning();
		const bool bIsStanding = !IsCrouching() && !IsProning();

		if (bIsCrouching && !bWantsToProne && (!bWantsToCrouch || !CanCrouchInCurrentState()))
		{
			UnCrouch();
		}
		else if (bIsCrouching && bWantsToProne && CanProneInCurrentState())
		{
			Prone();
		}
		else if (bIsProning && !bWantsToProne && bWantsToCrouch && CanCrouchInCurrentState())
		{
			UnProne();
		}
		else if (bIsStanding && bWantsToCrouch && CanCrouchInCurrentState())
		{
			Crouch();
		}
		else if (bIsProning && !bWantsToCrouch && !bWantsToProne)
		{
			UnProne();
			UnCrouch();
		}
	}
}

void UGCBaseCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// crouch if no longer allowed to prone
		if (IsProning() && !CanProneInCurrentState())
		{
			UnProne();
		}

		// Uncrouch if no longer allowed to be crouched
		if (IsCrouching() && !CanCrouchInCurrentState())
		{
			UnCrouch();
		}
	}
}

void FSavedMove_GC::Clear()
{
	Super::Clear();
	bSavedIsSprinting = 0;
	bSavedIsMantling = 0;
	bSavedIsPressingSlide = 0;
}

uint8 FSavedMove_GC::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	//	FLAG_JumpPressed = 0x01,	// Jump pressed
	//	FLAG_WantsToCrouch = 0x02,	// Wants to crouch
	//	FLAG_Reserved_1 = 0x04,	// Reserved for future use
	//	FLAG_Reserved_2 = 0x08,	// Reserved for future use
	//	FLAG_Custom_0 = 0x10 - Sprinting Flag,
	//	FLAG_Custom_1 = 0x20 - Mantling Flag,
	//	FLAG_Custom_2 = 0x40 - Sliding Flag,
	//	FLAG_Custom_3 = 0x80,

	if (bSavedIsSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if (bSavedIsMantling)
	{
		Result &= ~FLAG_JumpPressed;
		Result |= FLAG_Custom_1;
	}

	if (bSavedIsPressingSlide)
	{
		Result &= ~FLAG_WantsToCrouch;
		Result |= FLAG_Custom_2;
	}

	return Result;
}

bool FSavedMove_GC::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_GC* GCNewMovePtr = StaticCast<const FSavedMove_GC*>(NewMovePtr.Get());

	if (bSavedIsSprinting != GCNewMovePtr->bSavedIsSprinting ||
		bSavedIsMantling != GCNewMovePtr->bSavedIsMantling ||
		bSavedIsPressingSlide != GCNewMovePtr->bSavedIsPressingSlide)
	{
		return false;
	}

	return Super::CanCombineWith(NewMovePtr, InCharacter, MaxDelta);
}

void FSavedMove_GC::SetMoveFor(ACharacter* InCharacter, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(InCharacter, InDeltaTime, NewAccel, ClientData);

	check(InCharacter->IsA<AGCBaseCharacter>());
	AGCBaseCharacter* GCBaseCharacter = StaticCast<AGCBaseCharacter*>(InCharacter);
	UGCBaseCharacterMovementComponent* MovementComponent = GCBaseCharacter->GetBaseCharacterMovementComponent();

	bSavedIsSprinting = MovementComponent->bIsSprinting;
	bSavedIsMantling = GCBaseCharacter->bIsMantling;
	bSavedIsPressingSlide = GCBaseCharacter->bIsSliding;
}

void FSavedMove_GC::PrepMoveFor(ACharacter* Character)
{
	check(Character->IsA<AGCBaseCharacter>());
	AGCBaseCharacter* GCBaseCharacter = StaticCast<AGCBaseCharacter*>(Character);

	Super::PrepMoveFor(Character);

	UGCBaseCharacterMovementComponent* MovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(Character->GetMovementComponent());
	MovementComponent->bIsSprinting = bSavedIsSprinting;
	GCBaseCharacter->bIsSliding = bSavedIsPressingSlide;
}

FNetworkPredictionData_Client_Character_GC::FNetworkPredictionData_Client_Character_GC(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Character_GC::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_GC());
}