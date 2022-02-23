#pragma once

#define ECC_Climbing ECC_GameTraceChannel1
#define ECC_InteractionVolume ECC_GameTraceChannel2
#define ECC_WallRunnable ECC_GameTraceChannel3
#define ECC_Bullet ECC_GameTraceChannel4
#define ECC_Melee ECC_GameTraceChannel5
#define ECC_PickableItem ECC_GameTraceChannel6

const FName FXParamTraceEnd = FName("TraceEnd");

const FName CollisionProfilePawn = FName("Pawn");
const FName CollisionProfileNoCollision = FName("NoCollision");
const FName CollisionProfileRagdoll = FName("Ragdoll");
const FName CollisionProfilePawnInteractionVolume = FName("PawnInteractionVolume");
const FName CollisionProfilePickableItemVolume = FName("PickableItemVolume");

const FName DebugCategoryLedgeDetection = FName("LedgeDetection");
const FName DebugCategoryCharacterAttributes = FName("CharacterAttributes");
const FName DebugCategoryRangeWeapon = FName("RangeWeapon");
const FName DebugCategoryMeleeWeapon = FName("MeleeWeapon");

const FName SocketFPCamera = FName("CameraSocket");
const FName SockedHead = FName("head");
const FName SocketCharacterWeapon = FName("CharacterWeaponSocket");
const FName SocketWeaponMuzzle = FName("MuzzleSocket");
const FName SocketWeaponForeGrip = FName("ForeGripSocket");
const FName SocketBowString = FName("SocketBowString");
const FName SocketCharacterThrowable = FName("ThrowableSocket");

const FName SectionMontageReloadEnd = FName("ReloadEnd");

const FName BB_CurrentTarget = FName("CurrentTarget");
const FName BB_NextLocation = FName("NextLocation");

const FName ActionInteract = FName("Interact");

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	None = 0,
	Left,
	Right
};

UENUM(BlueprintType)
enum class EEquipableItemType : uint8
{
	None = 0,
	Pistol,
	Rifle,
	Throwable,
	Bow,
	Melee
};

UENUM(BlueprintType)
enum class EAmmunitionType : uint8
{
	None = 0,
	Pistol,
	Rifle,
	ShotgunShells,
	FragGrenades,
	RifleGrenades,
	Arrows,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEquipmentSlots : uint8
{
	None = 0,
	SideArm,
	PrimaryWeapon,
	SecondaryWeapon,
	PrimaryItemSlot,
	MeleeWeapon,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EReticleType : uint8
{
	None = 0,
	Default,
	SniperRifle,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMeleeAttackType : uint8
{
	None = 0,
	PrimaryAttack,
	SecondaryAttack,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterAttributeType : uint8
{
	None = 0,
	Health,
	Stamina,
	Oxygen,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETeams : uint8
{
	Player = 0,
	Enemy,
};

UENUM(BlueprintType)
enum class EHitRegistrationType : uint8
{
	HitScan,
	Projectile
};

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
	None = 0,

	Ammo,
	Consumables,
	Weapon,

	MAX UMETA(Hidden)
};
