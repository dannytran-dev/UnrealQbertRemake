#include "UI/QBertNameEntryWidget.h"
#include "Core/QBertHighScoreSubsystem.h"
#include "Core/QBertPlayerController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void UQBertNameEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SubmitButton->OnClicked.AddDynamic(this, &UQBertNameEntryWidget::HandleSubmitClicked);
	NameTextBox->OnTextCommitted.AddDynamic(this, &UQBertNameEntryWidget::HandleTextCommitted);
}

void UQBertNameEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NameTextBox->SetText(FText::GetEmpty());
}

FReply UQBertNameEntryWidget::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	// Send focus straight to the text box so the player can start typing.
	return FReply::Handled().SetUserFocus(NameTextBox->TakeWidget(), InFocusEvent.GetCause());
}

void UQBertNameEntryWidget::HandleSubmitClicked()
{
	Submit();
}

void UQBertNameEntryWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		Submit();
	}
}

void UQBertNameEntryWidget::Submit()
{
	if (bSubmitted)
	{
		return;
	}
	bSubmitted = true;

	FString Name = NameTextBox->GetText().ToString().TrimStartAndEnd().Left(MaxNameLength);
	if (Name.IsEmpty())
	{
		Name = DefaultName;
	}

	if (UQBertHighScoreSubsystem* HighScores = GetGameInstance()->GetSubsystem<UQBertHighScoreSubsystem>())
	{
		HighScores->SubmitScore(Name, Score);
	}

	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->HandleNameEntryFinished();
	}
}
