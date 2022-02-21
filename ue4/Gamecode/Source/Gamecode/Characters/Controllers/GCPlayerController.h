// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Characters/GCBaseCharacter.h"
#include "GCPlayerController.generated.h"

UCLASS()
class GAMECODE_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void SetPawn(APawn* InPawn) override;

	bool GetIgnoreCameraPitch() const { return bIgnoreCameraPitch; }
	void SetIgnoreCameraPitch(bool bIgnoreCameraPitchIn) { bIgnoreCameraPitch = bIgnoreCameraPitchIn; }

protected:
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> MainMenuWidgetClass;

private:
	TSoftObjectPtr<class AGCBaseCharacter> CachedBaseCharacter;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);

	void ChangeCrouchState();
	void ChangeProneState();

	void StartSprint();
	void StopSprint();

	void SwimForward(float Value);
	void SwimRight(float Value);
	void SwimUp(float Value);

	void Mantle();
	void Jump();
	void Slide();

	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	void InteractWithZipline();
	void PlayerStartFire();
	void PlayerStopFire();

	void StartAiming();
	void StopAiming();

	void Reload();
	void NextItem();
	void PrevItem();
	void EquipPrimaryItem();

	void PrimaryMeleeAttack();
	void SecondaryMeleeAttack();
	void ChangeAmmoType();

	void ToggleMainMenu();
	void Interact();
	void UseInventory();

private:

	void OnInteractableObjectFound(FName ActionName);
	bool bIgnoreCameraPitch = false;
	UPlayerHUDWidget* PlayerHUDWidget = nullptr;
	UUserWidget* MainMenuWidget = nullptr;
	void CreateAndInitializeWidgets();
};
