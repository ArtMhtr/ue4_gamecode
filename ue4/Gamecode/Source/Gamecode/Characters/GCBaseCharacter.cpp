
#include "GCBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Components/SkeletalMeshComponent.h>
#include "Components/LedgeDetectorComponent.h"
#include <Math/UnrealMathUtility.h>
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include <GameFramework/PhysicsVolume.h>
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include <AIController.h>
#include <Net/UnrealNetwork.h>
#include "Actors/Interactive/Interface/Interactive.h"
#include <Components/WidgetComponent.h>
#include "UI/Widget/World/GCAttributeProgressBar.h"
#include "Inventory/Items/InventoryItem.h"
#include "Components/CharacterComponents/CharacterInventoryComponent.h"
#include "Utils/GCDataTableUtils.h"

AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());
	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetector"));
	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>(TEXT("CharacterAttributes"));
	CharacterInventoryComponent = CreateDefaultSubobject<UCharacterInventoryComponent>(TEXT("CharacterInventory"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));

	HealthBarProgressComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarProgressComponent"));
	HealthBarProgressComponent->SetupAttachment(GetCapsuleComponent());

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;
}

void AGCBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAIController* AIController = Cast<AAIController>(NewController);
	if (IsValid(AIController))
	{
		FGenericTeamId TeamId((uint8)Team);
		AIController->SetGenericTeamId(TeamId);
	}
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeHealthProgress();

	CachedPreviousCharacterState = EGCBaseCharacterCachedPrevState::Stand;

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AGCBaseCharacter::OnPlayerCapsuleHit);
	GCBaseCharacterMovementComponent->SetPlaneConstraintEnabled(true);

	IKScale = GetActorScale3D().Z;
	IKTraceDistance = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	CharacterAttributesComponent->OnDeathEvent.AddUObject(this, &AGCBaseCharacter::OnDeath);
	GCBaseCharacterMovementComponent->bNotifyApex = 1;
	CharacterAttributesComponent->OnStaminaChangedState.BindUFunction(this, FName("OnStaminaStateChanged"));
}

void AGCBaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OnInteractableObjectFound.IsBound())
	{
		OnInteractableObjectFound.Unbind();
	}

	Super::EndPlay(EndPlayReason);
}

void AGCBaseCharacter::OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GCBaseCharacterMovementComponent->OnPlayerCapsuleHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AGCBaseCharacter::OnStaminaStateChanged(bool bIsOutOfStaminaState)
{
	bIsOutOfStamina = bIsOutOfStaminaState;
}

bool AGCBaseCharacter::CanCrouch() const
{
	if (!bIsOutOfStamina && !IsHardLanding() && !GCBaseCharacterMovementComponent->IsSprinting() && !GCBaseCharacterMovementComponent->IsOnWallRun() && !GCBaseCharacterMovementComponent->IsOnZipline() && !GCBaseCharacterMovementComponent->IsOnLadder() && !GCBaseCharacterMovementComponent->IsSwimming())
	{
		return Super::CanCrouch();
	}
	else
	{
		return false;
	}
}

