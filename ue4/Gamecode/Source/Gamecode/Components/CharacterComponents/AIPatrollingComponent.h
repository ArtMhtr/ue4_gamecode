// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrollingComponent.generated.h"

class APatrollingPath;

UENUM(BlueprintType)
enum class EPatrollingRouteType : uint8
{
	Circle = 0,
	PingPong,
	MAX UMETA(Hidden)
};

enum class EPatrolDirection : uint8
{
	Direct,
	Reverse,
};

USTRUCT(BlueprintType)
struct FPatrolSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol settings")
	EPatrollingRouteType PatrollingRouteType;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol settings")
	APatrollingPath* PatrollingPath;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UAIPatrollingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	virtual void BeginPlay() override;
	bool CanPatrol() const;

	FVector SelectClosestWayPoint();
	FVector SelectNextWayPoint();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol settings")
	FPatrolSettings PatrolSettings;

private:

	int32 CurrentWayPointIndex = -1;
	EPatrolDirection PatrolRoute = EPatrolDirection::Direct;
};
