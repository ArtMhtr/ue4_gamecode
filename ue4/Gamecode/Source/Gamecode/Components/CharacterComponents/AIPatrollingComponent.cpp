// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPatrollingComponent.h"
#include "Actors/Navigation/PatrollingPath.h"

bool UAIPatrollingComponent::CanPatrol() const
{
	return IsValid(PatrolSettings.PatrollingPath) && PatrolSettings.PatrollingPath->GetWayPoints().Num() > 0;
}

void UAIPatrollingComponent::BeginPlay()
{
	Super::BeginPlay();
	if (CanPatrol())
	{
		if (PatrolSettings.PatrollingRouteType == EPatrollingRouteType::Circle)
		{
			CurrentWayPointIndex = PatrolSettings.PatrollingPath->GetWayPoints().Num() - 1;
		}
		else if (PatrolSettings.PatrollingRouteType == EPatrollingRouteType::PingPong)
		{
			PatrolRoute = EPatrolDirection::Reverse;
			CurrentWayPointIndex = 1;
		}
	}
}

FVector UAIPatrollingComponent::SelectClosestWayPoint()
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const TArray<FVector> WayPoints = PatrolSettings.PatrollingPath->GetWayPoints();
	FTransform PathTransform = PatrolSettings.PatrollingPath->GetActorTransform();

	FVector ClosestWayPoint;
	float MinSquaredDistance = FLT_MAX;

	for (int32 i = 0; i < WayPoints.Num(); ++i)
	{
		FVector WayPointWorld = PathTransform.TransformPosition(WayPoints[i]);
		float CurrentSquaredDistance = (OwnerLocation - WayPointWorld).Size();
		if (CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestWayPoint = WayPointWorld;
			CurrentWayPointIndex = i;
		}
	}

	return ClosestWayPoint;
}

FVector UAIPatrollingComponent::SelectNextWayPoint()
{
	const TArray<FVector> WayPoints = PatrolSettings.PatrollingPath->GetWayPoints();

	if (CurrentWayPointIndex == WayPoints.Num() - 1)
	{
		if (PatrolSettings.PatrollingRouteType == EPatrollingRouteType::Circle)
		{
			CurrentWayPointIndex = 0;
		}
		else
		{
			PatrolRoute = EPatrolDirection::Reverse;
			--CurrentWayPointIndex;
		}
	}
	else if (CurrentWayPointIndex == 0)
	{
		if (PatrolSettings.PatrollingRouteType == EPatrollingRouteType::PingPong)
		{
			PatrolRoute = EPatrolDirection::Direct;
		}
		++CurrentWayPointIndex;
	}
	else
	{
		if (PatrolSettings.PatrollingRouteType == EPatrollingRouteType::Circle || PatrolRoute == EPatrolDirection::Direct)
		{
			++CurrentWayPointIndex;
		}
		else
		{
			--CurrentWayPointIndex;
		}
	}

	FTransform PathTransform = PatrolSettings.PatrollingPath->GetActorTransform();
	FVector WayPoint = PathTransform.TransformPosition(WayPoints[CurrentWayPointIndex]);
	return WayPoint;
}
