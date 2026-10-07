#include "Characters/QBertCharacter.h"
#include "Characters/QBertMoverComponent.h"
#include "World/QBertPyramid.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"
#include "TimerManager.h"

AQBertCharacter::AQBertCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AutoPossessAI = EAutoPossessAI::Disabled;

	Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	Collision->InitCapsuleSize(CollisionRadius, CollisionRadius);
	Collision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;

	Sprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Collision);
	Sprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mover = CreateDefaultSubobject<UQBertMoverComponent>(TEXT("Mover"));
}

void AQBertCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Collision->SetCapsuleSize(CollisionRadius, CollisionRadius);
	ShowIdle();
}

void AQBertCharacter::BeginPlay()
{
	Super::BeginPlay();
	ShowIdle();
}

AQBertPyramid* AQBertCharacter::GetPyramid() const
{
	if (!CachedPyramid.IsValid())
	{
		CachedPyramid = AQBertPyramid::FindInWorld(this);
	}
	return CachedPyramid.Get();
}

FVector AQBertCharacter::GetRestLocation(const FIntPoint& InCell) const
{
	const AQBertPyramid* Pyramid = GetPyramid();
	return Pyramid ? Pyramid->GetCellLocation(InCell) + GetLandingOffset() : GetActorLocation();
}

void AQBertCharacter::PlaceAtCell(const FIntPoint& NewCell)
{
	Mover->Stop();
	GetWorldTimerManager().ClearAllTimersForObject(this);

	Cell = NewCell;
	bIsHopping = false;
	bIsFalling = false;
	SetActorLocation(GetRestLocation(Cell));
	ShowIdle();
}

void AQBertCharacter::Hop(EQBertDirection Direction)
{
	AQBertPyramid* Pyramid = GetPyramid();
	if (!Pyramid)
	{
		return;
	}

	const FIntPoint FromCell = Cell;
	Facing = Direction;
	Cell += QBertGrid::GetDelta(Direction);
	bIsHopping = true;
	ShowJump();

	OnHopStarted(FromCell);

	if (!Pyramid->IsCubeCell(Cell))
	{
		if (!HandleHopOffPyramid(Direction))
		{
			FallOff();
		}
		return;
	}

	RunAfter(HopWindup, [this]()
	{
		PlaySound(GetHopSound());
		Mover->Hop(GetRestLocation(Cell), HopDuration, HopArcOffset,
			FSimpleDelegate::CreateUObject(this, &AQBertCharacter::HandleHopFinished));
	});
}

void AQBertCharacter::HandleHopFinished()
{
	bIsHopping = false;
	ShowIdle();
	OnHopLanded();
}

void AQBertCharacter::FallOff()
{
	bIsHopping = false;
	bIsFalling = true;
	ShowIdle();
	PlaySound(FallSound);
	OnStartedFalling();

	// Arc outwards over the edge, then drop behind the pyramid.
	const FVector MissedCell = GetPyramid()->GetCellLocation(Cell);
	const float Side = QBertGrid::IsLeftward(Facing) ? -10.f : 10.f;
	const FVector ControlPoint = MissedCell + FVector(Side, 0.f, 10.f);

	Mover->MoveAlongCurve(ControlPoint, MissedCell + FallEndOffset, FallDuration, FSimpleDelegate::CreateWeakLambda(this, [this]()
	{
		bIsFalling = false;
		OnFellOffPyramid();
	}));
}

UPaperFlipbook* AQBertCharacter::GetJumpFlipbook(EQBertDirection Direction) const
{
	const TObjectPtr<UPaperFlipbook>* Found = JumpFlipbooks.Find(Direction);
	return Found && *Found ? Found->Get() : DefaultJumpFlipbook.Get();
}

UPaperFlipbook* AQBertCharacter::GetIdleFlipbook(EQBertDirection Direction) const
{
	const TObjectPtr<UPaperFlipbook>* Found = IdleFlipbooks.Find(Direction);
	return Found && *Found ? Found->Get() : DefaultIdleFlipbook.Get();
}

void AQBertCharacter::ShowIdle()
{
	if (UPaperFlipbook* Flipbook = GetIdleFlipbook(Facing))
	{
		Sprite->SetFlipbook(Flipbook);
	}
}

void AQBertCharacter::ShowJump()
{
	if (UPaperFlipbook* Flipbook = GetJumpFlipbook(Facing))
	{
		Sprite->SetFlipbook(Flipbook);
	}
}

void AQBertCharacter::PlaySound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void AQBertCharacter::RunAfter(float Delay, TFunction<void()>&& Callback)
{
	if (Delay <= 0.f)
	{
		Callback();
		return;
	}

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, MoveTemp(Callback)), Delay, false);
}
