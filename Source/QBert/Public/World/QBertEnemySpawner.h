#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QBertEnemySpawner.generated.h"

class AQBertEnemy;

/**
 * Drops enemies onto the second row of the pyramid on a random timer while active.
 * The game mode activates it on Q*bert's first hop and resets it after a death or a new round.
 */
UCLASS()
class QBERT_API AQBertEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AQBertEnemySpawner();

	void Activate();
	void Deactivate();

	/** Deactivates and allows Coily to be spawned again. */
	void ResetSpawner();

	void NotifyCoilyRemoved() { bCoilyInPlay = false; }

	/** Debug helper: stops new enemies appearing without resetting anything. */
	void SetSpawningEnabled(bool bEnabled) { bSpawningEnabled = bEnabled; }
	bool IsSpawningEnabled() const { return bSpawningEnabled; }

	AQBertEnemy* SpawnEnemy(TSubclassOf<AQBertEnemy> EnemyClass, const FIntPoint& Cell);
	AQBertEnemy* SpawnEnemyAtRandomCell(TSubclassOf<AQBertEnemy> EnemyClass);

	TSubclassOf<AQBertEnemy> GetRedBallClass() const { return RedBallClass; }
	TSubclassOf<AQBertEnemy> GetGreenBallClass() const { return GreenBallClass; }
	TSubclassOf<AQBertEnemy> GetCoilyClass() const { return CoilyClass; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	TSubclassOf<AQBertEnemy> RedBallClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	TSubclassOf<AQBertEnemy> GreenBallClass;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	TSubclassOf<AQBertEnemy> CoilyClass;

	/** Whole seconds between spawns are picked uniformly from this range. */
	UPROPERTY(EditDefaultsOnly, Category = "Spawner", meta = (ClampMin = "0"))
	int32 MinSpawnInterval = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner", meta = (ClampMin = "0"))
	int32 MaxSpawnInterval = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner", meta = (ClampMin = "0", ClampMax = "1"))
	float GreenBallChance = 0.25f;

	/** Seconds after the first ball before Coily appears. */
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	float CoilyDelay = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	TArray<FIntPoint> SpawnCells = { {1, 6}, {2, 6} };

private:
	void ScheduleNextSpawn();
	void SpawnNext();

	FTimerHandle SpawnTimer;
	FTimerHandle CoilyTimer;
	bool bActive = false;
	bool bCoilyInPlay = false;
	bool bSpawningEnabled = true;
};
