#include "World/QBertDisc.h"
#include "Characters/QBertMoverComponent.h"
#include "PaperFlipbookComponent.h"

AQBertDisc::AQBertDisc()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Sprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Root);
	Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mover = CreateDefaultSubobject<UQBertMoverComponent>(TEXT("Mover"));
}

void AQBertDisc::Initialize(const FIntPoint& InCell, UPaperFlipbook* Flipbook)
{
	Cell = InCell;
	if (Flipbook)
	{
		Sprite->SetFlipbook(Flipbook);
	}
}

void AQBertDisc::FlyTo(const FVector& Destination, float Duration, FSimpleDelegate OnArrived)
{
	Mover->MoveLinear(Destination, Duration, true, MoveTemp(OnArrived));
}
