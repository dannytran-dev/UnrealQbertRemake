#include "UI/QBertMenuWidgets.h"
#include "Core/QBertHighScoreSubsystem.h"
#include "Core/QBertPlayerController.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UQBertMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetIsFocusable(true);
}

AQBertPlayerController* UQBertMenuWidget::GetQBertController() const
{
	return Cast<AQBertPlayerController>(GetOwningPlayer());
}

void UQBertMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayButton->OnClicked.AddDynamic(this, &UQBertMainMenuWidget::HandlePlayClicked);
	HighScoresButton->OnClicked.AddDynamic(this, &UQBertMainMenuWidget::HandleHighScoresClicked);
	QuitButton->OnClicked.AddDynamic(this, &UQBertMainMenuWidget::HandleQuitClicked);
}

void UQBertMainMenuWidget::HandlePlayClicked()
{
	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->StartGame();
	}
}

void UQBertMainMenuWidget::HandleHighScoresClicked()
{
	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->ShowHighScores();
	}
}

void UQBertMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UQBertPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ResumeButton->OnClicked.AddDynamic(this, &UQBertPauseMenuWidget::HandleResumeClicked);
	MainMenuButton->OnClicked.AddDynamic(this, &UQBertPauseMenuWidget::HandleMainMenuClicked);
}

void UQBertPauseMenuWidget::HandleResumeClicked()
{
	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->ResumeGame();
	}
}

void UQBertPauseMenuWidget::HandleMainMenuClicked()
{
	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->ReturnToMainMenu();
	}
}

void UQBertHighScoreRowWidget::Setup(int32 Rank, const FQBertScoreEntry& Entry)
{
	RankText->SetText(FText::Format(NSLOCTEXT("QBert", "RankFormat", "{0}."), FText::AsNumber(Rank)));
	NameText->SetText(FText::FromString(Entry.Name));
	ScoreText->SetText(FText::AsNumber(Entry.Score));
}

void UQBertHighScoresWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	MainMenuButton->OnClicked.AddDynamic(this, &UQBertHighScoresWidget::HandleMainMenuClicked);
}

void UQBertHighScoresWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ScoreList->ClearChildren();

	const UGameInstance* GameInstance = GetGameInstance();
	const UQBertHighScoreSubsystem* HighScores = GameInstance ? GameInstance->GetSubsystem<UQBertHighScoreSubsystem>() : nullptr;
	if (!HighScores || !RowWidgetClass)
	{
		return;
	}

	const TArray<FQBertScoreEntry> Entries = HighScores->GetEntries();
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (UQBertHighScoreRowWidget* Row = CreateWidget<UQBertHighScoreRowWidget>(this, RowWidgetClass))
		{
			Row->Setup(Index + 1, Entries[Index]);
			ScoreList->AddChild(Row);
		}
	}
}

void UQBertHighScoresWidget::HandleMainMenuClicked()
{
	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->ReturnToMainMenu();
	}
}
