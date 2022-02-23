// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BowAnimInstance.generated.h"

class ARangeWeaponItem;

UCLASS()
class GAMECODE_API UBowAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bow Animation")
	bool bIsAiming = false;

private:
	TWeakObjectPtr<ARangeWeaponItem> CachedBow;
};
