#include "Core/QBertMenuGameMode.h"
#include "Core/QBertPlayerController.h"
#include "Kismet/GameplayStatics.h"

AQBertMenuGameMode::AQBertMenuGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AQBertPlayerController::StaticClass();
}

void AQBertMenuGameMode::StartPlay()
{
	Super::StartPlay();

	if (AQBertPlayerController* Controller = Cast<AQBertPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		Controller->ShowMainMenu();
	}

	if (MenuMusic)
	{
		UGameplayStatics::PlaySound2D(this, MenuMusic);
	}
}
