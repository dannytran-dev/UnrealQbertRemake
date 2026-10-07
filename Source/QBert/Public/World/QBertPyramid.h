#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QBertTypes.h"
#include "QBertPyramid.generated.h"

class UPaperSpriteComponent;
class AQBertDisc;

/**
 * The playing field. Builds the cube pyramid from grid maths, tracks which cubes have been
 * changed this round and owns the escape discs on either side.
 *
 * Grid layout: row Y (1 = bottom row, NumRows = apex) holds cubes X = 1 .. NumRows + 1 - Y.
 * Any other cell is open air; hopping into it means falling off, unless a disc is parked there.
 */
UCLASS()
class QBERT_API AQBertPyramid : public AActor
{
	GENERATED_BODY()

public:
	AQBertPyramid();

	static AQBertPyramid* FindInWorld(const UObject* WorldContextObject);

	virtual void OnConstruction(const FTransform& Transform) override;

	/** World location of the centre of a grid cell (cube or open air). */
	FVector GetCellLocation(const FIntPoint& Cell) const;

	bool IsCubeCell(const FIntPoint& Cell) const;
	FIntPoint GetTopCell() const { return FIntPoint(1, NumRows); }
	int32 GetCubeCount() const { return CubeCells.Num(); }

	const FQBertRoundTheme& GetTheme(int32 Round) const;

	/** Turns the cube into this round's target colour. Returns true if it changed. */
	bool ActivateCube(const FIntPoint& Cell, int32 Round);
	bool AreAllCubesActivated(int32 Round) const;
	void ResetCubes(int32 Round);

	/** Alternates every cube between the round's two colours, then restores the start colour. */
	void FlashCubes(int32 Round, int32 FlashCount, float Interval, FSimpleDelegate OnFinished);

	void SpawnDiscs(int32 Round);
	void ClearDiscs();
	void RemoveDisc(AQBertDisc* Disc);
	AQBertDisc* GetDiscAt(const FIntPoint& Cell) const;
	int32 GetDiscCount() const { return Discs.Num(); }

	/** Where a disc carries Q*bert before he hops back onto the apex. */
	FVector GetDiscDropOffLocation() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	/** One entry per round; the last entry is reused if the game runs longer. */
	UPROPERTY(EditAnywhere, Category = "Pyramid")
	TArray<FQBertRoundTheme> RoundThemes;

	UPROPERTY(EditAnywhere, Category = "Pyramid|Layout")
	FVector GridOrigin = FVector(-48.f, 0.f, -24.f);

	/** Offset between neighbouring cubes in a row. */
	UPROPERTY(EditAnywhere, Category = "Pyramid|Layout")
	FVector ColumnStep = FVector(32.f, 0.f, 0.f);

	/** Offset from one row to the row above it. */
	UPROPERTY(EditAnywhere, Category = "Pyramid|Layout")
	FVector RowStep = FVector(16.f, 0.f, 24.f);

	/** Extra depth per row so the cube sprites of lower (nearer) rows always sort in front. */
	UPROPERTY(EditAnywhere, Category = "Pyramid|Layout")
	float RowDepthOffset = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Discs")
	TSubclassOf<AQBertDisc> DiscClass;

	/** Open-air cells left of the pyramid where a disc may appear. */
	UPROPERTY(EditAnywhere, Category = "Discs")
	TArray<FIntPoint> LeftDiscCells = { {0, 3}, {0, 4}, {0, 5}, {0, 6} };

	/** Open-air cells right of the pyramid where a disc may appear. */
	UPROPERTY(EditAnywhere, Category = "Discs")
	TArray<FIntPoint> RightDiscCells = { {6, 3}, {5, 4}, {4, 5}, {3, 6} };

	UPROPERTY(EditAnywhere, Category = "Discs", meta = (ClampMin = "0"))
	int32 DiscsPerSide = 1;

	/** Height above the apex cube where discs stop. */
	UPROPERTY(EditAnywhere, Category = "Discs")
	float DiscDropOffHeight = 48.f;

private:
	static constexpr int32 NumRows = 7;

	int32 FindCubeIndex(const FIntPoint& Cell) const;
	FVector GetCellLocalLocation(const FIntPoint& Cell) const;
	FVector GetCubeLocalLocation(const FIntPoint& Cell) const;
	void SetAllCubes(UPaperSprite* Sprite);
	void SpawnDiscsOnSide(const TArray<FIntPoint>& Candidates, const FQBertRoundTheme& Theme);
	void TickFlash();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TArray<TObjectPtr<UPaperSpriteComponent>> CubeSprites;

	TArray<FIntPoint> CubeCells;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AQBertDisc>> Discs;

	FTimerHandle FlashTimer;
	int32 FlashRound = 1;
	int32 FlashStepsRemaining = 0;
	bool bFlashShowingTarget = false;
	FSimpleDelegate FlashFinished;
};
