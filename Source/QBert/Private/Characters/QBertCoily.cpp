#include "Characters/QBertCoily.h"
#include "Characters/QBertPlayerCharacter.h"
#include "Core/QBertGameMode.h"
#include "World/QBertPyramid.h"

AQBertCoily::AQBertCoily()
{
	FallDuration = 1.f;
}

void AQBertCoily::Think()
{
	if (Phase == EPhase::Egg)
	{
		Hop(FMath::RandBool() ? EQBertDirection::DownLeft : EQBertDirection::DownRight);
		return;
	}

	if (Phase == EPhase::Hatching)
	{
		Phase = EPhase::Snake;
	}

	EQBertDirection Direction;
	if (ChooseChaseDirection(Direction))
	{
		Hop(Direction);
	}
	else
	{
		ScheduleThink(RestBetweenHops);
	}
}

bool AQBertCoily::ChooseChaseDirection(EQBertDirection& OutDirection) const
{
	const AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>();
	const AQBertPlayerCharacter* Player = GameMode ? GameMode->GetPlayerCharacter() : nullptr;
	const AQBertPyramid* Pyramid = GetPyramid();
	if (!Player || !Pyramid)
	{
		return false;
	}

	// The lure: standing where Q*bert jumped onto a disc, Coily follows him off the edge.
	if (Player->IsRidingDisc() && Cell == Player->GetDepartureCell())
	{
		for (EQBertDirection Direction : QBertGrid::AllDirections)
		{
			if (Cell + QBertGrid::GetDelta(Direction) == Player->GetCell())
			{
				OutDirection = Direction;
				return true;
			}
		}
	}

	// Otherwise take whichever neighbouring cube is closest to Q*bert's last cube.
	const FVector Target = Pyramid->GetCellLocation(Player->GetDepartureCell());
	float BestDistanceSq = TNumericLimits<float>::Max();
	bool bFound = false;

	for (EQBertDirection Direction : QBertGrid::AllDirections)
	{
		const FIntPoint Candidate = Cell + QBertGrid::GetDelta(Direction);
		if (!Pyramid->IsCubeCell(Candidate))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(Pyramid->GetCellLocation(Candidate), Target);
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			OutDirection = Direction;
			bFound = true;
		}
	}
	return bFound;
}

void AQBertCoily::OnHopLanded()
{
	if (Phase == EPhase::Egg && ++EggHops >= HopsBeforeHatching)
	{
		Phase = EPhase::Hatching;
		ScheduleThink(RestBetweenHops + HatchDuration);
		return;
	}

	Super::OnHopLanded();
}

void AQBertCoily::OnStartedFalling()
{
	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandleCoilyLured(this);
	}
}

void AQBertCoily::OnFellOffPyramid()
{
	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandleCoilyFell(this);
	}
	Super::OnFellOffPyramid();
}

UPaperFlipbook* AQBertCoily::GetJumpFlipbook(EQBertDirection Direction) const
{
	return Phase == EPhase::Snake ? Super::GetJumpFlipbook(Direction) : EggJumpFlipbook.Get();
}

UPaperFlipbook* AQBertCoily::GetIdleFlipbook(EQBertDirection Direction) const
{
	return Phase == EPhase::Snake ? Super::GetIdleFlipbook(Direction) : EggIdleFlipbook.Get();
}

USoundBase* AQBertCoily::GetHopSound() const
{
	return Phase == EPhase::Snake ? Super::GetHopSound() : EggHopSound.Get();
}

FVector AQBertCoily::GetLandingOffset() const
{
	return Phase == EPhase::Snake ? SnakeLandingOffset : Super::GetLandingOffset();
}
