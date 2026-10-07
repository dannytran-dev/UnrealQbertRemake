#include "World/QBertEnemySpawner.h"
#include "World/QBertPyramid.h"
#include "Characters/QBertEnemy.h"
#include "TimerManager.h"

AQBertEnemySpawner::AQBertEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AQBertEnemySpawner::Activate()
{
	if (bActive)
	{
		return;
	}
	bActive = true;
	ScheduleNextSpawn();
}

void AQBertEnemySpawner::Deactivate()
{
	bActive = false;
	GetWorldTimerManager().ClearTimer(SpawnTimer);
	GetWorldTimerManager().ClearTimer(CoilyTimer);
}

void AQBertEnemySpawner::ResetSpawner()
{
	Deactivate();
	bCoilyInPlay = false;
}

void AQBertEnemySpawner::ScheduleNextSpawn()
{
	const float Interval = static_cast<float>(FMath::RandRange(MinSpawnInterval, FMath::Max(MinSpawnInterval, MaxSpawnInterval)));
	GetWorldTimerManager().SetTimer(SpawnTimer, this, &AQBertEnemySpawner::SpawnNext, FMath::Max(Interval, 0.1f), false);
}

void AQBertEnemySpawner::SpawnNext()
{
	if (!bActive)
	{
		return;
	}

	if (bSpawningEnabled && SpawnCells.Num() > 0)
	{
		const FIntPoint Cell = SpawnCells[FMath::RandRange(0, SpawnCells.Num() - 1)];
		const bool bGreen = FMath::FRand() < GreenBallChance;
		SpawnEnemy(bGreen ? GreenBallClass : RedBallClass, Cell);

		// Coily follows the first ball of each life, from the same side.
		if (!bCoilyInPlay && CoilyClass)
		{
			bCoilyInPlay = true;
			GetWorldTimerManager().SetTimer(CoilyTimer, FTimerDelegate::CreateWeakLambda(this, [this, Cell]()
			{
				SpawnEnemy(CoilyClass, Cell);
			}), CoilyDelay, false);
		}
	}

	ScheduleNextSpawn();
}

AQBertEnemy* AQBertEnemySpawner::SpawnEnemy(TSubclassOf<AQBertEnemy> EnemyClass, const FIntPoint& Cell)
{
	const AQBertPyramid* Pyramid = AQBertPyramid::FindInWorld(this);
	if (!EnemyClass || !Pyramid)
	{
		return nullptr;
	}

	const FTransform SpawnTransform(Pyramid->GetCellLocation(Cell));
	AQBertEnemy* Enemy = GetWorld()->SpawnActorDeferred<AQBertEnemy>(EnemyClass, SpawnTransform, this, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Enemy)
	{
		Enemy->SetStartCell(Cell);
		Enemy->FinishSpawning(SpawnTransform);
	}
	return Enemy;
}

AQBertEnemy* AQBertEnemySpawner::SpawnEnemyAtRandomCell(TSubclassOf<AQBertEnemy> EnemyClass)
{
	if (SpawnCells.IsEmpty())
	{
		return nullptr;
	}
	return SpawnEnemy(EnemyClass, SpawnCells[FMath::RandRange(0, SpawnCells.Num() - 1)]);
}
