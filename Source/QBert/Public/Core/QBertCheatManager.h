#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "QBertCheatManager.generated.h"

/**
 * Console commands for testing (development builds only). These replace the original project's
 * hard-wired debug keys.
 */
UCLASS()
class QBERT_API UQBertCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void QBertAddScore(int32 Points);

	/** Type is one of: Red, Green, Coily. */
	UFUNCTION(Exec)
	void QBertSpawn(const FString& Type);

	UFUNCTION(Exec)
	void QBertFinishRound();

	UFUNCTION(Exec)
	void QBertToggleSpawner();
};
