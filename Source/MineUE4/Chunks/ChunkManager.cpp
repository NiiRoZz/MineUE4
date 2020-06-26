// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkManager.h"
#include "../Block.h"

// Sets default values
AChunkManager::AChunkManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

  if (GetLocalRole() == ENetRole::ROLE_Authority)
  {
    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::BeginPlay"));

    FIntVector pos = FIntVector(0);
    AChunk* chunk = GetWorld()->SpawnActor<AChunk>(FVector(0.0), FRotator(0.0));

    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::BeginPlay %p"), chunk);

    FBlock block;
    block.RelativeLocation[0] = 0;
    block.RelativeLocation[1] = 0;
    block.RelativeLocation[2] = 0;
    chunk->GetAllBlocks().Add(block);
    chunk->FlushNetDormancy();

    m_Chunks.Add( pos, chunk );
  }
	
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

