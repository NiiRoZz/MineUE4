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

    const int nmbChunk = 3;
    for (int x = 0; x < (AChunk::CHUNKSIZEX * nmbChunk); ++x)
    {
      for (int y = 0; y < (AChunk::CHUNKSIZEY * nmbChunk); ++y)
      {
        for (int z = 0; z < (AChunk::CHUNKSIZEZ * nmbChunk); ++z)
        {
          FIntVector pos = FIntVector(x, y, z);
          AddBlock(pos, 1);
        }
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

void AChunkManager::AddBlock(FIntVector& pos, uint32 BlockType)
{
  FIntVector chunkPos = FIntVector(
    pos[0] / AChunk::CHUNKSIZEX,
    pos[1] / AChunk::CHUNKSIZEY,
    pos[2] / AChunk::CHUNKSIZEZ
  );

  FIntVector relativePos = FIntVector(
    pos[0] % AChunk::CHUNKSIZEX,
    pos[1] % AChunk::CHUNKSIZEY,
    pos[2] % AChunk::CHUNKSIZEZ
  );

  FBlock block;
  block.BlockType = BlockType;
  block.RelativeLocation = relativePos;

  AChunk* chunk;
  if (m_Chunks.Contains(chunkPos))
  {
    chunk = m_Chunks[chunkPos];
  }
  else
  {
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

void AChunkManager::AddChunk(AChunk* chunk)
{
  if (!chunk)
    return;

  FIntVector chunkPos = chunk->GetChunkPos();

  if (!m_Chunks.Contains(chunkPos))
  {
    m_Chunks.Add(chunkPos, chunk);
  }
}

void AChunkManager::RemoveChunk(AChunk* chunk)
{
  if (!chunk)
    return;

  m_Chunks.Remove(chunk->GetChunkPos());
}

UMaterial* AChunkManager::GetDefaultMaterialChunk()
{
  return m_DefaultMaterialChunk;
}

FBlock* AChunkManager::GetBlock(FIntVector chunkPos, FIntVector relativePos)
{
  AChunk** chunkFind = m_Chunks.Find(chunkPos);
  if (!chunkFind)
    return nullptr;

  AChunk* chunk = *chunkFind;
  if (!chunk)
    return nullptr;

  return chunk->GetBlock(relativePos);
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

