#include "Characters/QBertPlayerCharacter.h"
#include "Characters/QBertEnemy.h"
#include "Characters/QBertMoverComponent.h"
#include "Core/QBertGameMode.h"
#include "World/QBertDisc.h"
#include "World/QBertPyramid.h"
#include "EnhancedInputComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperSpriteComponent.h"

AQBertPlayerCharacter::AQBertPlayerCharacter()
{
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	SpeechBubble = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpeechBubble"));
	SpeechBubble->SetupAttachment(Sprite);
	SpeechBubble->SetRelativeLocation(FVector(10.f, 0.f, 20.f));
	SpeechBubble->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpeechBubble->SetVisibility(false);
}

void AQBertPlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SpeechBubble->SetSprite(SpeechBubbleSprite);
}

void AQBertPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const AQBertPyramid* Pyramid = GetPyramid())
	{
		PlaceAtCell(Pyramid->GetTopCell());
	}
}

void AQBertPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (HopUpLeftAction)    { Input->BindAction(HopUpLeftAction, ETriggerEvent::Started, this, &AQBertPlayerCharacter::HopUpLeft); }
	if (HopUpRightAction)   { Input->BindAction(HopUpRightAction, ETriggerEvent::Started, this, &AQBertPlayerCharacter::HopUpRight); }
	if (HopDownLeftAction)  { Input->BindAction(HopDownLeftAction, ETriggerEvent::Started, this, &AQBertPlayerCharacter::HopDownLeft); }
	if (HopDownRightAction) { Input->BindAction(HopDownRightAction, ETriggerEvent::Started, this, &AQBertPlayerCharacter::HopDownRight); }
}

void AQBertPlayerCharacter::RequestHop(EQBertDirection Direction)
{
	AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>();
	if (!GameMode || !GameMode->IsHopInputAllowed())
	{
		return;
	}
	if (bIsHopping || bIsFalling || bRidingDisc || bKnockedOut)
	{
		return;
	}

	GameMode->NotifyPlayerHopped();
	Hop(Direction);
}

void AQBertPlayerCharacter::PlaceAtCell(const FIntPoint& NewCell)
{
	if (bRidingDisc)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		bRidingDisc = false;
	}

	Super::PlaceAtCell(NewCell);
	DepartureCell = NewCell;
}

bool AQBertPlayerCharacter::CanBeCaught() const
{
	return !bRidingDisc && !bKnockedOut && !bIsFalling && !IsHidden();
}

void AQBertPlayerCharacter::ShowSpeechBubble(bool bVisible)
{
	SpeechBubble->SetVisibility(bVisible);
}

void AQBertPlayerCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (AQBertEnemy* Enemy = Cast<AQBertEnemy>(OtherActor))
	{
		Enemy->HandleTouchedPlayer(this);
	}
}

void AQBertPlayerCharacter::OnHopStarted(const FIntPoint& FromCell)
{
	DepartureCell = FromCell;
}

bool AQBertPlayerCharacter::HandleHopOffPyramid(EQBertDirection Direction)
{
	AQBertDisc* Disc = GetPyramid()->GetDiscAt(Cell);
	if (!Disc)
	{
		return false;
	}

	BoardDisc(Disc);
	return true;
}

void AQBertPlayerCharacter::BoardDisc(AQBertDisc* InDisc)
{
	bRidingDisc = true;

	TWeakObjectPtr<AQBertDisc> WeakDisc = InDisc;
	RunAfter(HopWindup, [this, WeakDisc]()
	{
		AQBertDisc* Disc = WeakDisc.Get();
		if (!Disc)
		{
			return;
		}

		PlaySound(DiscSound);

		const bool bDiscOnLeft = QBertGrid::IsLeftward(Facing);
		const FVector StandOffset(bDiscOnLeft ? DiscStandOffset.X : -DiscStandOffset.X, DiscStandOffset.Y, DiscStandOffset.Z);

		Mover->Hop(Disc->GetActorLocation() + StandOffset, DiscBoardDuration, FVector(0.f, 0.f, 4.f),
			FSimpleDelegate::CreateWeakLambda(this, [this, WeakDisc]()
			{
				AQBertDisc* Disc = WeakDisc.Get();
				if (!Disc)
				{
					return;
				}

				AttachToActor(Disc, FAttachmentTransformRules::KeepWorldTransform);

				// Face back towards the pyramid for the ride up.
				Facing = QBertGrid::IsLeftward(Facing) ? EQBertDirection::DownRight : EQBertDirection::DownLeft;
				ShowIdle();

				Disc->FlyTo(GetPyramid()->GetDiscDropOffLocation(), DiscRideDuration,
					FSimpleDelegate::CreateUObject(this, &AQBertPlayerCharacter::OnDiscArrived, WeakDisc));
			}));
	});
}

void AQBertPlayerCharacter::OnDiscArrived(TWeakObjectPtr<AQBertDisc> Disc)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Cell = GetPyramid()->GetTopCell();
	ShowJump();

	Mover->MoveLinear(GetRestLocation(Cell), DiscDismountDuration, true,
		FSimpleDelegate::CreateUObject(this, &AQBertPlayerCharacter::OnDismounted, Disc));
}

void AQBertPlayerCharacter::OnDismounted(TWeakObjectPtr<AQBertDisc> Disc)
{
	bRidingDisc = false;
	bIsHopping = false;
	DepartureCell = Cell;
	ShowIdle();

	GetPyramid()->RemoveDisc(Disc.Get());

	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandlePlayerLanded(this, Cell);
	}
}

void AQBertPlayerCharacter::OnHopLanded()
{
	DepartureCell = Cell;

	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandlePlayerLanded(this, Cell);
	}
}

void AQBertPlayerCharacter::OnFellOffPyramid()
{
	if (AQBertGameMode* GameMode = GetWorld()->GetAuthGameMode<AQBertGameMode>())
	{
		GameMode->HandlePlayerFell(this);
	}
}
