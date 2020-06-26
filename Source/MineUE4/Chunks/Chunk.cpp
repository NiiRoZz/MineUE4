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

  m_AllBlocks.Reserve(CHUNKSIZEX * CHUNKSIZEY * CHUNKSIZEZ);
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
  m_VisibleBlocks.Empty();

  //TODO: Make a algortihm to find only visible blocks
  for (auto& currBlock : m_AllBlocks)
  {
    m_VisibleBlocks.Add(currBlock);
  }
}

void AChunk::OnRep_VisibleBlocks()
{
  UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 1 %d"), m_VisibleBlocks.Num());
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
  for (auto &visibleBlock : m_VisibleBlocks)
  {
      UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 2 %d %f"), visibleBlock.RelativeLocation[0], GetActorLocation().X);
      UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 3 %d %f"), visibleBlock.RelativeLocation[1], GetActorLocation().Y);
      UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 4 %d %f"), visibleBlock.RelativeLocation[2], GetActorLocation().Z);
      FVector pos = FVector(
          visibleBlock.RelativeLocation[0] + GetActorLocation().X,
          visibleBlock.RelativeLocation[1] + GetActorLocation().Y,
          visibleBlock.RelativeLocation[2] + GetActorLocation().Z
      );

      transform.SetLocation(pos);

      int32 idx = cubeInst->GetMeshInst()->AddInstanceWorldSpace(transform);
      UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 5 %d"), idx);
      cubeInst->GetMeshInst()->SetCustomDataValue(idx, 0, 255.f, true);

      m_CubeInstancies.Add(idx);
  }
}

TArray<FBlock>& AChunk::GetAllBlocks()
{
  return m_AllBlocks;
}

