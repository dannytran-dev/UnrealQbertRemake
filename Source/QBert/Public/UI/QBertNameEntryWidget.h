#pragma once

#include "CoreMinimal.h"
#include "UI/QBertMenuWidgets.h"
#include "Types/SlateEnums.h"
#include "QBertNameEntryWidget.generated.h"

class UEditableTextBox;

/** Shown at the end of a game: records the player's name against their score. */
UCLASS(Abstract)
class QBERT_API UQBertNameEntryWidget : public UQBertMenuWidget
{
	GENERATED_BODY()

public:
	void SetScore(int32 InScore) { Score = InScore; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> NameTextBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SubmitButton;

	/** Used when the player leaves the box empty. */
	UPROPERTY(EditDefaultsOnly, Category = "Name Entry")
	FString DefaultName = TEXT("QBERT");

	UPROPERTY(EditDefaultsOnly, Category = "Name Entry", meta = (ClampMin = "1"))
	int32 MaxNameLength = 12;

private:
	UFUNCTION()
	void HandleSubmitClicked();

	UFUNCTION()
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void Submit();

	int32 Score = 0;
	bool bSubmitted = false;
};
