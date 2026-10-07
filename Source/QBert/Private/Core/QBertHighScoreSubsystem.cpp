#include "Core/QBertHighScoreSubsystem.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const FString SlotName = TEXT("HighScores");
	constexpr int32 UserIndex = 0;
}

void UQBertHighScoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (const UQBertHighScoreSave* Loaded = Cast<UQBertHighScoreSave>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex)))
	{
		Entries = Loaded->Entries;
	}
}

void UQBertHighScoreSubsystem::SubmitScore(const FString& Name, int32 Score)
{
	FQBertScoreEntry NewEntry;
	NewEntry.Name = Name;
	NewEntry.Score = Score;

	const int32 InsertAt = Entries.IndexOfByPredicate([Score](const FQBertScoreEntry& Entry) { return Entry.Score <= Score; });
	Entries.Insert(NewEntry, InsertAt == INDEX_NONE ? Entries.Num() : InsertAt);

	if (Entries.Num() > MaxEntries)
	{
		Entries.SetNum(MaxEntries);
	}
	Save();
}

void UQBertHighScoreSubsystem::Save() const
{
	if (UQBertHighScoreSave* SaveObject = Cast<UQBertHighScoreSave>(UGameplayStatics::CreateSaveGameObject(UQBertHighScoreSave::StaticClass())))
	{
		SaveObject->Entries = Entries;
		UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, UserIndex);
	}
}