bool AGCBaseCharacter::CanProne() const
{
	return !bIsProned && bIsCrouched && GCBaseCharacterMovementComponent && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AGCBaseCharacter::Prone()
{
	if (CanProne())
	{
		GCBaseCharacterMovementComponent->bWantsToProne = true;
	}
}

void AGCBaseCharacter::UnProne()
{
	GCBaseCharacterMovementComponent->bWantsToProne = false;
}

bool AGCBaseCharacter::IsStandingOnGround() const
{
	return GCBaseCharacterMovementComponent->IsMovingOnGround() && !bIsProned && !bIsCrouched;
}

void AGCBaseCharacter::ChangeCrouchState()
{
	/* crouch->stand, stand->crouch*/
	if (!GCBaseCharacterMovementComponent->IsSprinting() && ((bIsCrouched && CachedPreviousCharacterState == EGCBaseCharacterCachedPrevState::Prone) || IsStandingOnGround()))
	{
		bIsSprintRequested = false;
		if (GetCharacterMovement()->IsCrouching())
		{
			UnCrouch();
		}
		else
		{
			Crouch();
		}
	}
}

void AGCBaseCharacter::ChangeProneState()
{
	/* crouch->prone, prone->crouch*/
	if (!GCBaseCharacterMovementComponent->IsSprinting() && ((bIsCrouched && CachedPreviousCharacterState == EGCBaseCharacterCachedPrevState::Stand) || bIsProned))
	{
		bIsSprintRequested = false;
		if (bIsProned)
		{
			UnProne();
			Crouch();
		}
		else
		{
			UnCrouch();
			Prone();
		}
	}
}


void AGCBaseCharacter::StartSprint()
{
	if (bIsCrouched)
	{
		// TODO should be CanGrowToUncrouchedSize()
		UnCrouch();
		GCBaseCharacterMovementComponent->UnCrouch();
		if (bIsCrouched)
		{
			// if still unable to stand up due to collisions - do nothing
			return;
		}
	}
	bIsSprintRequested = true;
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

bool AGCBaseCharacter::CanMantle() const
{
	return GCBaseCharacterMovementComponent && !GCBaseCharacterMovementComponent->IsOnLadder() && !GCBaseCharacterMovementComponent->IsOnZipline() && !GCBaseCharacterMovementComponent->IsMantling() && !bIsProned && !GCBaseCharacterMovementComponent->IsSliding() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AGCBaseCharacter::Mantle(bool bForce /*= false*/)
{
	if (!bForce)
	{
		if (!CanMantle())
		{
			return;
		}
	}

	FLedgeDescription LedgeDesricption;
	if (LedgeDetectorComponent->DetectLedge(LedgeDesricption))
	{
		bIsMantling = true;

		FMantlingMovementParameters MantlingParameters;

		MantlingParameters.Ledge = LedgeDesricption.Ledge;
		MantlingParameters.LedgeInitialLocation = MantlingParameters.Ledge->GetComponentLocation();

		ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
		float DefaultScaledCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		MantlingParameters.InitialLocation = GetActorLocation() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + DefaultScaledCapsuleHalfHeight;
		MantlingParameters.InitialRotation = GetActorRotation();
		MantlingParameters.TargetLocation = LedgeDesricption.Location;
		MantlingParameters.TargetRotation = LedgeDesricption.Rotation;

		FVector CharacterBottom = GetActorLocation() - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * FVector::UpVector;
		float MantlingHeight = (LedgeDesricption.LedgePoint - CharacterBottom).Z;
		const FMantlingSettings& MantlingSettings = GetMantlingSettings(MantlingHeight);
		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;

		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);
		MantlingParameters.Duration = MaxRange - MinRange;

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationLocation = MantlingParameters.TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector + MantlingSettings.AnimationCorrectionXY * LedgeDesricption.LedgeNormal;

		if (IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			GCBaseCharacterMovementComponent->StartMantle(MantlingParameters);
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

void AGCBaseCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float StartTime)
{
}

bool AGCBaseCharacter::IsSwimmingUnderWater() const
{
	if (!GetCharacterMovement()->IsSwimming())
	{
		return false;
	}

	APhysicsVolume* Volume = GetCharacterMovement()->GetPhysicsVolume();
	float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z;
	FVector HeadPosition = GetMesh()->GetSocketLocation(SockedHead);

	return (VolumeTopPlane > HeadPosition.Z);
}

void AGCBaseCharacter::StartFire()
{
	if (CharacterEquipmentComponent->IsEquipping())
	{
		return;
	}

	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StartFire();
	}
}

void AGCBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

void AGCBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon))
	{
		return;
	}

	CurrentRangeWeapon->StartAim();
	CurrentAimingMovementSpeed = CurrentRangeWeapon->GetAimMovementMaxSpeed();
	bIsAiming = true;
	OnStartAiming();
}

