#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QBertGameMode.generated.h"

class AQBertPyramid;
class AQBertEnemy;
class AQBertEnemySpawner;
class AQBertPlayerCharacter;
class AQBertPlayerController;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FQBertIntChangedSignature, int32, NewValue);

/**
 * Rules of a game of Q*bert: score, lives and rounds, plus the sequences that play out when a round
 * is cleared, Q*bert is caught or falls, and the game ends.
 */
UCLASS()
class QBERT_API AQBertGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AQBertGameMode();

	virtual void StartPlay() override;

	UFUNCTION(BlueprintPure, Category = "QBert")
	int32 GetScore() const { return Score; }

	UFUNCTION(BlueprintPure, Category = "QBert")
	int32 GetLives() const { return Lives; }

	UFUNCTION(BlueprintPure, Category = "QBert")
	int32 GetRound() const { return Round; }

	bool IsHopInputAllowed() const { return bHopInputAllowed && !bGameOver; }

	AQBertPyramid* GetPyramid() const;
	AQBertPlayerCharacter* GetPlayerCharacter() const;
	AQBertEnemySpawner* GetSpawner() const { return Spawner; }

	UFUNCTION(BlueprintCallable, Category = "QBert")
	void AddScore(int32 Points);

	/** Enemies only start appearing once Q*bert makes his first hop of a life. */
	void NotifyPlayerHopped();

	void HandlePlayerLanded(AQBertPlayerCharacter* Player, const FIntPoint& Cell);
	void HandlePlayerFell(AQBertPlayerCharacter* Player);
	void HandlePlayerCaught(AQBertPlayerCharacter* Player, AQBertEnemy* Enemy);
	void HandleGreenBallCollected(AQBertEnemy* GreenBall);
	void HandleCoilyLured(AQBertEnemy* Coily);
	void HandleCoilyFell(AQBertEnemy* Coily);

	/** Debug: finish the current round immediately. */
	void CompleteRound();

	UPROPERTY(BlueprintAssignable, Category = "QBert")
	FQBertIntChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "QBert")
	FQBertIntChangedSignature OnLivesChanged;

	UPROPERTY(BlueprintAssignable, Category = "QBert")
	FQBertIntChangedSignature OnRoundChanged;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Classes")
	TSubclassOf<AQBertEnemySpawner> SpawnerClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Rules", meta = (ClampMin = "1"))
	int32 StartingLives = 3;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Rules", meta = (ClampMin = "1"))
	int32 NumRounds = 4;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Scoring")
	int32 CubeScore = 25;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Scoring")
	int32 RoundBonus = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Scoring")
	int32 UnusedDiscBonus = 100;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Scoring")
	int32 GreenBallScore = 100;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Scoring")
	int32 CoilyScore = 500;

	/** How long a green ball freezes the other enemies. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Rules")
	float FreezeDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Pyramid")
	int32 FlashCount = 21;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Pyramid")
	float FlashInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> RoundStartSound;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> RoundCompleteSound;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> GreenBallSound;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> GameOverSound;

	/** Indexed by lives remaining minus one, so the last life gets the last entry. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TArray<TObjectPtr<USoundBase>> CaughtSounds;

private:
	void BeginRound(bool bPlayIntroFlash);
	void FinishRound();
	void ClearEnemies();
	void LoseLife();
	void EndGame(bool bWon);
	void PlaySound(USoundBase* Sound) const;
	void RunAfter(float Delay, TFunction<void()>&& Callback);

	AQBertPlayerController* GetQBertController() const;

	UPROPERTY(Transient)
	TObjectPtr<AQBertEnemySpawner> Spawner;

	mutable TWeakObjectPtr<AQBertPyramid> CachedPyramid;

	int32 Score = 0;
	int32 Lives = 3;
	int32 Round = 1;
	bool bHopInputAllowed = false;
	bool bPlayerHasMoved = false;
	bool bGameOver = false;
};
