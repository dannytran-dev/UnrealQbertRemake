#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QBertMenuWidgets.generated.h"

class UButton;
class UPanelWidget;
class UTextBlock;
class AQBertPlayerController;
struct FQBertScoreEntry;

/** Base for menu screens: gives quick access to the owning controller. Layouts live in Widget Blueprints. */
UCLASS(Abstract)
class QBERT_API UQBertMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Menus take keyboard / gamepad focus so their buttons can be navigated without a mouse. */
	virtual void NativeOnInitialized() override;

	AQBertPlayerController* GetQBertController() const;
};

UCLASS(Abstract)
class QBERT_API UQBertMainMenuWidget : public UQBertMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HighScoresButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

private:
	UFUNCTION()
	void HandlePlayClicked();

	UFUNCTION()
	void HandleHighScoresClicked();

	UFUNCTION()
	void HandleQuitClicked();
};

UCLASS(Abstract)
class QBERT_API UQBertPauseMenuWidget : public UQBertMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

private:
	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleMainMenuClicked();
};

/** One row of the high-score table. */
UCLASS(Abstract)
class QBERT_API UQBertHighScoreRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(int32 Rank, const FQBertScoreEntry& Entry);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RankText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;
};

UCLASS(Abstract)
class QBERT_API UQBertHighScoresWidget : public UQBertMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> ScoreList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(EditDefaultsOnly, Category = "High Scores")
	TSubclassOf<UQBertHighScoreRowWidget> RowWidgetClass;

private:
	UFUNCTION()
	void HandleMainMenuClicked();
};
