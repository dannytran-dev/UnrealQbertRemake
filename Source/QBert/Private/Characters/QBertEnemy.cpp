#include "Characters/QBertEnemy.h"
#include "Characters/QBertMoverComponent.h"
#include "Characters/QBertPlayerCharacter.h"
#include "Core/QBertGameMode.h"
#include "TimerManager.h"

void AQBertEnemy::BeginPlay()
{
	Super::BeginPlay();

	// Drop in from above the start cube, then start hopping.
	const FVector Rest = GetRestLocation(Cell);
	SetActorLocation(Rest + FVector(0.f, 0.f, DropHeight));
	Mover->MoveLinear(Rest, DropDuration, false);

	ScheduleThink(FirstHopDelay);
}

void AQBertEnemy::ScheduleThink(float Delay)
{
	GetWorldTimerManager().SetTimer(ThinkTimer, this, &AQBertEnemy::TryThink, FMath::Max(Delay, KINDA_SMALL_NUMBER), false);
}

void AQBertEnemy::TryThink()
{
	if (bStopped || bIsHopping || bIsFalling)
	{
		return;
	}
	if (bFrozen)
	{
		bThinkPending = true;
		return;
	}
	Think();
}

void AQBertEnemy::OnHopLanded()
{
	ScheduleThink(RestBetweenHops);
}

void AQBertEnemy::OnFellOffPyramid()
{
	Destroy();
}

void AQBertEnemy::Freeze(float Duration)
{
	bFrozen = true;
	GetWorldTimerManager().SetTimer(FreezeTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		bFrozen = false;
		if (bThinkPending)
		{
			bThinkPending = false;
			TryThink();
		}
	}), Duration, false);
}

void AQBertEnemy::HandleTouchedPlayer(AQBertPlayerCharacter* Player)
{
	if (bIsFalling)
	{
		return;
	}

	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandlePlayerCaught(Player, this);
	}
}

AQBertBall::AQBertBall()
{
	// Balls bounce off the bottom of the pyramid much more slowly than Q*bert falls.
	FallDuration = 3.f;
}

void AQBertBall::Think()
{
	Hop(FMath::RandBool() ? EQBertDirection::DownLeft : EQBertDirection::DownRight);
}

AQBertGreenBall::AQBertGreenBall()
{
	CollisionRadius = 8.f;
}

void AQBertGreenBall::HandleTouchedPlayer(AQBertPlayerCharacter* Player)
{
	if (IsFalling())
	{
		return;
	}

	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandleGreenBallCollected(this);
	}
	Destroy();
}
