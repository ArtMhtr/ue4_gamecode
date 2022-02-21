// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectorComponent.h"
#include <GameFramework/Character.h>
#include <Components/CapsuleComponent.h>
#include "../GameCodeTypes.h"
#include <DrawDebugHelpers.h>
#include "../Utils/GCTraceUtils.h"
#include "../Characters/GCBaseCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "../GCGameInstance.h"
#include "../Subsystems/DebugSubsystem.h"

void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<ACharacter>(), TEXT("ULedgeDetectorComponent::BeginPlay() only a character can use ULedgeDetectorComponent"));

	CachedCharacterOwner = StaticCast<ACharacter*>(GetOwner());

}

bool ULedgeDetectorComponent::DetectLedge(OUT FLedgeDescription& LedgeDescription)
{
	ACharacter* DefaultCharacter = CachedCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	float ScaledCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float ScaledCapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

#if ENABLE_DRAW_DEBUG
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bool bIsDebugEnabled = false;
#endif

	float DrawTime = 3.0f;

	float BottomZOffset = 2.0f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() -
		(ScaledCapsuleHalfHeight - BottomZOffset) * FVector::UpVector;

	// 1. Forward Check
	float ForwardCheckCapsuleRadius = ScaledCapsuleRadius;
	float ForwardCheckCapsuleHalfHeight = (MaximumLedgeHeight - MinimumLedgeHeight) * 0.5f;

	FVector ForwardCheckStartLocation = CharacterBottom +
		(MinimumLedgeHeight + ForwardCheckCapsuleHalfHeight) * FVector::UpVector;
	FVector ForwardCheckEndLocation = ForwardCheckStartLocation +
		CachedCharacterOwner->GetActorForwardVector() * ForwardCheckDistance;

	FHitResult ForwardCheckHitResult;

	if (!GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), ForwardCheckHitResult, ForwardCheckStartLocation, ForwardCheckEndLocation,
		ForwardCheckCapsuleRadius, ForwardCheckCapsuleHalfHeight, FQuat::Identity, ECC_Climbing, QueryParams,
		FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	// 2. Downward check
	FHitResult DownwardCheckHitResult;
	float DownwardCheckSphereRadius = ScaledCapsuleRadius;

	float DownwardCheckDepthOffset = 10.0f;
	FVector DownwardStartLocation =
		ForwardCheckHitResult.ImpactPoint - ForwardCheckHitResult.ImpactNormal * DownwardCheckDepthOffset;
	DownwardStartLocation.Z = CharacterBottom.Z + MaximumLedgeHeight + DownwardCheckSphereRadius;

	FVector DownwardEndLocation(DownwardStartLocation.X, DownwardStartLocation.Y, CharacterBottom.Z);

	if (!GCTraceUtils::SweepSphereSingleByChannel(GetWorld(), DownwardCheckHitResult, DownwardStartLocation, DownwardEndLocation,
		DownwardCheckSphereRadius, ECC_Climbing, QueryParams, FCollisionResponseParams::DefaultResponseParam,
		bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	LedgeDescription.LedgePoint = DownwardCheckHitResult.ImpactPoint;
	LedgeDescription.Ledge = DownwardCheckHitResult.Component;

	// 3. Overlap check
	float OverlapCapsuleFloorOffset = 2.0f;
	float OverlapCapsuleRadius = ScaledCapsuleRadius;
	float OverlapCapsuleHalfHeight = ScaledCapsuleHalfHeight;
	FVector OverlapLocation = DownwardCheckHitResult.ImpactPoint + (OverlapCapsuleHalfHeight + OverlapCapsuleFloorOffset) * FVector::UpVector;

	if (GCTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius, OverlapCapsuleHalfHeight,
		FQuat::Identity, CollisionProfilePawn, QueryParams, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	LedgeDescription.Location = OverlapLocation;
	LedgeDescription.Rotation = (ForwardCheckHitResult.ImpactNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator();
	LedgeDescription.LedgeNormal = ForwardCheckHitResult.ImpactNormal;

	return true;
}
