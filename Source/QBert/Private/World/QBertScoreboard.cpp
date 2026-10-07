#include "World/QBertScoreboard.h"
#include "World/QBertPyramid.h"
#include "Core/QBertGameMode.h"
#include "PaperFlipbookComponent.h"
#include "PaperSpriteComponent.h"
#include "TimerManager.h"

namespace
{
	constexpr int32 NumScoreDigits = 5;
	constexpr int32 NumLifeIcons = 3;
}

AQBertScoreboard::AQBertScoreboard()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	for (int32 Index = 0; Index < NumScoreDigits; ++Index)
	{
		const float X = -20.f + 10.f * Index;
		ScoreDigits.Add(CreateSprite(*FString::Printf(TEXT("ScoreDigit%d"), Index), Root, FVector(X, 0.f, 0.f)));
	}

	PlayerLabel = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PlayerLabel"));
	PlayerLabel->SetupAttachment(Root);
	PlayerLabel->SetRelativeLocation(FVector(-5.f, 0.f, 10.f));
	PlayerLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PlayerNumber = CreateSprite(TEXT("PlayerNumber"), Root, FVector(30.f, 0.f, 10.f));
	ChangeToLabel = CreateSprite(TEXT("ChangeToLabel"), Root, FVector(0.f, 0.f, -10.f));
	ChangeToCube = CreateSprite(TEXT("ChangeToCube"), Root, FVector(0.f, 0.f, -20.f));

	for (int32 Index = 0; Index < NumLifeIcons; ++Index)
	{
		LifeIcons.Add(CreateSprite(*FString::Printf(TEXT("Life%d"), Index), Root, FVector(0.f, 0.f, -40.f - 10.f * Index)));
	}

	const float ArrowX[] = { -25.f, -15.f, 15.f, 25.f };
	for (int32 Index = 0; Index < 4; ++Index)
	{
		UPaperSpriteComponent* Arrow = CreateSprite(*FString::Printf(TEXT("Arrow%d"), Index), Root, FVector(ArrowX[Index], 0.f, -20.f));
		Arrow->SetVisibility(false);
		Arrows.Add(Arrow);
	}

	RoundPanel = CreateDefaultSubobject<USceneComponent>(TEXT("RoundPanel"));
	RoundPanel->SetupAttachment(Root);
	RoundPanel->SetRelativeLocation(FVector(270.f, -1.f, 0.f));

	LevelLabel = CreateSprite(TEXT("LevelLabel"), RoundPanel, FVector(0.f, 0.f, 10.f));
	RoundLabel = CreateSprite(TEXT("RoundLabel"), RoundPanel, FVector(0.f, 0.f, 0.f));
	LevelDigit = CreateSprite(TEXT("LevelDigit"), RoundPanel, FVector(10.f, 0.f, 10.f));
	RoundDigit = CreateSprite(TEXT("RoundDigit"), RoundPanel, FVector(10.f, 0.f, 0.f));
}

UPaperSpriteComponent* AQBertScoreboard::CreateSprite(FName Name, USceneComponent* Parent, const FVector& Location)
{
	UPaperSpriteComponent* Component = CreateDefaultSubobject<UPaperSpriteComponent>(Name);
	Component->SetupAttachment(Parent);
	Component->SetRelativeLocation(Location);
	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return Component;
}

void AQBertScoreboard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PlayerLabel->SetFlipbook(PlayerLabelFlipbook);
	PlayerNumber->SetSprite(PlayerNumberSprite);
	ChangeToLabel->SetSprite(ChangeToLabelSprite);
	LevelLabel->SetSprite(LevelLabelSprite);
	RoundLabel->SetSprite(RoundLabelSprite);

	for (UPaperSpriteComponent* Life : LifeIcons)
	{
		Life->SetSprite(LifeSprite);
	}
	for (int32 Index = 0; Index < Arrows.Num(); ++Index)
	{
		Arrows[Index]->SetSprite(Index < 2 ? LeftArrowSprite : RightArrowSprite);
	}

	SetDigit(LevelDigit, DisplayedLevel);
	SetScore(0);
	SetLives(NumLifeIcons + 1);
	SetRound(1);
}

void AQBertScoreboard::BeginPlay()
{
	Super::BeginPlay();

	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->OnScoreChanged.AddDynamic(this, &AQBertScoreboard::SetScore);
		GameMode->OnLivesChanged.AddDynamic(this, &AQBertScoreboard::SetLives);
		GameMode->OnRoundChanged.AddDynamic(this, &AQBertScoreboard::SetRound);

		SetScore(GameMode->GetScore());
		SetLives(GameMode->GetLives());
		SetRound(GameMode->GetRound());
	}

	GetWorldTimerManager().SetTimer(ArrowTimer, this, &AQBertScoreboard::StepArrowBlink, ArrowBlinkInterval, true);
}

void AQBertScoreboard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->OnScoreChanged.RemoveAll(this);
		GameMode->OnLivesChanged.RemoveAll(this);
		GameMode->OnRoundChanged.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void AQBertScoreboard::SetDigit(UPaperSpriteComponent* Component, int32 Digit) const
{
	if (DigitSprites.IsValidIndex(Digit))
	{
		Component->SetSprite(DigitSprites[Digit]);
	}
}

void AQBertScoreboard::SetScore(int32 Score)
{
	// Zero-padded, like the arcade cabinet.
	int32 Remaining = FMath::Clamp(Score, 0, 99999);
	for (int32 Index = ScoreDigits.Num() - 1; Index >= 0; --Index)
	{
		SetDigit(ScoreDigits[Index], Remaining % 10);
		Remaining /= 10;
	}
}

void AQBertScoreboard::SetLives(int32 Lives)
{
	// The life currently being played is not shown, only the spares.
	for (int32 Index = 0; Index < LifeIcons.Num(); ++Index)
	{
		LifeIcons[Index]->SetVisibility(Index < Lives - 1);
	}
}

void AQBertScoreboard::SetRound(int32 Round)
{
	SetDigit(RoundDigit, Round);

	if (const AQBertPyramid* Pyramid = AQBertPyramid::FindInWorld(this))
	{
		ChangeToCube->SetSprite(Pyramid->GetTheme(Round).ChangeToIcon);
	}
}

void AQBertScoreboard::StepArrowBlink()
{
	// Outer arrows, then inner arrows, then a blank beat.
	ArrowPhase = (ArrowPhase + 1) % 3;
	const bool bOuter = ArrowPhase == 1;
	const bool bInner = ArrowPhase == 2;

	Arrows[0]->SetVisibility(bOuter || bInner);
	Arrows[3]->SetVisibility(bOuter || bInner);
	Arrows[1]->SetVisibility(bInner);
	Arrows[2]->SetVisibility(bInner);
}
