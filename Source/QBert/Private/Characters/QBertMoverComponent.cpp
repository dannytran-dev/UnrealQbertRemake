#include "Characters/QBertMoverComponent.h"
#include "GameFramework/Actor.h"

UQBertMoverComponent::UQBertMoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UQBertMoverComponent::MoveAlongCurve(const FVector& ControlPoint, const FVector& Destination, float Duration, FSimpleDelegate OnFinished)
{
	Begin(ControlPoint, Destination, Duration, false, MoveTemp(OnFinished));
}

void UQBertMoverComponent::Hop(const FVector& Destination, float Duration, const FVector& ArcOffset, FSimpleDelegate OnFinished)
{
	const FVector Midpoint = (GetOwner()->GetActorLocation() + Destination) * 0.5f;
	Begin(Midpoint + ArcOffset, Destination, Duration, false, MoveTemp(OnFinished));
}

void UQBertMoverComponent::MoveLinear(const FVector& Destination, float Duration, bool bEaseOut, FSimpleDelegate OnFinished)
{
	// A Bezier whose control point is the midpoint is a straight line.
	const FVector Midpoint = (GetOwner()->GetActorLocation() + Destination) * 0.5f;
	Begin(Midpoint, Destination, Duration, bEaseOut, MoveTemp(OnFinished));
}

void UQBertMoverComponent::Stop()
{
	bMoving = false;
	FinishedDelegate.Unbind();
	SetComponentTickEnabled(false);
}

void UQBertMoverComponent::Begin(const FVector& ControlPoint, const FVector& Destination, float Duration, bool bEaseOut, FSimpleDelegate&& OnFinished)
{
	StartLocation = GetOwner()->GetActorLocation();
	ControlLocation = ControlPoint;
	EndLocation = Destination;
	MoveDuration = FMath::Max(Duration, 0.f);
	Elapsed = 0.f;
	bUseEaseOut = bEaseOut;
	FinishedDelegate = MoveTemp(OnFinished);
	bMoving = true;
	SetComponentTickEnabled(true);
}

void UQBertMoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bMoving)
	{
		return;
	}

	Elapsed += DeltaTime;
	const float LinearAlpha = MoveDuration > 0.f ? FMath::Clamp(Elapsed / MoveDuration, 0.f, 1.f) : 1.f;
	const float Alpha = bUseEaseOut ? FMath::InterpEaseOut(0.f, 1.f, LinearAlpha, 2.f) : LinearAlpha;

	const float InvAlpha = 1.f - Alpha;
	const FVector Location = InvAlpha * InvAlpha * StartLocation + 2.f * Alpha * InvAlpha * ControlLocation + Alpha * Alpha * EndLocation;
	GetOwner()->SetActorLocation(Location);

	if (LinearAlpha >= 1.f)
	{
		bMoving = false;
		SetComponentTickEnabled(false);

		// Move the delegate out first: the callback commonly starts the next move.
		FSimpleDelegate Finished = MoveTemp(FinishedDelegate);
		FinishedDelegate.Unbind();
		Finished.ExecuteIfBound();
	}
}
