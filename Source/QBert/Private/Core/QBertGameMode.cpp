#include "Core/QBertGameMode.h"
#include "Core/QBertPlayerController.h"
#include "Characters/QBertCoily.h"
#include "Characters/QBertEnemy.h"
#include "Characters/QBertPlayerCharacter.h"
#include "World/QBertEnemySpawner.h"
#include "World/QBertPyramid.h"
#include "QBert.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AQBertGameMode::AQBertGameMode()
{
	DefaultPawnClass = AQBertPlayerCharacter::StaticClass();
	PlayerControllerClass = AQBertPlayerController::StaticClass();
	SpawnerClass = AQBertEnemySpawner::StaticClass();
}

void AQBertGameMode::StartPlay()
{
	Super::StartPlay();

	Score = 0;
	Lives = StartingLives;
	Round = 1;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Spawner = GetWorld()->SpawnActor<AQBertEnemySpawner>(SpawnerClass ? SpawnerClass.Get() : AQBertEnemySpawner::StaticClass(), FTransform::Identity, Params);

	if (!GetPyramid())
	{
		UE_LOG(LogQBert, Error, TEXT("No AQBertPyramid in the level; the game cannot start."));
		return;
	}

	OnScoreChanged.Broadcast(Score);
	OnLivesChanged.Broadcast(Lives);
	OnRoundChanged.Broadcast(Round);

	PlaySound(RoundStartSound);
	BeginRound(true);
}

AQBertPyramid* AQBertGameMode::GetPyramid() const
{
	if (!CachedPyramid.IsValid())
	{
		CachedPyramid = AQBertPyramid::FindInWorld(this);
	}
	return CachedPyramid.Get();
}

AQBertPlayerCharacter* AQBertGameMode::GetPlayerCharacter() const
{
	const APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	return Controller ? Cast<AQBertPlayerCharacter>(Controller->GetPawn()) : nullptr;
}

AQBertPlayerController* AQBertGameMode::GetQBertController() const
{
	return Cast<AQBertPlayerController>(GetWorld()->GetFirstPlayerController());
}

void AQBertGameMode::AddScore(int32 Points)
{
	Score += Points;
	OnScoreChanged.Broadcast(Score);
}

void AQBertGameMode::BeginRound(bool bPlayIntroFlash)
{
	AQBertPyramid* Pyramid = GetPyramid();
	Pyramid->ResetCubes(Round);
	Pyramid->SpawnDiscs(Round);

	if (AQBertPlayerCharacter* Player = GetPlayerCharacter())
	{
		Player->PlaceAtCell(Pyramid->GetTopCell());
	}

	Spawner->ResetSpawner();
	bPlayerHasMoved = false;

	if (!bPlayIntroFlash)
	{
		bHopInputAllowed = true;
		return;
	}

	// The pyramid flashes when the level starts; Q*bert can't move until it stops.
	bHopInputAllowed = false;
	Pyramid->FlashCubes(Round, FlashCount, FlashInterval, FSimpleDelegate::CreateWeakLambda(this, [this]()
	{
		bHopInputAllowed = true;
	}));
}

void AQBertGameMode::NotifyPlayerHopped()
{
	if (!bPlayerHasMoved)
	{
		bPlayerHasMoved = true;
		Spawner->Activate();
	}
}

void AQBertGameMode::HandlePlayerLanded(AQBertPlayerCharacter* Player, const FIntPoint& Cell)
{
	AQBertPyramid* Pyramid = GetPyramid();
	if (bGameOver || !Pyramid->ActivateCube(Cell, Round))
	{
		return;
	}

	AddScore(CubeScore);
	if (Pyramid->AreAllCubesActivated(Round))
	{
		CompleteRound();
	}
}

void AQBertGameMode::CompleteRound()
{
	if (bGameOver)
	{
		return;
	}

	bHopInputAllowed = false;
	ClearEnemies();

	AQBertPyramid* Pyramid = GetPyramid();
	AddScore(RoundBonus + UnusedDiscBonus * Pyramid->GetDiscCount());
	Pyramid->ClearDiscs();

	PlaySound(RoundCompleteSound);
	Pyramid->FlashCubes(Round, FlashCount, FlashInterval, FSimpleDelegate::CreateUObject(this, &AQBertGameMode::FinishRound));
}

void AQBertGameMode::FinishRound()
{
	if (Round >= NumRounds)
	{
		EndGame(true);
		return;
	}

	++Round;
	OnRoundChanged.Broadcast(Round);
	BeginRound(false);
}

