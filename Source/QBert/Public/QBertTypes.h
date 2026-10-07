#pragma once

#include "CoreMinimal.h"
#include "QBertTypes.generated.h"

class UPaperSprite;
class UPaperFlipbook;

/** The four diagonal hops available on the pyramid. */
UENUM(BlueprintType)
enum class EQBertDirection : uint8
{
	UpLeft,
	UpRight,
	DownLeft,
	DownRight
};

namespace QBertGrid
{
	/**
	 * The pyramid uses a skewed grid: X runs along a row, Y climbs towards the apex.
	 * Every hop changes Y by one row, and moving "left" or "right" on screen is encoded in how X changes.
	 */
	inline FIntPoint GetDelta(EQBertDirection Direction)
	{
		switch (Direction)
		{
		case EQBertDirection::UpLeft:   return FIntPoint(-1, 1);
		case EQBertDirection::UpRight:  return FIntPoint(0, 1);
		case EQBertDirection::DownLeft: return FIntPoint(0, -1);
		default:                        return FIntPoint(1, -1);
		}
	}

	inline bool IsLeftward(EQBertDirection Direction)
	{
		return Direction == EQBertDirection::UpLeft || Direction == EQBertDirection::DownLeft;
	}

	inline constexpr EQBertDirection AllDirections[] =
	{
		EQBertDirection::UpLeft, EQBertDirection::UpRight, EQBertDirection::DownLeft, EQBertDirection::DownRight
	};
}

/** Visual set used by the pyramid, the discs and the scoreboard for one round. */
USTRUCT(BlueprintType)
struct FQBertRoundTheme
{
	GENERATED_BODY()

	/** Cube top before Q*bert has stepped on it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
	TObjectPtr<UPaperSprite> StartCube;

	/** Cube top the player has to turn every cube into. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
	TObjectPtr<UPaperSprite> TargetCube;

	/** Small "change to" cube shown on the scoreboard. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
	TObjectPtr<UPaperSprite> ChangeToIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Theme")
	TObjectPtr<UPaperFlipbook> DiscFlipbook;
};

USTRUCT(BlueprintType)
struct FQBertScoreEntry
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "High Score")
	FString Name;

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "High Score")
	int32 Score = 0;
};
