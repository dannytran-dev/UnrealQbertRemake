#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QBertDisc.generated.h"

class UPaperFlipbook;
class UPaperFlipbookComponent;
class UQBertMoverComponent;

/** Spinning escape disc parked beside the pyramid. Carries Q*bert back to the apex. */
UCLASS()
class QBERT_API AQBertDisc : public AActor
{
	GENERATED_BODY()

public:
	AQBertDisc();

	void Initialize(const FIntPoint& InCell, UPaperFlipbook* Flipbook);
	void FlyTo(const FVector& Destination, float Duration, FSimpleDelegate OnArrived);

	FIntPoint GetCell() const { return Cell; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperFlipbookComponent> Sprite;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UQBertMoverComponent> Mover;

private:
	FIntPoint Cell = FIntPoint::ZeroValue;
};
