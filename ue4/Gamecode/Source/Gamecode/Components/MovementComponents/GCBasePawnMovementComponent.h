// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GCBasePawnMovementComponent.generated.h"

UCLASS()
class GAMECODE_API UGCBasePawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual bool IsFalling() const override;

public:
	void JumpStart();

protected:

	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1000.0f;

	UPROPERTY(EditAnywhere)
	bool bEnableGravity = true;

	UPROPERTY(EditAnywhere)
	float InitialJumpVelocity = 500.0f;

private:
	FVector VerticalVelocity = FVector::ZeroVector;
	bool bIsFalling = false;
};
