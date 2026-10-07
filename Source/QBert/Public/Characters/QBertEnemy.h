#pragma once

#include "CoreMinimal.h"
#include "Characters/QBertCharacter.h"
#include "QBertEnemy.generated.h"

class AQBertPlayerCharacter;

/**
 * Base for everything the spawner drops onto the pyramid. Runs a simple think loop:
 * wait, pick a direction, hop, repeat. Can be frozen (green ball) or stopped (player caught).
 */
UCLASS(Abstract)
class QBERT_API AQBertEnemy : public AQBertCharacter
{
	GENERATED_BODY()

public:
	/** Must be called before FinishSpawning so BeginPlay knows where to drop in. */
	void SetStartCell(const FIntPoint& StartCell) { Cell = StartCell; }

	/** Called by Q*bert when the two overlap. Default behaviour: Q*bert loses a life. */
	virtual void HandleTouchedPlayer(AQBertPlayerCharacter* Player);

	/** Played alongside the swear when this enemy catches Q*bert. */
	virtual USoundBase* GetCatchSound() const { return CatchSound; }

	/** Holds the enemy in place; a hop already in progress still completes. */
	void Freeze(float Duration);

	/** Permanently stops thinking (used once Q*bert has been caught). */
	void StopMoving() { bStopped = true; }

protected:
	virtual void BeginPlay() override;
	virtual void OnHopLanded() override;
	virtual void OnFellOffPyramid() override;

	/** Pick a direction and call Hop(). Called whenever the enemy is ready to move. */
	virtual void Think() PURE_VIRTUAL(AQBertEnemy::Think, );

	void ScheduleThink(float Delay);

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Enemy")
	TObjectPtr<USoundBase> CatchSound;

	/** How high above its first cube the enemy appears before dropping in. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Enemy")
	float DropHeight = 40.f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Enemy")
	float DropDuration = 0.5f;

	/** Delay between spawning and the first hop. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Enemy")
	float FirstHopDelay = 1.f;

	/** Pause on each cube between hops. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Enemy")
	float RestBetweenHops = 0.5f;

private:
	void TryThink();

	FTimerHandle ThinkTimer;
	FTimerHandle FreezeTimer;
	bool bFrozen = false;
	bool bThinkPending = false;
	bool bStopped = false;
};

/** Red ball: bounces straight down the pyramid and off the bottom. Touching it costs a life. */
UCLASS()
class QBERT_API AQBertBall : public AQBertEnemy
{
	GENERATED_BODY()

public:
	AQBertBall();

protected:
	virtual void Think() override;
};

/** Green ball: harmless. Catching it scores points and freezes every other enemy. */
UCLASS()
class QBERT_API AQBertGreenBall : public AQBertBall
{
	GENERATED_BODY()

public:
	AQBertGreenBall();

	virtual void HandleTouchedPlayer(AQBertPlayerCharacter* Player) override;
};
