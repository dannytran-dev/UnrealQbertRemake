#include "Core/QBertPlayerController.h"
#include "Core/QBertCheatManager.h"
#include "Core/QBertGameMode.h"
#include "UI/QBertNameEntryWidget.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

AQBertPlayerController::AQBertPlayerController()
{
	CheatClass = UQBertCheatManager::StaticClass();
}

void AQBertPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (IsInGameLevel())
	{
		SetGameInputMode();
	}
}

void AQBertPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			Input->BindAction(PauseAction, ETriggerEvent::Started, this, &AQBertPlayerController::TogglePauseMenu);
		}
	}
}

bool AQBertPlayerController::IsInGameLevel() const
{
	return GetWorld()->GetAuthGameMode<AQBertGameMode>() != nullptr;
}

UUserWidget* AQBertPlayerController::ShowWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(this, WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport();
	}
	return Widget;
}

void AQBertPlayerController::CloseWidget(TObjectPtr<UUserWidget>& Widget)
{
	if (Widget)
	{
		Widget->RemoveFromParent();
		Widget = nullptr;
	}
}

void AQBertPlayerController::SetMenuInputMode(UUserWidget* FocusWidget, bool bAllowGameInput)
{
	SetShowMouseCursor(true);

	if (bAllowGameInput)
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(FocusWidget ? FocusWidget->TakeWidget() : TSharedPtr<SWidget>());
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
	}
	else
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(FocusWidget ? FocusWidget->TakeWidget() : TSharedPtr<SWidget>());
		SetInputMode(Mode);
	}
}

void AQBertPlayerController::SetGameInputMode()
{
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}

void AQBertPlayerController::ShowMainMenu()
{
	CloseWidget(HighScoresWidget);
	if (!MainMenuWidget)
	{
		MainMenuWidget = ShowWidget(MainMenuWidgetClass);
	}
	SetMenuInputMode(MainMenuWidget, false);
}

void AQBertPlayerController::ShowHighScores()
{
	CloseWidget(MainMenuWidget);
	if (!HighScoresWidget)
	{
		HighScoresWidget = ShowWidget(HighScoresWidgetClass);
	}
	SetMenuInputMode(HighScoresWidget, false);
}

void AQBertPlayerController::StartGame()
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameLevel);
}

void AQBertPlayerController::ReturnToMainMenu()
{
	if (IsInGameLevel())
	{
		SetPause(false);
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuLevel);
	}
	else
	{
		ShowMainMenu();
	}
}

void AQBertPlayerController::TogglePauseMenu()
{
	if (!IsInGameLevel() || NameEntryWidget || HighScoresWidget)
	{
		return;
	}

	if (PauseMenuWidget)
	{
		ResumeGame();
		return;
	}

	PauseMenuWidget = ShowWidget(PauseMenuWidgetClass);
	SetPause(true);
	SetMenuInputMode(PauseMenuWidget, true);
}

void AQBertPlayerController::ResumeGame()
{
	CloseWidget(PauseMenuWidget);
	SetPause(false);
	SetGameInputMode();
}

void AQBertPlayerController::ShowEndScreen(bool bWon, int32 FinalScore)
{
	CloseWidget(PauseMenuWidget);
	SetPause(false);

	ResultWidget = ShowWidget(bWon ? VictoryWidgetClass : GameOverWidgetClass);

	UQBertNameEntryWidget* NameEntry = NameEntryWidgetClass ? CreateWidget<UQBertNameEntryWidget>(this, NameEntryWidgetClass) : nullptr;
	if (NameEntry)
	{
		NameEntry->SetScore(FinalScore);
		NameEntry->AddToViewport(1);
		NameEntryWidget = NameEntry;
	}
	SetMenuInputMode(NameEntryWidget, false);
}

void AQBertPlayerController::HandleNameEntryFinished()
{
	CloseWidget(NameEntryWidget);
	CloseWidget(ResultWidget);
	ShowHighScores();
}
