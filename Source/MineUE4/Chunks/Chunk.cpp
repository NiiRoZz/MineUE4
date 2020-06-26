// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AChunk::AChunk()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  m_AllBlocks.Reserve(CHUNKSIZEX * CHUNKSIZEY * CHUNKSIZEZ);
  m_VisibleBlocks.Reserve(CHUNKSIZEX * CHUNKSIZEY * CHUNKSIZEZ);
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

void AChunk::OnRep_VisibleBlocks()
{
  UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks %d"), m_VisibleBlocks.Num());

  for (int x = 0; x < CHUNKSIZEX; ++x)
  {
    for (int y = 0; y < CHUNKSIZEY; ++y)
    {
      for (int z = 0; z < CHUNKSIZEZ; ++z)
      {
        //m_AllBlocks[(x * CHUNKSIZEY + y) * CHUNKSIZEZ + z] = 1;
        
      }
    }
  }
}

void AChunk::UpdateVisibleBlocks()
{
  m_VisibleBlocks.Empty();

  for (uint16 i = 0; i < m_AllBlocks.Num(); ++i)
  {
    if (m_AllBlocks[i])
      m_VisibleBlocks.Insert(m_AllBlocks[i], i);
  }

  UE_LOG(LogTemp, Warning, TEXT("UpdateVisibleBlocks %d"), m_VisibleBlocks.Num());
}

TArray<UBlock*>& AChunk::GetAllBlocks()
{
  return m_AllBlocks;
}

bool AChunk::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
  bool wroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

  for (UBlock* block : m_VisibleBlocks)
  {
    if (block)
    {
      wroteSomething |= Channel->ReplicateSubobject(block, *Bunch, *RepFlags);
    }
  }

  return wroteSomething;
}

