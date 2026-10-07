#include "Core/QBertCheatManager.h"
#include "Core/QBertGameMode.h"
#include "Characters/QBertEnemy.h"
#include "World/QBertEnemySpawner.h"
#include "QBert.h"

namespace
{
	AQBertGameMode* GetQBertGameMode(const UObject* Context)
	{
		const UWorld* World = Context ? Context->GetWorld() : nullptr;
		return World ? World->GetAuthGameMode<AQBertGameMode>() : nullptr;
	}
}

void UQBertCheatManager::QBertAddScore(int32 Points)
{
	if (AQBertGameMode* GameMode = GetQBertGameMode(this))
	{
		GameMode->AddScore(Points);
	}
}

void UQBertCheatManager::QBertSpawn(const FString& Type)
{
	AQBertGameMode* GameMode = GetQBertGameMode(this);
	AQBertEnemySpawner* Spawner = GameMode ? GameMode->GetSpawner() : nullptr;
	if (!Spawner)
	{
		return;
	}

	TSubclassOf<AQBertEnemy> EnemyClass;
	if (Type.Equals(TEXT("Red"), ESearchCase::IgnoreCase))
	{
		EnemyClass = Spawner->GetRedBallClass();
	}
	else if (Type.Equals(TEXT("Green"), ESearchCase::IgnoreCase))
	{
		EnemyClass = Spawner->GetGreenBallClass();
	}
	else if (Type.Equals(TEXT("Coily"), ESearchCase::IgnoreCase))
	{
		EnemyClass = Spawner->GetCoilyClass();
	}
	else
	{
		UE_LOG(LogQBert, Warning, TEXT("QBertSpawn: unknown type '%s' (use Red, Green or Coily)."), *Type);
		return;
	}

	Spawner->SpawnEnemyAtRandomCell(EnemyClass);
}

void UQBertCheatManager::QBertFinishRound()
{
	if (AQBertGameMode* GameMode = GetQBertGameMode(this))
	{
		GameMode->CompleteRound();
	}
}

void UQBertCheatManager::QBertToggleSpawner()
{
	AQBertGameMode* GameMode = GetQBertGameMode(this);
	if (AQBertEnemySpawner* Spawner = GameMode ? GameMode->GetSpawner() : nullptr)
	{
		Spawner->SetSpawningEnabled(!Spawner->IsSpawningEnabled());
		UE_LOG(LogQBert, Display, TEXT("Enemy spawning %s."), Spawner->IsSpawningEnabled() ? TEXT("enabled") : TEXT("disabled"));
	}
}
