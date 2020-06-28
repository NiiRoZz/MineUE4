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

    const int nmbChunk = 9;
    for (int x = 0; x < (16 * nmbChunk); ++x)
    {
      for (int y = 0; y < (16 * nmbChunk); ++y)
      {
        AddBlock(FIntVector(x, y, 0), 1);
      }
    }

    for (auto currChunk : m_Chunks)
    {
      if (currChunk.Value)
      {
        currChunk.Value->UpdateVisibleBlocks();
        currChunk.Value->FlushNetDormancy();
      }
    }

    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::BeginPlay End"));
  }
}

void AChunkManager::AddBlock(FIntVector pos, uint32 BlockType)
{
  FIntVector chunkPos = FIntVector(
    pos[0] / (AChunk::CHUNKSIZEX),
    pos[1] / (AChunk::CHUNKSIZEY),
    pos[2] / (AChunk::CHUNKSIZEZ)
  );

  FIntVector relativePos = FIntVector(
    pos[0] % (AChunk::CHUNKSIZEX),
    pos[1] % (AChunk::CHUNKSIZEY),
    pos[2] % (AChunk::CHUNKSIZEZ)
  );

  UE_LOG(LogTemp, Warning, TEXT("AChunkManager::AddBlock 1 %d %d %d"), chunkPos[0], chunkPos[1], chunkPos[2]);
  UE_LOG(LogTemp, Warning, TEXT("AChunkManager::AddBlock 2 %d %d %d"), relativePos[0], relativePos[1], relativePos[2]);

  FBlock block;
  block.BlockType = BlockType;
  block.RelativeLocation = relativePos;

  AChunk* chunk;
  if (m_Chunks.Contains(chunkPos))
  {
    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::AddBlock 3 contains"));
    chunk = m_Chunks[chunkPos];
  }
  else
  {
    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::AddBlock 3 not contains"));
    FVector realPosChunk = FVector(
      chunkPos[0] * AChunk::CHUNKSIZEX * AChunk::CubeSize,
      chunkPos[1] * AChunk::CHUNKSIZEY * AChunk::CubeSize,
      chunkPos[2] * AChunk::CHUNKSIZEZ * AChunk::CubeSize
    );

    chunk = GetWorld()->SpawnActor<AChunk>(realPosChunk, FRotator(0.0));
    m_Chunks.Add(chunkPos, chunk);
  }
  check(chunk);

  chunk->SetBlock(relativePos, block);
}

UMaterial* AChunkManager::GetDefaultMaterialChunk()
{
  return m_DefaultMaterialChunk;
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

