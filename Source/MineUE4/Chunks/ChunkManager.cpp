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

  /*if (GetLocalRole() == ENetRole::ROLE_Authority)
  {
    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::BeginPlay"));

    UE_LOG(LogTemp, Warning, TEXT("AChunkManager::BeginPlay End"));
  }*/
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
    TArray<AActor*> allPlayers;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), allPlayers);

    //UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 1 %d"), allPlayers.Num());

    //Find the deleted chunks
    TArray<FIntVector> allDeletedChunks;
    for (auto& currChunk : m_Chunks)
    {
      if (!IsPlayerNearby(currChunk.Value, allPlayers))
      {
        allDeletedChunks.Add(currChunk.Key);
      }
    }

    if (allDeletedChunks.Num() > 0)
    {
      UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 2 %d"), allDeletedChunks.Num());
    }

    //Delete chunks
    for (auto& currChunk : allDeletedChunks)
    {
      AChunk* chunk = m_Chunks.FindAndRemoveChecked(currChunk);
      if (!chunk)
      {
        continue;
      }
      chunk->Destroy();
    }

    //Spawn new chunks
    for (auto player : allPlayers)
    {
      APlayerController* currController = Cast<APlayerController>(player);
      //UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 3 %p %p"), currController, currController->GetCharacter());
      if (currController)
      {
        FVector playerLocation = GetPlayerLocation(currController);

        //UE_LOG(LogTemp, Warning, TEXT("AChunkManager::Tick 4 %f %f %f"), playerLocation[0], playerLocation[1], playerLocation[2]);

        FIntVector playerChunkCoords = FIntVector(
          playerLocation[0] / AChunk::CHUNKSIZEX / AChunk::CubeSize,
          playerLocation[1] / AChunk::CHUNKSIZEY / AChunk::CubeSize,
          playerLocation[2] / AChunk::CHUNKSIZEZ / AChunk::CubeSize
        );

        uint8 nmbChunkGenerated = 0;

        for (int x = 0; x <= CHUNKRENDERDISTANCEX; ++x)
        {
          for (int y = 0; y <= CHUNKRENDERDISTANCEY; ++y)
          {
            for (int z = 0; z <= CHUNKRENDERDISTANCEZ; ++z)
            {
              if (nmbChunkGenerated > 5)
              {
                return;
              }

              AChunk* chunk = CreateChunk(FIntVector(playerChunkCoords.X + x, playerChunkCoords.Y + y, 0/*playerChunkCoords.Z + z*/));

              //We created the chunk
              if (chunk)
              {
                nmbChunkGenerated++;
                chunk->UpdateCompressedBlocks();
                chunk->FlushNetDormancy();
              }

              AChunk* chunk2 = CreateChunk(FIntVector(playerChunkCoords.X - x, playerChunkCoords.Y - y, 0/*playerChunkCoords.Z + z*/));

              //We created the chunk
              if (chunk2)
              {
                nmbChunkGenerated++;
                chunk2->UpdateCompressedBlocks();
                chunk2->FlushNetDormancy();
              }

              AChunk* chunk3 = CreateChunk(FIntVector(playerChunkCoords.X + x, playerChunkCoords.Y - y, 0/*playerChunkCoords.Z + z*/));

              //We created the chunk
              if (chunk3)
              {
                nmbChunkGenerated++;
                chunk3->UpdateCompressedBlocks();
                chunk3->FlushNetDormancy();
              }

              AChunk* chunk4 = CreateChunk(FIntVector(playerChunkCoords.X - x, playerChunkCoords.Y + y, 0/*playerChunkCoords.Z + z*/));

              //We created the chunk
              if (chunk4)
              {
                nmbChunkGenerated++;
                chunk4->UpdateCompressedBlocks();
                chunk4->FlushNetDormancy();
              }
            }
          }
        }
      }
    }
  }
}

FVector AChunkManager::GetPlayerLocation(APlayerController* playerController) const
{
  FVector playerLocation(0.f);

  if (!ensure(playerController))
  {
    return playerLocation;
  }

  if (playerController->GetCharacter())
  {
    playerLocation = playerController->GetCharacter()->GetActorLocation();
  }
  else
  {
    FRotator rotation;
    playerController->GetActorEyesViewPoint(playerLocation, rotation);
  }

  return playerLocation;
}

bool AChunkManager::IsPlayerNearby(AChunk* chunk, TArray<AActor*>& allPlayers) const
{
  if (!ensure(chunk))
  {
    return false;
  }

  FIntVector chunkLocation = chunk->GetChunkPos();

  for (auto player : allPlayers)
  {
    APlayerController* currController = Cast<APlayerController>(player);

    if (currController)
    {
      FVector playerLocation = GetPlayerLocation(currController);
      FIntVector playerChunkCoords = FIntVector(
        playerLocation[0] / AChunk::CHUNKSIZEX / AChunk::CubeSize,
        playerLocation[1] / AChunk::CHUNKSIZEY / AChunk::CubeSize,
        playerLocation[2] / AChunk::CHUNKSIZEZ / AChunk::CubeSize
      );

      //UE_LOG(LogTemp, Warning, TEXT("AChunkManager::IsPlayerNearby 1 %d %d %d"), chunkLocation.X, chunkLocation.Y, chunkLocation.Z);
      //UE_LOG(LogTemp, Warning, TEXT("AChunkManager::IsPlayerNearby 2 %d %d %d"), playerChunkCoords.X, playerChunkCoords.Y, playerChunkCoords.Z);
      //UE_LOG(LogTemp, Warning, TEXT("AChunkManager::IsPlayerNearby 1 %s %d %d %d\n"), *(chunk->GetActorLabel()), FMath::Abs(chunkLocation.X - playerChunkCoords.X), FMath::Abs(chunkLocation.Y - playerChunkCoords.Y), FMath::Abs(chunkLocation.Z - playerChunkCoords.Z));

      if (FMath::Abs(chunkLocation.X - playerChunkCoords.X) <= CHUNKRENDERDISTANCEX && FMath::Abs(chunkLocation.Y - playerChunkCoords.Y) <= CHUNKRENDERDISTANCEY /*|| FMath::Abs(chunkLocation.Z - playerChunkCoords.Z) < CHUNKRENDERDISTANCE*/)
      {
        return true;
      }
    }
  }

  return false;
}