void AGCBaseCharacter::StopAiming()
{
	if (!bIsAiming)
	{
		return;
	}

	ARangeWeaponItem* CurrentRangeWeapon = GetCharacterEquipmentComponent()->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopAim();
	}

	CurrentAimingMovementSpeed = 0;
	bIsAiming = false;
	OnStopAiming();
}

FRotator AGCBaseCharacter::GetAimOffset() const
{
	FVector AimDirectionWorld = GetBaseAimRotation().Vector();
	FVector AimDirectionLocal = GetTransform().InverseTransformVectorNoScale(AimDirectionWorld);

	FRotator Result = AimDirectionLocal.ToOrientationRotator();
	return Result;
}

void AGCBaseCharacter::Reload()
{
	if (IsValid(GetCharacterEquipmentComponent()->GetCurrentRangeWeapon()))
	{
		CharacterEquipmentComponent->ReloadCurrentWeapon();
	}
}

void AGCBaseCharacter::OnStartAiming_Implementation()
{
	OnStartAimingInternal();
}

void AGCBaseCharacter::OnStopAiming_Implementation()
{
	OnStopAimingInternal();
}

void AGCBaseCharacter::PrimaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::PrimaryAttack);
	}
}

void AGCBaseCharacter::SecondaryMeleeAttack()
{
	AMeleeWeaponItem* CurrentMeleeWeapon = GetCharacterEquipmentComponent()->GetCurrentMeleeWeapon();
	if (IsValid(CurrentMeleeWeapon))
	{
		CurrentMeleeWeapon->StartAttack(EMeleeAttackType::SecondaryAttack);
	}
}

FGenericTeamId AGCBaseCharacter::GetGenericTeamId() const
{
	return FGenericTeamId(uint8(Team));
}

void AGCBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.AddUnique(InteractiveActor);
}

void AGCBaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvailableInteractiveActors.RemoveSingleSwap(InteractiveActor);
}

void AGCBaseCharacter::ClimbLadderUp(float Value)
{
	if (GCBaseCharacterMovementComponent->IsOnLadder() && !FMath::IsNearlyZero(Value))
	{
		FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation();
}

void AGCBaseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, GetIKOffsetForASocket(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, GetIKOffsetForASocket(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	TryChangeSprintState();
	TraceLineOfSight();
}

void AGCBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGCBaseCharacter, bIsMantling);
	DOREPLIFETIME(AGCBaseCharacter, bIsSliding);
}

bool AGCBaseCharacter::CanSprint() const
{
	return !bIsOutOfStamina && !GCBaseCharacterMovementComponent->IsSliding() && !bIsProned && !GCBaseCharacterMovementComponent->IsSwimming() && !GCBaseCharacterMovementComponent->IsOnLadder() &&
		!GCBaseCharacterMovementComponent->IsOnWallRun() && !GCBaseCharacterMovementComponent->IsOnZipline() && !GCBaseCharacterMovementComponent->IsFalling() && !IsHardLanding();
}

const FMantlingSettings& AGCBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

float AGCBaseCharacter::GetIKOffsetForASocket(const FName& SocketName)
{
	float Result = 0.0f;
	FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - IKTraceDistance * FVector::UpVector;

	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z) / IKScale;
	}
	else if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceEnd, TraceEnd - IKTraceExtendDistance * FVector::UpVector, TraceType, true, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true))
	{
		Result = (TraceEnd.Z - HitResult.Location.Z) / IKScale;
	}

	return Result;
}

void AGCBaseCharacter::OnRep_IsSliding(bool bIsSliding_Old)
{
	if (bIsSliding)
	{
		bIsSprintRequested = false;
		GCBaseCharacterMovementComponent->StartSlide();
	}
	else
	{
		GCBaseCharacterMovementComponent->StopSlide();
	}
}

