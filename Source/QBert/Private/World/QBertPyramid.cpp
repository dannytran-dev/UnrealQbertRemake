#include "World/QBertPyramid.h"
#include "World/QBertDisc.h"
#include "PaperSpriteComponent.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AQBertPyramid::AQBertPyramid()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	for (int32 Row = 1; Row <= NumRows; ++Row)
	{
		for (int32 Column = 1; Column <= NumRows + 1 - Row; ++Column)
		{
			const FIntPoint Cell(Column, Row);
			const FName Name(*FString::Printf(TEXT("Cube_%d_%d"), Column, Row));

			UPaperSpriteComponent* Cube = CreateDefaultSubobject<UPaperSpriteComponent>(Name);
			Cube->SetupAttachment(Root);
			Cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Cube->SetRelativeLocation(GetCubeLocalLocation(Cell));
			// Rows nearer the bottom are in front, so they must draw over the row behind them.
			Cube->SetTranslucentSortPriority(NumRows - Row);

			CubeSprites.Add(Cube);
			CubeCells.Add(Cell);
		}
	}
}

AQBertPyramid* AQBertPyramid::FindInWorld(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AQBertPyramid> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void AQBertPyramid::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Layout properties may have been changed in a Blueprint or in the level, so re-place every cube.
	for (int32 Index = 0; Index < CubeSprites.Num(); ++Index)
	{
		CubeSprites[Index]->SetRelativeLocation(GetCubeLocalLocation(CubeCells[Index]));
	}
	ResetCubes(1);
}

FVector AQBertPyramid::GetCellLocalLocation(const FIntPoint& Cell) const
{
	return GridOrigin + ColumnStep * Cell.X + RowStep * Cell.Y;
}

FVector AQBertPyramid::GetCubeLocalLocation(const FIntPoint& Cell) const
{
	// Nudge each row towards the camera so overlapping cube sprites never share a depth.
	return GetCellLocalLocation(Cell) + FVector(0.f, RowDepthOffset * (NumRows - Cell.Y), 0.f);
}

FVector AQBertPyramid::GetCellLocation(const FIntPoint& Cell) const
{
	return GetActorTransform().TransformPosition(GetCellLocalLocation(Cell));
}

bool AQBertPyramid::IsCubeCell(const FIntPoint& Cell) const
{
	return Cell.X >= 1 && Cell.Y >= 1 && Cell.X + Cell.Y <= NumRows + 1;
}

int32 AQBertPyramid::FindCubeIndex(const FIntPoint& Cell) const
{
	return CubeCells.IndexOfByKey(Cell);
}

const FQBertRoundTheme& AQBertPyramid::GetTheme(int32 Round) const
{
	static const FQBertRoundTheme EmptyTheme;
	if (RoundThemes.IsEmpty())
	{
		return EmptyTheme;
	}
	return RoundThemes[FMath::Clamp(Round - 1, 0, RoundThemes.Num() - 1)];
}

bool AQBertPyramid::ActivateCube(const FIntPoint& Cell, int32 Round)
{
	const int32 Index = FindCubeIndex(Cell);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	// SetSprite only reports true when the sprite actually changed.
	return CubeSprites[Index]->SetSprite(GetTheme(Round).TargetCube);
}

bool AQBertPyramid::AreAllCubesActivated(int32 Round) const
{
	const UPaperSprite* Target = GetTheme(Round).TargetCube;
	for (const TObjectPtr<UPaperSpriteComponent>& Cube : CubeSprites)
	{
		if (Cube->GetSprite() != Target)
		{
			return false;
		}
	}
	return true;
}

void AQBertPyramid::ResetCubes(int32 Round)
{
	SetAllCubes(GetTheme(Round).StartCube);
}

void AQBertPyramid::SetAllCubes(UPaperSprite* Sprite)
{
	for (UPaperSpriteComponent* Cube : CubeSprites)
	{
		Cube->SetSprite(Sprite);
	}
}

void AQBertPyramid::FlashCubes(int32 Round, int32 FlashCount, float Interval, FSimpleDelegate OnFinished)
{
	FlashRound = Round;
	FlashStepsRemaining = FlashCount;
	bFlashShowingTarget = false;
	FlashFinished = MoveTemp(OnFinished);

	TickFlash();
	if (FlashStepsRemaining > 0)
	{
		GetWorldTimerManager().SetTimer(FlashTimer, this, &AQBertPyramid::TickFlash, Interval, true);
	}
}

void AQBertPyramid::TickFlash()
{
	if (FlashStepsRemaining <= 0)
	{
		GetWorldTimerManager().ClearTimer(FlashTimer);
		ResetCubes(FlashRound);

		FSimpleDelegate Finished = MoveTemp(FlashFinished);
		FlashFinished.Unbind();
		Finished.ExecuteIfBound();
		return;
	}

	bFlashShowingTarget = !bFlashShowingTarget;
	const FQBertRoundTheme& Theme = GetTheme(FlashRound);
	SetAllCubes(bFlashShowingTarget ? Theme.TargetCube : Theme.StartCube);
	--FlashStepsRemaining;
}

void AQBertPyramid::SpawnDiscs(int32 Round)
{
	ClearDiscs();
	if (!DiscClass)
	{
		return;
	}

	const FQBertRoundTheme& Theme = GetTheme(Round);
	SpawnDiscsOnSide(LeftDiscCells, Theme);
	SpawnDiscsOnSide(RightDiscCells, Theme);
}

void AQBertPyramid::SpawnDiscsOnSide(const TArray<FIntPoint>& Candidates, const FQBertRoundTheme& Theme)
{
	TArray<FIntPoint> Shuffled = Candidates;
	for (int32 Index = Shuffled.Num() - 1; Index > 0; --Index)
	{
		Shuffled.Swap(Index, FMath::RandRange(0, Index));
	}

	const int32 Count = FMath::Min(DiscsPerSide, Shuffled.Num());
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FIntPoint Cell = Shuffled[Index];

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AQBertDisc* Disc = GetWorld()->SpawnActor<AQBertDisc>(DiscClass, GetCellLocation(Cell), FRotator::ZeroRotator, Params))
		{
			Disc->Initialize(Cell, Theme.DiscFlipbook);
			Discs.Add(Disc);
		}
	}
}

void AQBertPyramid::ClearDiscs()
{
	for (AQBertDisc* Disc : Discs)
	{
		if (IsValid(Disc))
		{
			Disc->Destroy();
		}
	}
	Discs.Reset();
}

void AQBertPyramid::RemoveDisc(AQBertDisc* Disc)
{
	Discs.Remove(Disc);
	if (IsValid(Disc))
	{
		Disc->Destroy();
	}
}

AQBertDisc* AQBertPyramid::GetDiscAt(const FIntPoint& Cell) const
{
	for (AQBertDisc* Disc : Discs)
	{
		if (IsValid(Disc) && Disc->GetCell() == Cell)
		{
			return Disc;
		}
	}
	return nullptr;
}

FVector AQBertPyramid::GetDiscDropOffLocation() const
{
	return GetCellLocation(GetTopCell()) + FVector(0.f, 0.f, DiscDropOffHeight);
}
