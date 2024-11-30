// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"

static FAutoConsoleCommand TestCommand2 = FAutoConsoleCommand(
	TEXT("Sample.Command"),
	TEXT(""),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			UE_LOG(LogTemp, Log, TEXT("call Sample.Command"));

			for (const FString& Arg : Args)
			{
				UE_LOG(LogTemp, Log, TEXT("arg : [%s]"), *Arg);
			}
		})
);


// Sets default values
AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

