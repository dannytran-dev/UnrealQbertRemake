#pragma once

#include "CoreMinimal.h"
#include "Characters/QBertCharacter.h"
#include "QBertPlayerCharacter.generated.h"

class AQBertDisc;
class UInputAction;
class UPaperSprite;
class UPaperSpriteComponent;

/** Q*bert himself: hops on player input, changes cube colours and can escape on discs. */
UCLASS()
class QBERT_API AQBertPlayerCharacter : public AQBertCharacter
{
	GENERATED_BODY()

public:
	AQBertPlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PlaceAtCell(const FIntPoint& NewCell) override;

	/** Tries to hop in a direction; ignored while busy or while the game mode has input locked. */
	UFUNCTION(BlueprintCallable, Category = "QBert")
	void RequestHop(EQBertDirection Direction);

	/** The cube Q*bert last stood on. Equals GetCell() except mid-hop or while riding a disc. */
	FIntPoint GetDepartureCell() const { return DepartureCell; }

	bool IsRidingDisc() const { return bRidingDisc; }
	bool IsKnockedOut() const { return bKnockedOut; }

	/** Enemies only hurt Q*bert while he is on the pyramid and not already knocked out. */
	bool CanBeCaught() const;

	void SetKnockedOut(bool bInKnockedOut) { bKnockedOut = bInKnockedOut; }
	void ShowSpeechBubble(bool bVisible);

protected:
	virtual void BeginPlay() override;
	virtual void OnHopStarted(const FIntPoint& FromCell) override;
	virtual bool HandleHopOffPyramid(EQBertDirection Direction) override;
	virtual void OnHopLanded() override;
	virtual void OnFellOffPyramid() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpeechBubble;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Input")
	TObjectPtr<UInputAction> HopUpLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Input")
	TObjectPtr<UInputAction> HopUpRightAction;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Input")
	TObjectPtr<UInputAction> HopDownLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Input")
	TObjectPtr<UInputAction> HopDownRightAction;

	/** The "@!#?@!" bubble shown when Q*bert gets caught. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Animation")
	TObjectPtr<UPaperSprite> SpeechBubbleSprite;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Audio")
	TObjectPtr<USoundBase> DiscSound;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Disc")
	float DiscBoardDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Disc")
	float DiscRideDuration = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Disc")
	float DiscDismountDuration = 0.5f;

	/** Where Q*bert stands on a disc; X is mirrored for discs on the right-hand side. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Disc")
	FVector DiscStandOffset = FVector(1.f, 1.f, 6.f);

private:
	void BoardDisc(AQBertDisc* Disc);
	void OnDiscArrived(TWeakObjectPtr<AQBertDisc> Disc);
	void OnDismounted(TWeakObjectPtr<AQBertDisc> Disc);

	void HopUpLeft() { RequestHop(EQBertDirection::UpLeft); }
	void HopUpRight() { RequestHop(EQBertDirection::UpRight); }
	void HopDownLeft() { RequestHop(EQBertDirection::DownLeft); }
	void HopDownRight() { RequestHop(EQBertDirection::DownRight); }

	FIntPoint DepartureCell = FIntPoint(1, 7);
	bool bRidingDisc = false;
	bool bKnockedOut = false;
};
