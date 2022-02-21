// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBasePawnAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include <GameFramework/Pawn.h>

void UGCBasePawnAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGameCodeBasePawn>(), TEXT("UGCBasePawnAnimInstance::NativeBeginPlay only works with AGameCodeBasePawn"));
	CachedGamePawn = StaticCast<AGameCodeBasePawn*>(TryGetPawnOwner());
}

void UGCBasePawnAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (CachedGamePawn.IsValid())
	{
		InputForward = CachedGamePawn->GetInputForward();
		InputRight = CachedGamePawn->GetInputRight();
		bIsInAir = CachedGamePawn->GetMovementComponent()->IsFalling();
	}
}
