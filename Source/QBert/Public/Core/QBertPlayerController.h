#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "QBertPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;
class UQBertNameEntryWidget;

/** Owns input setup and every menu screen, in both the front-end and the game level. */
UCLASS()
class QBERT_API AQBertPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AQBertPlayerController();

	UFUNCTION(BlueprintCallable, Category = "QBert|UI")
	void ShowMainMenu();

	UFUNCTION(BlueprintCallable, Category = "QBert|UI")
	void ShowHighScores();

	UFUNCTION(BlueprintCallable, Category = "QBert|UI")
	void StartGame();

	/** From the game this loads the front-end; from the front-end it just swaps screens. */
	UFUNCTION(BlueprintCallable, Category = "QBert|UI")
	void ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "QBert|UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "QBert|UI")
	void ResumeGame();

	/** Game over / victory: shows the result banner and asks for a name for the high-score table. */
	void ShowEndScreen(bool bWon, int32 FinalScore);

	/** Called by the name entry widget once the score has been saved. */
	void HandleNameEntryFinished();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Should have "Trigger when Paused" enabled so the same key closes the menu. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|UI")
	TSubclassOf<UUserWidget> HighScoresWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|UI")
	TSubclassOf<UQBertNameEntryWidget> NameEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|UI")
	TSubclassOf<UUserWidget> VictoryWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Levels")
	TSoftObjectPtr<UWorld> GameLevel;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Levels")
	TSoftObjectPtr<UWorld> MainMenuLevel;

private:
	UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass);
	void CloseWidget(TObjectPtr<UUserWidget>& Widget);
	void SetMenuInputMode(UUserWidget* FocusWidget, bool bAllowGameInput);
	void SetGameInputMode();
	bool IsInGameLevel() const;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> HighScoresWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> NameEntryWidget;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> ResultWidget;
};
