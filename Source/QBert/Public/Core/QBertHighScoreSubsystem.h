#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QBertTypes.h"
#include "QBertHighScoreSubsystem.generated.h"

UCLASS()
class QBERT_API UQBertHighScoreSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame)
	TArray<FQBertScoreEntry> Entries;
};

/** Loads, ranks and saves the local high-score table. */
UCLASS()
class QBERT_API UQBertHighScoreSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Best first. */
	UFUNCTION(BlueprintPure, Category = "QBert|High Scores")
	TArray<FQBertScoreEntry> GetEntries() const { return Entries; }

	/** Inserts the score in rank order (ahead of equal scores), trims the table and saves it. */
	UFUNCTION(BlueprintCallable, Category = "QBert|High Scores")
	void SubmitScore(const FString& Name, int32 Score);

	static constexpr int32 MaxEntries = 10;

private:
	void Save() const;

	TArray<FQBertScoreEntry> Entries;
};
