// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkManager.h"
#include "../Block.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

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

    /*const int nmbChunk = 10;
    for (int x = 0; x < (nmbChunk * AChunk::CHUNKSIZEX); ++x)
    {
      for (int y = 0; y < (nmbChunk * AChunk::CHUNKSIZEY); ++y)
      {
        const float height = FMath::PerlinNoise2D(FVector2D(x, y) / 64) * 8 + 8;

        for (int z = 0; z < height; ++z)
        {
          FIntVector pos = FIntVector(x, y, z);
          AddBlock(pos, 1);
        }
      }
    }

    for (auto& currChunk : m_Chunks)
    {
      if (currChunk.Value)
      {
        currChunk.Value->UpdateVisibleBlocks();
        currChunk.Value->FlushNetDormancy();
      }
    }*/

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
    chunk = CreateChunk(chunkPos);

    if (chunk)
      m_Chunks.Add(chunkPos, chunk);
  }
  check(chunk);

  chunk->SetBlock(relativePos, block);
}

AChunk* AChunkManager::CreateChunk(FIntVector chunkPos)
{
  if (m_Chunks.Contains(chunkPos))
  {
    return nullptr;
  }

  FVector realPosChunk = FVector(
    chunkPos[0] * AChunk::CHUNKSIZEX * AChunk::CubeSize,
    chunkPos[1] * AChunk::CHUNKSIZEY * AChunk::CubeSize,
    chunkPos[2] * AChunk::CHUNKSIZEZ * AChunk::CubeSize
  );

  AChunk* chunk = GetWorld()->SpawnActor<AChunk>(realPosChunk, FRotator(0.0));
  if (chunk)
  {
    chunk->Generate();
    m_Chunks.Add(chunkPos, chunk);
  }

  return chunk;
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

UMaterial* AChunkManager::GetDefaultOpaqueMaterialChunk()
{
  return m_DefaultOpaqueMaterialChunk;
}

UMaterial* AChunkManager::GetDefaultTransluscentMaterialChunk()
{
  return m_DefaulTransluscentMaterialChunk;
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

AChunk** AChunkManager::GetChunk(FIntVector chunkPos)
{
  return m_Chunks.Find(chunkPos);
}

// Called every frame
void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  //Only at server side
  if (GetLocalRole() == ENetRole::ROLE_Authority)
  {
    TArray<AActor*> Playersarray;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), Playersarray);

    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 1 %d"), Playersarray.Num());

    for (auto player : Playersarray)
    {
      APlayerController * currController = Cast<APlayerController>(player);
      UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 2 %p %p"), currController, currController->GetCharacter());
      if (currController)
      {
        FVector playerLocation;

        if (currController->GetCharacter())
        {
          playerLocation  = currController->GetCharacter()->GetActorLocation();
        }
        else
        {
          FRotator rotation;
          currController->GetActorEyesViewPoint(playerLocation, rotation);
        }

        UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 3 %f %f %f"), playerLocation[0], playerLocation[1], playerLocation[2]);

        FIntVector playerChunkCoords = FIntVector(
          playerLocation[0] / AChunk::CHUNKSIZEX / AChunk::CubeSize,
          playerLocation[1] / AChunk::CHUNKSIZEY / AChunk::CubeSize,
          playerLocation[2] / AChunk::CHUNKSIZEZ / AChunk::CubeSize
        );

        for (int x = -CHUNKRENDERDISTANCE; x < CHUNKRENDERDISTANCE; ++x)
        {
          for (int y = -CHUNKRENDERDISTANCE; y < CHUNKRENDERDISTANCE; ++y)
          {
            AChunk* chunk = CreateChunk(FIntVector(playerChunkCoords[0] + x, playerChunkCoords[1] + y, 0));

            //We created the chunk
            if (chunk)
            {
              chunk->UpdateVisibleBlocks();
              chunk->FlushNetDormancy();
            }
          }
        }
      }
    }
  }
}