void AGCBaseCharacter::Server_StopSlide_Implementation()
{
	GCBaseCharacterMovementComponent->StopSlide();
}

void AGCBaseCharacter::Server_StartSlide_Implementation()
{
	GCBaseCharacterMovementComponent->StartSlide();
}

bool AGCBaseCharacter::CanSlide()
{
	return GCBaseCharacterMovementComponent && GCBaseCharacterMovementComponent->IsSprinting() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void AGCBaseCharacter::Slide()
{
	if (CanSlide())
	{
		bIsSprintRequested = false;
		GCBaseCharacterMovementComponent->StartSlide();
	}
}

void AGCBaseCharacter::OnCapsuleSizeAdjustDown(float HeightAdjust)
{
	// same code as in OnStartCrouch but without direct mutable
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* CurrentMesh = GetMesh();
	if (CurrentMesh && DefaultChar->GetMesh())
	{
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		CurrentMesh->SetRelativeLocation(DefaultChar->GetMesh()->GetRelativeLocation() + HeightAdjust * FVector::UpVector);
		BaseTranslationOffset.Z = CurrentMesh->GetRelativeLocation().Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z + HeightAdjust;
	}
}

void AGCBaseCharacter::OnCapsuleSizeAdjustUp(float HalfHeightAdjust)
{
	// same code as in OnStopCrouch but without direct mutable
	const ACharacter* DefaultChar = GetDefault<ACharacter>(GetClass());
	USkeletalMeshComponent* CurrentMesh = GetMesh();
	if (CurrentMesh && DefaultChar->GetMesh())
	{
		FVector MeshRelativeLocation = GetMesh()->GetRelativeLocation();
		CurrentMesh->SetRelativeLocation(DefaultChar->GetMesh()->GetRelativeLocation());
		BaseTranslationOffset.Z = CurrentMesh->GetRelativeLocation().Z;
	}
	else
	{
		BaseTranslationOffset.Z = DefaultChar->GetBaseTranslationOffset().Z;
	}
}

void AGCBaseCharacter::InteractWithLadder()
{
	if (GCBaseCharacterMovementComponent->IsSliding())
	{
		return;
	}

	if (GCBaseCharacterMovementComponent->IsOnLadder())
	{
		GCBaseCharacterMovementComponent->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* Ladder = GetAvailableInteractiveObject<ALadder>();
		if (Ladder)
		{
			if (Ladder->GetIsOnTop())
			{
				PlayAnimMontage(Ladder->GetAttachFromTopAnimMontage());
			}

			GCBaseCharacterMovementComponent->AttachToLadder(Ladder);
		}
	}
}

void AGCBaseCharacter::InteractWithZipline()
{
	if (GCBaseCharacterMovementComponent->IsOnZipline())
	{
		GCBaseCharacterMovementComponent->DetachFromZipline(EDetachFromZiplineMethod::JumpOff);
	}
	else
	{
		const AZipline* Zipline = GetAvailableInteractiveObject<AZipline>();
		if (Zipline)
		{
			GCBaseCharacterMovementComponent->AttachToZipline(Zipline);
		}
	}
}

bool AGCBaseCharacter::areRequiredKeysDown(EWallRunSide Side) const
{
	if (FMath::IsNearlyZero(ForwardAxisValue))
	{
		return false;
	}

	if (Side == EWallRunSide::Right && RightAxisValue < -0.1f)
	{
		return false;
	}

	if (Side == EWallRunSide::Left && RightAxisValue > 0.1f)
	{
		return false;
	}

	return true;
}

template<class InteractiveObjectType>
const InteractiveObjectType* AGCBaseCharacter::GetAvailableInteractiveObject() const
{
	const InteractiveObjectType* Result = nullptr;

	for (const AInteractiveActor* Actor : AvailableInteractiveActors)
	{
		if (Actor->IsA<InteractiveObjectType>())
		{
			// C-style cast
			Result = (InteractiveObjectType*)(Actor);
		}
	}

	return Result;
}

float AGCBaseCharacter::GetAimingMovementSpeed() const
{
	return CurrentAimingMovementSpeed;
}

void AGCBaseCharacter::OnRep_IsMantling(bool bWasMantling)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (!bWasMantling && bIsMantling)
		{
			Mantle(true);
		}
	}
}

