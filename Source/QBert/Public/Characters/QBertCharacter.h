#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "QBertTypes.h"
#include "QBertCharacter.generated.h"

class UCapsuleComponent;
class UPaperFlipbook;
class UPaperFlipbookComponent;
class UQBertMoverComponent;
class USoundBase;
class AQBertPyramid;

/**
 * Anything that hops around the pyramid: Q*bert, the balls and Coily.
 * Owns the grid position and the shared hop / fall-off-the-edge behaviour; subclasses decide
 * where to hop and react to landing.
 */
UCLASS(Abstract)
class QBERT_API AQBertCharacter : public APawn
{
	GENERATED_BODY()

public:
	AQBertCharacter();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Snaps to a cell, cancelling any movement in progress. */
	virtual void PlaceAtCell(const FIntPoint& NewCell);

	FIntPoint GetCell() const { return Cell; }
	bool IsHopping() const { return bIsHopping; }
	bool IsFalling() const { return bIsFalling; }

protected:
	virtual void BeginPlay() override;

	/** Starts a hop. The grid cell updates immediately; the sprite catches up over HopDuration. */
	void Hop(EQBertDirection Direction);

	/** Called right after the cell changes, before anything moves. */
	virtual void OnHopStarted(const FIntPoint& FromCell) {}

	/** Called when hopping into open air. Return true to handle it (e.g. board a disc) instead of falling. */
	virtual bool HandleHopOffPyramid(EQBertDirection Direction) { return false; }

	virtual void OnHopLanded() {}
	virtual void OnStartedFalling() {}
	virtual void OnFellOffPyramid() {}

	virtual UPaperFlipbook* GetJumpFlipbook(EQBertDirection Direction) const;
	virtual UPaperFlipbook* GetIdleFlipbook(EQBertDirection Direction) const;
	virtual USoundBase* GetHopSound() const { return HopSound; }
	virtual FVector GetLandingOffset() const { return LandingOffset; }

	void ShowIdle();
	void ShowJump();
	void PlaySound(USoundBase* Sound) const;

	FVector GetRestLocation(const FIntPoint& InCell) const;
	AQBertPyramid* GetPyramid() const;

	/** Runs Callback after Delay seconds unless this actor is destroyed or its timers are cleared first. */
	void RunAfter(float Delay, TFunction<void()>&& Callback);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperFlipbookComponent> Sprite;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UQBertMoverComponent> Mover;

	/** Radius of the sphere used to detect contact with other characters. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Collision")
	float CollisionRadius = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Animation")
	TMap<EQBertDirection, TObjectPtr<UPaperFlipbook>> JumpFlipbooks;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Animation")
	TMap<EQBertDirection, TObjectPtr<UPaperFlipbook>> IdleFlipbooks;

	/** Used for any direction missing from JumpFlipbooks. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Animation")
	TObjectPtr<UPaperFlipbook> DefaultJumpFlipbook;

	/** Used for any direction missing from IdleFlipbooks. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Animation")
	TObjectPtr<UPaperFlipbook> DefaultIdleFlipbook;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> HopSound;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> FallSound;

	/** Pause between the jump pose appearing and the character leaving the cube. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Movement")
	float HopWindup = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Movement")
	float HopDuration = 0.5f;

	/** Control-point offset from the middle of the hop; Z gives the arc its height. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Movement")
	FVector HopArcOffset = FVector(0.f, 0.f, 16.f);

	/** Where the character stands relative to the centre of a cube. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Movement")
	FVector LandingOffset = FVector(0.f, 2.f, 15.f);

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Movement")
	float FallDuration = 1.f;

	/** How far below the missed cube the fall ends (negative Y drops behind the pyramid). */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Movement")
	FVector FallEndOffset = FVector(0.f, -1.f, -250.f);

	FIntPoint Cell = FIntPoint(1, 7);
	EQBertDirection Facing = EQBertDirection::DownLeft;
	bool bIsHopping = false;
	bool bIsFalling = false;

private:
	void FallOff();
	void HandleHopFinished();

	mutable TWeakObjectPtr<AQBertPyramid> CachedPyramid;
};
