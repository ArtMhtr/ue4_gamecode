// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Actors/Interactive/InteractiveActor.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "GameCodeTypes.h"
#include <Animation/AnimMontage.h>
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include <GenericTeamAgentInterface.h>
#include "Actors/Interactive/Interface/Interactive.h"
#include <UObject/ScriptInterface.h>
#include "GCBaseCharacter.generated.h"

enum class EGCBaseCharacterCachedPrevState
{
	Stand = 0,
	Prone
};

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeight = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingStateChanged, bool)
DECLARE_DELEGATE_OneParam(FOnInteractableObjectFound, FName);

class UGCBaseCharacterMovementComponent;
class IInteractable;
class UWidgetComponent;
class UInventoryItem;
class UCharacterInventoryComponent;
class UInventorySlot;
UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void PossessedBy(AController* NewController) override;

	virtual void OnCapsuleSizeAdjustUp(float HalfHeightAdjust);
	virtual void OnCapsuleSizeAdjustDown(float HalfHeightAdjust);

	/* Controller */
	virtual void MoveForward(float Value) {};
	virtual void MoveRight(float Value) {};
	virtual void Turn(float Value) {};
	virtual void LookUp(float Value) {};
	virtual void TurnAtRate(float Value) {};
	virtual void LookUpAtRate(float Value) {};

	/* Jump */
	virtual void Jump() override;
	virtual bool CanJumpInternal_Implementation() const override;

	virtual void ChangeCrouchState();
	virtual void ChangeProneState();
	virtual bool IsStandingOnGround() const;

	virtual void SwimForward(float Value) {};
	virtual void SwimRight(float Value) {};
	virtual void SwimUp(float Value) {};

	/* Prone */
	virtual bool CanProne() const;
	virtual void Prone();
	virtual void UnProne();

	/* Sprint */
	virtual void TryChangeSprintState();
	virtual void StartSprint();
	virtual void StopSprint();

	/* Slide */
	UPROPERTY(ReplicatedUsing = OnRep_IsSliding)
	bool bIsSliding = false;

	UFUNCTION(Server, Reliable)
	void Server_StopSlide();

	UFUNCTION(Server, Reliable)
	void Server_StartSlide();

	virtual bool CanSlide();
	virtual void Slide();

	virtual class UAnimMontage* GetSlidingMontage() const { return SlidingMontage; }

	/* Crouch */
	virtual bool CanCrouch() const override;

	/* Mantle */
	virtual bool CanMantle() const;

	UFUNCTION(BlueprintCallable)
	virtual void Mantle(bool bForce = false);
	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float StartTime);

	/* Stamina */
	bool IsOutOfStamina() const { return bIsOutOfStamina; }

	/* Swimming */
	bool IsSwimmingUnderWater() const;

	/* Fire */
	void StartFire();
	void StopFire();

	/* Aim */
	void StartAiming();
	void StopAiming();
	FRotator GetAimOffset() const;

	void Reload();
	void NextItem();
	void PrevItem();
	void EquipPrimaryItem();
	void ChangeAmmoType();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStartAiming();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Character")
	void OnStopAiming();

	/* Melee */
	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();

	/* IGenericTeamInterface */
	virtual FGenericTeamId GetGenericTeamId() const override;

	/* IK */
	UFUNCTION(BlueprintGetter)
	FORCEINLINE float GetIKRightFootOffset() { return IKRightFootOffset; }

	UFUNCTION(BlueprintGetter)
	FORCEINLINE float GetIKLeftFootOffset() { return IKLeftFootOffset; }

	/* Interactive actors */
	TArray<AInteractiveActor*> AvailableInteractiveActors;
	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	void InteractWithZipline();
	bool areRequiredKeysDown(EWallRunSide Side) const;

	template<class InteractiveObjectType>
	const InteractiveObjectType* GetAvailableInteractiveObject() const;

	bool bIsProned = false;
	EGCBaseCharacterCachedPrevState CachedPreviousCharacterState;

	class UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const { return GCBaseCharacterMovementComponent; }

	const class UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const { return CharacterEquipmentComponent; }
	const class UCharacterAttributesComponent* GetCharacterAttributesComponent() const { return CharacterAttributesComponent; }

	class UCharacterAttributesComponent* GetCharacterAttributesComponent_Mutable() const { return CharacterAttributesComponent; }
	class UCharacterEquipmentComponent* GetCharacterEquipmentComponent_Mutable() const { return CharacterEquipmentComponent; }
	class UCharacterInventoryComponent* GetCharacterInventoryComponent_Mutable() const { return CharacterInventoryComponent; }

	bool IsAiming() const { return bIsAiming; }
	float GetAimingMovementSpeed() const;

	FOnAimingStateChanged OnAimingStateChanged;

	UPROPERTY(ReplicatedUsing=OnRep_IsMantling)
	bool bIsMantling;

	UFUNCTION()
	void OnRep_IsMantling(bool bWasMantling);

	FOnInteractableObjectFound OnInteractableObjectFound;
	void Interact();

	void UseInventory();
	
	bool AddItems(FName ItemId, EInventoryItemType ItemType, int32 AmountToAdd);
	void RemoveItems(FName ItemId, EInventoryItemType ItemType, int32 AmountToRemove);
	void DropItems(UInventorySlot* Slot);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/* Stamina */
	UFUNCTION()
	virtual void OnStaminaStateChanged(bool bIsOutOfStamina);

	/* Sprint */
	virtual bool CanSprint() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Sprint Settings")
	void OnSprintStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Sprint Settings")
	void OnSprintEnd();

	/* IK */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK Settings")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK Settings")
	FName LeftFootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKTraceExtendDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float IKInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Controls")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Controls")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Prone")
	float ProneMeshAdjust = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Sprint")
	float SprintSpeed = 1000.0f;

	/* Mantling */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LowMantleMaxHeight = 125.0f;

	/* Controller */
	float ForwardAxisValue = 0.0f;
	float RightAxisValue = 0.0f;

	/* Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterEquipmentComponent* CharacterEquipmentComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterAttributesComponent* CharacterAttributesComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterInventoryComponent* CharacterInventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class ULedgeDetectorComponent* LedgeDetectorComponent;

	/* Slide */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Movement | Sliding")
	class UAnimMontage* SlidingMontage = nullptr;

	/* Damage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Animations")
	class UAnimMontage* OnDeathAnimMontage = nullptr;

	virtual void OnDeath();

	// Damage depending from fall height in meters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	class UCurveFloat* FallDamageCurve;

	/* Hard landing */
	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyJumpApex() override;
	virtual void Falling() override;
	virtual void OnHardLandingTimerElapsed();
	virtual void OnStartHardLanding();
	bool IsHardLanding() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Movement | Landing")
	float HardLandingDamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Movement | Landing")
	class UAnimMontage* HardLandingMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Movement | Landing")
	class UAnimMontage* FPHardLandingMontage = nullptr;

	FTimerHandle HardLandingTimer;
	FVector CurrentFallApex;

	virtual void OnStartAimingInternal();
	virtual void OnStopAimingInternal();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character | Team")
	ETeams Team = ETeams::Enemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Team")
	float LineOfSightDistance = 500.0f;

	void TraceLineOfSight();

	UPROPERTY()
	TScriptInterface<IInteractable> LineOfSightObject;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character | Components")
	UWidgetComponent* HealthBarProgressComponent;

	void InitializeHealthProgress();

private:

	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;
	void EnableRagdoll();

	/* IK */
	float GetIKOffsetForASocket(const FName& SocketName);

	float IKRightFootOffset = 0.0f;
	float IKLeftFootOffset = 0.0f;
	float IKTraceDistance = 0.0f;
	float IKScale = 1.0f;

	/* Sprint */
	bool bIsSprintRequested = false;

	/* Stamina */
	bool bIsOutOfStamina = false;

	/* Sliding */
	UFUNCTION()
	void OnRep_IsSliding(bool bIsSliding_Old);

	bool bIsAiming;
	float CurrentAimingMovementSpeed = 200.0f;
};
