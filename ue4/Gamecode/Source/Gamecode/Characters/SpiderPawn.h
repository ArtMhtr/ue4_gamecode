// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCodeBasePawn.h"
#include "SpiderPawn.generated.h"

UCLASS()
class GAMECODE_API ASpiderPawn : public AGameCodeBasePawn
{
	GENERATED_BODY()

public:
	ASpiderPawn();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintGetter)
	FORCEINLINE float GetIKRightFrontFootOffset() { return IKRightFrontFootOffset; }

	UFUNCTION(BlueprintGetter)
	FORCEINLINE float GetIKRightRearFootOffset() { return IKRightRearFootOffset; }

	UFUNCTION(BlueprintGetter)
	FORCEINLINE float GetIKLeftFrontFootOffset() { return IKLeftFrontFootOffset; }

	UFUNCTION(BlueprintGetter)
	FORCEINLINE float GetIKLeftRearFootOffset() { return IKLeftRearFootOffset; }


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spider Bot")
	class USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider Bot|IK Settings")
	FName RightFrontFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider Bot|IK Settings")
	FName RightRearFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider Bot|IK Settings")
	FName LeftFrontFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spider Bot|IK Settings")
	FName LeftRearFootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider Bot|IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtendDistance = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spider Bot|IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;

private:

	float GetIKOffsetForASocket(const FName& SocketName);

	float IKRightFrontFootOffset = 0.0f;
	float IKRightRearFootOffset = 0.0f;
	float IKLeftFrontFootOffset = 0.0f;
	float IKLeftRearFootOffset = 0.0f;

	float IKTraceDistance = 0.0f;
	float IKScale = 1.0f;
};
