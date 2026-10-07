#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QBertScoreboard.generated.h"

class UPaperSprite;
class UPaperFlipbook;
class UPaperSpriteComponent;
class UPaperFlipbookComponent;

/**
 * Arcade-style in-world HUD built from sprites: score, "change to" cube, remaining lives on the
 * left of the screen and level / round on the right. Listens to the game mode for updates.
 */
UCLASS()
class QBERT_API AQBertScoreboard : public AActor
{
	GENERATED_BODY()

public:
	AQBertScoreboard();

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Sprites for the digits 0-9, in order. */
	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TArray<TObjectPtr<UPaperSprite>> DigitSprites;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperFlipbook> PlayerLabelFlipbook;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> PlayerNumberSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> ChangeToLabelSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> LifeSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> LeftArrowSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> RightArrowSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> LevelLabelSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	TObjectPtr<UPaperSprite> RoundLabelSprite;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	int32 DisplayedLevel = 1;

	UPROPERTY(EditAnywhere, Category = "Scoreboard")
	float ArrowBlinkInterval = 0.5f;

private:
	UFUNCTION()
	void SetScore(int32 Score);

	UFUNCTION()
	void SetLives(int32 Lives);

	UFUNCTION()
	void SetRound(int32 Round);

	void SetDigit(UPaperSpriteComponent* Component, int32 Digit) const;
	void StepArrowBlink();

	UPaperSpriteComponent* CreateSprite(FName Name, USceneComponent* Parent, const FVector& Location);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RoundPanel;

	/** Most significant digit first. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TArray<TObjectPtr<UPaperSpriteComponent>> ScoreDigits;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperFlipbookComponent> PlayerLabel;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> PlayerNumber;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> ChangeToLabel;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> ChangeToCube;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TArray<TObjectPtr<UPaperSpriteComponent>> LifeIcons;

	/** Outer left, inner left, inner right, outer right. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TArray<TObjectPtr<UPaperSpriteComponent>> Arrows;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> LevelLabel;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> RoundLabel;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> LevelDigit;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> RoundDigit;

	FTimerHandle ArrowTimer;
	int32 ArrowPhase = 0;
};