void AGCBaseCharacter::Interact()
{
	if (LineOfSightObject.GetInterface())
	{
		LineOfSightObject->Interact(this);
	}
}

void AGCBaseCharacter::UseInventory()
{
	if (!IsPlayerControlled())
	{
		return;
	}

	APlayerController* PlayerController = GetController<APlayerController>();
	if (!CharacterInventoryComponent->IsViewVisible())
	{
		CharacterInventoryComponent->OpenViewInventory(PlayerController);
		CharacterEquipmentComponent->OpenViewEquipment(PlayerController);
		PlayerController->SetInputMode(FInputModeGameAndUI{});
		PlayerController->bShowMouseCursor = true;
	}
	else
	{
		CharacterInventoryComponent->CloseViewInventory();
		CharacterEquipmentComponent->CloseViewEquipment();
		PlayerController->SetInputMode(FInputModeGameOnly{});
		PlayerController->bShowMouseCursor = false;
	}
}

bool AGCBaseCharacter::AddItems(FName ItemId, EInventoryItemType ItemType, int32 AmountToAdd)
{
	if (ItemType == EInventoryItemType::Ammo)
	{
		FAmmoTableRowBase* AmmoInfo = GCDataTableUtils::FindAmmoItemDataInfo(ItemId);
		if (AmmoInfo)
		{
			// AddAmmunition will add items to inventory by itself
			CharacterEquipmentComponent->AddAmmunition(AmmoInfo->AmmunitionType, AmountToAdd);
		}
	}
	else
	{
		CharacterInventoryComponent->AddItems(ItemId, ItemType, AmountToAdd);
	}

	// TODO handle case when only part of items or no items were picked up
	return true;
}

void AGCBaseCharacter::RemoveItems(FName ItemId, EInventoryItemType ItemType, int32 AmountToRemove)
{
	if (ItemType == EInventoryItemType::Ammo)
	{
		FAmmoTableRowBase* AmmoInfo = GCDataTableUtils::FindAmmoItemDataInfo(ItemId);
		if (AmmoInfo)
		{
			// RemoveAmmunition will remove items from inventory by itself
			CharacterEquipmentComponent->RemoveAmmunition(AmmoInfo->AmmunitionType, AmountToRemove);
		}
	}
	else
	{
		CharacterInventoryComponent->RemoveItems(ItemId, ItemType, AmountToRemove);
	}
}

void AGCBaseCharacter::DropItems(UInventorySlot* Slot)
{
	CharacterInventoryComponent->DropItemsFromSlot(Slot);
}

void AGCBaseCharacter::Falling()
{
	Super::Falling();
	GCBaseCharacterMovementComponent->bNotifyApex = 1;
}

void AGCBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	CurrentFallApex = GetActorLocation();
}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = (CurrentFallApex - GetActorLocation()).Z / 100.0f;
	if (IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallHeight);
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), Hit.Actor.Get());

		if (CharacterAttributesComponent->IsAlive() && DamageAmount > HardLandingDamageAmount)
		{
			OnStartHardLanding();
		}
	}
}

void AGCBaseCharacter::OnDeath()
{
	float Duration = 0.0f;
	if (OnDeathAnimMontage)
	{
		Duration = PlayAnimMontage(OnDeathAnimMontage);
	}

	if (FMath::IsNearlyZero(Duration))
	{
		EnableRagdoll();
	}
	GCBaseCharacterMovementComponent->DisableMovement();

}

void AGCBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll, false);
	GetMesh()->SetSimulatePhysics(true);
}

void AGCBaseCharacter::EquipPrimaryItem()
{
	CharacterEquipmentComponent->EquipItemInSlot(EEquipmentSlots::PrimaryItemSlot);
}

