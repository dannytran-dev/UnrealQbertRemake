#pragma once

#include "CoreMinimal.h"
#include "Characters/QBertEnemy.h"
#include "QBertCoily.generated.h"

/**
 * Coily starts as a purple ball bouncing down the pyramid, hatches at the bottom and then chases
 * Q*bert one cube at a time. He can be lured off the edge by riding a disc: if Coily reaches the
 * cube Q*bert jumped from, he follows him into open air.
 */
UCLASS()
class QBERT_API AQBertCoily : public AQBertEnemy
{
	GENERATED_BODY()

public:
	AQBertCoily();

protected:
	virtual void Think() override;
	virtual void OnHopLanded() override;
	virtual void OnStartedFalling() override;
	virtual void OnFellOffPyramid() override;

	virtual UPaperFlipbook* GetJumpFlipbook(EQBertDirection Direction) const override;
	virtual UPaperFlipbook* GetIdleFlipbook(EQBertDirection Direction) const override;
	virtual USoundBase* GetHopSound() const override;
	virtual FVector GetLandingOffset() const override;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Coily")
	TObjectPtr<UPaperFlipbook> EggJumpFlipbook;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Coily")
	TObjectPtr<UPaperFlipbook> EggIdleFlipbook;

	UPROPERTY(EditDefaultsOnly, Category = "QBert|Coily")
	TObjectPtr<USoundBase> EggHopSound;

	/** Hops made as a ball before hatching (5 takes him from the spawn row to the bottom row). */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Coily")
	int32 HopsBeforeHatching = 5;

	/** Extra pause at the bottom while hatching. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Coily")
	float HatchDuration = 4.f;

	/** The snake sprite is taller, so it stands a little higher on the cube than the ball. */
	UPROPERTY(EditDefaultsOnly, Category = "QBert|Coily")
	FVector SnakeLandingOffset = FVector(0.f, 2.f, 20.f);

private:
	enum class EPhase : uint8 { Egg, Hatching, Snake };

	bool ChooseChaseDirection(EQBertDirection& OutDirection) const;

	EPhase Phase = EPhase::Egg;
	int32 EggHops = 0;
};
