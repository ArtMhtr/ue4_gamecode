// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Zipline.generated.h"

UCLASS(Blueprintable)
class GAMECODE_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	AZipline();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "Zipline parameters")
	FVector GetCableNormalVector() const { return CableNormalVector; }

	UFUNCTION(BlueprintCallable, Category = "Zipline parameters")
	FVector GetZiplineTopPillarRelativeLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Zipline parameters")
	FVector GetZiplineBottomPillarRelativeLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Zipline parameters")
	float GetZiplineTopPillarHeight() const;

	UFUNCTION(BlueprintCallable, Category = "Zipline parameters")
	float GetZiplineBottomPillarHeight() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float FirstPillarHeight = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float SecondPillarHeight = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	UStaticMeshComponent* FirstPillarMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	UStaticMeshComponent* SecondPillarMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	UStaticMeshComponent* CableMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	FVector FirstPillarRelativeLocation = FVector(50.0f, 50.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	FVector SecondPillarRelativeLocation = FVector(-50.0f, -50.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float InteractionCapsuleRadius = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	FVector CableNormalVector = FVector(0.0f, 0.0f, 0.0f);

private:
	FVector FirstPillarTop = FVector::ZeroVector;
	FVector SecondPillarTop = FVector::ZeroVector;;
	bool bIsFirstPillarHigher = false;
};
