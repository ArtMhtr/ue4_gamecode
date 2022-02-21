// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ThrowGrenade.generated.h"

UCLASS()
class GAMECODE_API UBTService_ThrowGrenade : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_ThrowGrenade();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (UIMin = 1000.0f, ClampMin = 1000.0f))
	float MaxThrowGrenadeDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (UIMin = 1000.0f, ClampMin = 1000.0f))
	float MinThrowGrenadeDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (UIMin = 0.5f, ClampMin = 0.5f))
	float MaxGrenadeThrowInterval = 3.0f;

private:

	void OnThrowGrenadeIntervalTimerElapsed();
	FTimerHandle ThrowGrenadeIntervalTimer;
};