void AGCBaseCharacter::ChangeAmmoType()
{
	CharacterEquipmentComponent->ChangeAmmoType();
}

void AGCBaseCharacter::NextItem()
{
	CharacterEquipmentComponent->EquipNextItem();
}

void AGCBaseCharacter::PrevItem()
{
	CharacterEquipmentComponent->EquipPrevItem();
}

void AGCBaseCharacter::OnHardLandingTimerElapsed()
{
	AController* GCController = GetController();
	if (IsValid(GCController))
	{
		GCController->SetIgnoreMoveInput(false);
	}
}

bool AGCBaseCharacter::IsHardLanding() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(HardLandingTimer);
}

void AGCBaseCharacter::OnStartAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(true);
	}
}

void AGCBaseCharacter::OnStopAimingInternal()
{
	if (OnAimingStateChanged.IsBound())
	{
		OnAimingStateChanged.Broadcast(false);
	}
}

void AGCBaseCharacter::TraceLineOfSight()
{
	if (!IsPlayerControlled())
	{
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	APlayerController* PlayerController = GetController<APlayerController>();
	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector ViewDirection = ViewRotation.Vector();
	FVector TraceEnd = ViewLocation + ViewDirection * LineOfSightDistance;

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_Visibility);

	if (LineOfSightObject.GetObject() != HitResult.Actor)
	{
		LineOfSightObject = HitResult.Actor.Get();

		FName ActionName;
		if (LineOfSightObject.GetInterface())
		{
			ActionName = LineOfSightObject->GetActionEventName();
		}
		else
		{
			ActionName = NAME_None;
		}

		OnInteractableObjectFound.ExecuteIfBound(ActionName);
	}

}

void AGCBaseCharacter::InitializeHealthProgress()
{
	UGCAttributeProgressBar* Widget = Cast<UGCAttributeProgressBar>(HealthBarProgressComponent->GetUserWidgetObject());
	if (!IsValid(Widget))
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}

	if (IsPlayerControlled() && IsLocallyControlled())
	{
		HealthBarProgressComponent->SetVisibility(false);
		return;
	}

	CharacterAttributesComponent->OnHealthChangedEvent.AddUObject(Widget, &UGCAttributeProgressBar::SetProgressPercentage);
	CharacterAttributesComponent->OnDeathEvent.AddLambda([=]() { HealthBarProgressComponent->SetVisibility(false); });
	Widget->SetProgressPercentage(CharacterAttributesComponent->GetAttributePercent(ECharacterAttributeType::Health));
}

void AGCBaseCharacter::OnStartHardLanding()
{
	if (HardLandingMontage)
	{
		float MontageDuration = PlayAnimMontage(HardLandingMontage);
		AController* GCController = GetController();
		if (IsValid(GCController))
		{
			GCController->SetIgnoreMoveInput(true);
		}

		GetWorld()->GetTimerManager().SetTimer(HardLandingTimer, this, &AGCBaseCharacter::OnHardLandingTimerElapsed, MontageDuration, false);
	}
}

void AGCBaseCharacter::Jump()
{
	if (bIsProned)
	{
		UnProne();
		UnCrouch();
	}
	else if (GCBaseCharacterMovementComponent->IsOnWallRun())
	{
		GCBaseCharacterMovementComponent->JumpOffTheWall();
	}
	else
	{
		Super::Jump();
	}
}

void AGCBaseCharacter::TryChangeSprintState()
{
	if (bIsSprintRequested && !bIsOutOfStamina && !GCBaseCharacterMovementComponent->IsSprinting() && CanSprint())
	{
		GCBaseCharacterMovementComponent->StartSprint();
		OnSprintStart();
	}

	if ((!bIsSprintRequested && GCBaseCharacterMovementComponent->IsSprinting()) || bIsOutOfStamina)
	{
		GCBaseCharacterMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

