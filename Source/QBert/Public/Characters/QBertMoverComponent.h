#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QBertMoverComponent.generated.h"

/**
 * Moves its owner along a quadratic Bezier curve over a fixed duration.
 * Every hop, fall and disc ride in the game goes through this, so movement is frame-rate independent
 * (the original Blueprint timelines re-read the actor's location every frame, which made arcs depend on FPS).
 */
UCLASS(ClassGroup = (QBert), meta = (BlueprintSpawnableComponent))
class QBERT_API UQBertMoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQBertMoverComponent();

	/** Curve from the owner's current location through an explicit control point. */
	void MoveAlongCurve(const FVector& ControlPoint, const FVector& Destination, float Duration, FSimpleDelegate OnFinished = FSimpleDelegate());

	/** Arc whose control point sits at the midpoint of the move plus ArcOffset. */
	void Hop(const FVector& Destination, float Duration, const FVector& ArcOffset, FSimpleDelegate OnFinished = FSimpleDelegate());

	/** Straight line, optionally easing out towards the destination. */
	void MoveLinear(const FVector& Destination, float Duration, bool bEaseOut, FSimpleDelegate OnFinished = FSimpleDelegate());

	void Stop();

	bool IsMoving() const { return bMoving; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void Begin(const FVector& ControlPoint, const FVector& Destination, float Duration, bool bEaseOut, FSimpleDelegate&& OnFinished);

	FVector StartLocation = FVector::ZeroVector;
	FVector ControlLocation = FVector::ZeroVector;
	FVector EndLocation = FVector::ZeroVector;
	float MoveDuration = 0.f;
	float Elapsed = 0.f;
	bool bMoving = false;
	bool bUseEaseOut = false;
	FSimpleDelegate FinishedDelegate;
};