void AQBertGameMode::HandlePlayerCaught(AQBertPlayerCharacter* Player, AQBertEnemy* Enemy)
{
	if (bGameOver || !Player || !Player->CanBeCaught())
	{
		return;
	}

	Player->SetKnockedOut(true);
	bHopInputAllowed = false;
	Spawner->Deactivate();

	if (Enemy)
	{
		Enemy->StopMoving();
		PlaySound(Enemy->GetCatchSound());
	}
	if (CaughtSounds.Num() > 0)
	{
		PlaySound(CaughtSounds[FMath::Clamp(Lives - 1, 0, CaughtSounds.Num() - 1)]);
	}

	// Freeze briefly, clear the board, then show the swear bubble before Q*bert reappears.
	RunAfter(0.5f, [this]()
	{
		ClearEnemies();
		LoseLife();
		if (Lives <= 0)
		{
			EndGame(false);
			return;
		}

		RunAfter(0.5f, [this]()
		{
			if (AQBertPlayerCharacter* QBert = GetPlayerCharacter())
			{
				QBert->ShowSpeechBubble(true);
			}

			RunAfter(1.f, [this]()
			{
				if (AQBertPlayerCharacter* QBert = GetPlayerCharacter())
				{
					QBert->ShowSpeechBubble(false);
					QBert->SetActorHiddenInGame(true);
				}

				RunAfter(1.f, [this]()
				{
					if (AQBertPlayerCharacter* QBert = GetPlayerCharacter())
					{
						QBert->SetActorHiddenInGame(false);
						QBert->SetKnockedOut(false);
					}
					bPlayerHasMoved = false;
					bHopInputAllowed = true;
				});
			});
		});
	});
}

void AQBertGameMode::HandlePlayerFell(AQBertPlayerCharacter* Player)
{
	if (bGameOver || !Player)
	{
		return;
	}

	bHopInputAllowed = false;
	LoseLife();
	if (Lives <= 0)
	{
		EndGame(false);
		return;
	}

	// Put Q*bert back on the cube he jumped from and reset the board around him.
	Player->PlaceAtCell(Player->GetDepartureCell());
	Player->SetActorHiddenInGame(true);
	ClearEnemies();
	GetPyramid()->ClearDiscs();
	bPlayerHasMoved = false;

	RunAfter(1.f, [this]()
	{
		if (AQBertPlayerCharacter* QBert = GetPlayerCharacter())
		{
			QBert->SetActorHiddenInGame(false);
		}
		GetPyramid()->SpawnDiscs(Round);
		bHopInputAllowed = true;
	});
}

void AQBertGameMode::HandleGreenBallCollected(AQBertEnemy* GreenBall)
{
	AddScore(GreenBallScore);
	PlaySound(GreenBallSound);

	for (TActorIterator<AQBertEnemy> It(GetWorld()); It; ++It)
	{
		if (*It != GreenBall && !It->IsA(GreenBall->GetClass()))
		{
			It->Freeze(FreezeDuration);
		}
	}
}

void AQBertGameMode::HandleCoilyLured(AQBertEnemy* Coily)
{
	AddScore(CoilyScore);
	Spawner->Deactivate();
}

void AQBertGameMode::HandleCoilyFell(AQBertEnemy* Coily)
{
	// Luring Coily off clears the board; enemies return after Q*bert's next hop.
	for (TActorIterator<AQBertEnemy> It(GetWorld()); It; ++It)
	{
		if (*It != Coily)
		{
			It->Destroy();
		}
	}
	Spawner->ResetSpawner();
	bPlayerHasMoved = false;
}

void AQBertGameMode::ClearEnemies()
{
	Spawner->ResetSpawner();
	for (TActorIterator<AQBertEnemy> It(GetWorld()); It; ++It)
	{
		It->Destroy();
	}
}

void AQBertGameMode::LoseLife()
{
	Lives = FMath::Max(Lives - 1, 0);
	OnLivesChanged.Broadcast(Lives);
}

void AQBertGameMode::EndGame(bool bWon)
{
	bGameOver = true;
	bHopInputAllowed = false;
	GetWorldTimerManager().ClearAllTimersForObject(this);

	ClearEnemies();
	GetPyramid()->ClearDiscs();
	PlaySound(GameOverSound);

	if (AQBertPlayerController* Controller = GetQBertController())
	{
		Controller->ShowEndScreen(bWon, Score);
	}
}

void AQBertGameMode::PlaySound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void AQBertGameMode::RunAfter(float Delay, TFunction<void()>&& Callback)
{
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, MoveTemp(Callback)), Delay, false);
}
