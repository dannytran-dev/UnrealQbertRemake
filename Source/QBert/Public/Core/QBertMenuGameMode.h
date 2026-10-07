#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "QBertMenuGameMode.generated.h"

class USoundBase;

/** Front-end: no pawn, just the main menu and the theme tune. */
UCLASS()
class QBERT_API AQBertMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AQBertMenuGameMode();

	virtual void StartPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> MenuMusic;
};
