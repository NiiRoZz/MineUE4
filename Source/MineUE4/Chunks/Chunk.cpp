// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "../Instances/CubeInst.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChunk::AChunk()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
  Super::BeginPlay();

  SetReplicates(true);
  bAlwaysRelevant = true;
  SetNetDormancy(ENetDormancy::DORM_DormantAll);
  NetCullDistanceSquared = 368640000.f;
}

// Called every frame
void AChunk::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  //Only at server side
  if (GetLocalRole() == ENetRole::ROLE_Authority)
  {

  }
}

void AChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AChunk, m_VisibleBlocks);
}

void AChunk::UpdateVisibleBlocks()
{
  UE_LOG(LogTemp, Warning, TEXT("AChunkManager::UpdateVisibleBlocks 1 %d"), m_AllBlocks.Num());

  //TODO: Make a algortihm to find only visible blocks
  for (auto& currBlock : m_AllBlocks)
  {
    if (m_VisibleBlocksPos.Contains(currBlock.Key))
    {
      for (int32 i = 0; i < m_VisibleBlocks.VisibleBlocks.Num(); ++i)
      {
        if (m_VisibleBlocks.VisibleBlocks[i].RelativeLocation == currBlock.Key)
        {
          // If it's a air block, we don't care
          if (currBlock.Value.BlockType == 0)
          {
            m_VisibleBlocksPos.Remove(currBlock.Key);
            m_VisibleBlocks.VisibleBlocks.RemoveAt(i, 1, false);
            m_VisibleBlocks.MarkArrayDirty();
          }
          else if (m_VisibleBlocks.VisibleBlocks[i].RelativeLocation != currBlock.Value.RelativeLocation
            && m_VisibleBlocks.VisibleBlocks[i].BlockType != currBlock.Value.BlockType)
          {
            m_VisibleBlocks.VisibleBlocks[i].BlockType = currBlock.Value.BlockType;
            m_VisibleBlocks.VisibleBlocks[i].RelativeLocation = currBlock.Value.RelativeLocation;
            m_VisibleBlocks.MarkItemDirty(m_VisibleBlocks.VisibleBlocks[i]);
          }
          break;
        }
      }
    }
    else
    {
      // If it's a air block, we don't care
      if (currBlock.Value.BlockType == 0)
      {
        continue;
      }

      m_VisibleBlocks.MarkItemDirty(m_VisibleBlocks.VisibleBlocks.Add_GetRef(currBlock.Value));
      m_VisibleBlocksPos.Add(currBlock.Key);
    }
  }

  UE_LOG(LogTemp, Warning, TEXT("AChunkManager::UpdateVisibleBlocks 2 %d"), m_VisibleBlocks.VisibleBlocks.Num());
}

void AChunk::SetBlock(FIntVector relativePos, FBlock& block)
{
  if (m_AllBlocks.Contains(relativePos))
  {
    m_AllBlocks[relativePos] = block;
  }
  else
  {
    m_AllBlocks.Add(relativePos, block);
  }
}

void AChunk::OnRep_VisibleBlocks()
{
  UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 1 %d"), m_VisibleBlocks.VisibleBlocks.Num());
  TArray<AActor*> arrayCubeInst;

  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACubeInst::StaticClass(), arrayCubeInst);
  check(arrayCubeInst.Num() > 0);
  ACubeInst* cubeInst = Cast<ACubeInst>(arrayCubeInst[0]);

  for (auto cubeInstance : m_CubeInstancies)
  {
      cubeInst->GetMeshInst()->RemoveInstance(cubeInstance);
  }

  m_CubeInstancies.Empty();

  FTransform transform;
  for (auto &visibleBlock : m_VisibleBlocks.VisibleBlocks)
  {
      FVector pos = FVector(
          visibleBlock.RelativeLocation[0] + GetActorLocation().X,
          visibleBlock.RelativeLocation[1] + GetActorLocation().Y,
          visibleBlock.RelativeLocation[2] + GetActorLocation().Z
      );

      transform.SetLocation(pos);

      int32 idx = cubeInst->GetMeshInst()->AddInstanceWorldSpace(transform);
      cubeInst->GetMeshInst()->SetCustomDataValue(idx, 0, 255.f);

      m_CubeInstancies.Add(idx);
  }
}

