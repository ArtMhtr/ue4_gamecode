// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"
#include "Gamecode/GameCodeTypes.h"
#include <Components/BoxComponent.h>
#include <Components/CapsuleComponent.h>

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ZiplineRoot"));

	FirstPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPillar"));
	FirstPillarMeshComponent->SetupAttachment(RootComponent);
	SecondPillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondPillar"));
	SecondPillarMeshComponent->SetupAttachment(RootComponent);

	CableMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cable"));
	CableMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	// Forbid scaling and rotation
	RootComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	RootComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	FirstPillarMeshComponent->SetRelativeLocation(FVector(FirstPillarRelativeLocation.X, FirstPillarRelativeLocation.Y, FirstPillarRelativeLocation.Z));
	SecondPillarMeshComponent->SetRelativeLocation(FVector(SecondPillarRelativeLocation.X, SecondPillarRelativeLocation.Y, SecondPillarRelativeLocation.Z));

	UStaticMesh* FirstPillarStaticMesh = FirstPillarMeshComponent->GetStaticMesh();
	if (IsValid(FirstPillarStaticMesh))
	{
		float MeshHeight = FirstPillarStaticMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			FirstPillarMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, FirstPillarHeight / MeshHeight));
		}
	}

	UStaticMesh* SecondPillarStaticMesh = SecondPillarMeshComponent->GetStaticMesh();
	if (IsValid(SecondPillarStaticMesh))
	{
		float MeshHeight = SecondPillarStaticMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			SecondPillarMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, SecondPillarHeight / MeshHeight));
		}
	}

	UStaticMesh* CableMesh = CableMeshComponent->GetStaticMesh();
	if (IsValid(CableMesh))
	{
		FirstPillarTop = FirstPillarMeshComponent->GetRelativeLocation() + FVector(0.0f, 0.0f, FirstPillarHeight * 0.5f);
		SecondPillarTop = SecondPillarMeshComponent->GetRelativeLocation() + FVector(0.0f, 0.0f, SecondPillarHeight * 0.5f);
		bIsFirstPillarHigher = FirstPillarTop.Z > SecondPillarTop.Z;

		FVector CableVector = bIsFirstPillarHigher ? (SecondPillarTop - FirstPillarTop) : (FirstPillarTop - SecondPillarTop);
		FRotator CableOrientationRotation = CableVector.ToOrientationRotator();

		float CableLength = CableVector.Size();
		float CableMeshLength = CableMesh->GetBoundingBox().GetSize().X;
		if (!FMath::IsNearlyZero(CableLength))
		{
			CableMeshComponent->SetRelativeScale3D(FVector(CableLength / CableMeshLength, 1.0f, 1.0f));
			CableMeshComponent->SetRelativeRotation(CableOrientationRotation);
			CableMeshComponent->SetRelativeLocation((FirstPillarTop + SecondPillarTop) / 2);

			UCapsuleComponent* ZiplineInteractionVolume = StaticCast<UCapsuleComponent*>(InteractionVolume);
			if (IsValid(ZiplineInteractionVolume))
			{
				ZiplineInteractionVolume->SetCapsuleSize(InteractionCapsuleRadius, CableLength / 2);
				ZiplineInteractionVolume->SetRelativeRotation(CableOrientationRotation + FRotator(90.0f, 0.0f, 0.0f));
				ZiplineInteractionVolume->SetRelativeLocation((FirstPillarTop + SecondPillarTop) / 2);
			}
		}

		CableNormalVector = CableVector.GetSafeNormal();
	}
}

FVector AZipline::GetZiplineTopPillarRelativeLocation() const
{
	return bIsFirstPillarHigher ? FirstPillarRelativeLocation : SecondPillarRelativeLocation;
}

float AZipline::GetZiplineTopPillarHeight() const
{
	return bIsFirstPillarHigher ? FirstPillarHeight : SecondPillarHeight;
}

float AZipline::GetZiplineBottomPillarHeight() const
{
	return bIsFirstPillarHigher ? SecondPillarHeight : FirstPillarHeight;
}

FVector AZipline::GetZiplineBottomPillarRelativeLocation() const
{
	return bIsFirstPillarHigher ? SecondPillarRelativeLocation : FirstPillarRelativeLocation;
}
