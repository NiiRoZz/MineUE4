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

  m_ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
  RootComponent = m_ProceduralMesh;
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
  Super::BeginPlay();

  SetReplicates(true);
  SetNetDormancy(ENetDormancy::DORM_DormantAll);
  NetCullDistanceSquared = 368640000.f;

  TArray<FVector> vertices;
  TArray<int32> triangles;
  TArray<FVector2D> uvs;
  TArray<FLinearColor> colors;

  GenerateCube(vertices, triangles, uvs, colors, FVector(0.0), (uint8_t)EChunkCubeFace::ALL);

  m_ProceduralMesh->CreateMeshSection_LinearColor(0, vertices, triangles, TArray<FVector>(), uvs, colors, TArray<FProcMeshTangent>(), false);

  if (m_Material)
  {
    m_ProceduralMesh->SetMaterial(0, m_Material);
  }
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
    bool remove = (currBlock.Value.BlockType == 0);

    if (m_VisibleBlocksPos.Contains(currBlock.Key))
    {
      for (int32 i = 0; i < m_VisibleBlocks.VisibleBlocks.Num(); ++i)
      {
        if (m_VisibleBlocks.VisibleBlocks[i].RelativeLocation == currBlock.Key)
        {
          // If it's undesired block, we remove it
          if (remove)
          {
            m_VisibleBlocksPos.Remove(currBlock.Key);
            m_VisibleBlocks.VisibleBlocks.RemoveAt(i, 1, false);
            m_VisibleBlocks.MarkArrayDirty();
          }
          else if (m_VisibleBlocks.VisibleBlocks[i].RelativeLocation != currBlock.Value.RelativeLocation && m_VisibleBlocks.VisibleBlocks[i].BlockType != currBlock.Value.BlockType)
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
      // If it's undesired block, we don't care
      if (remove)
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
  /*TArray<AActor*> arrayCubeInst;

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
  }*/
}

void AChunk::GenerateQuad(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, FVector pos0, FVector pos1, FVector pos2, FVector pos3, FLinearColor color)
{
  const int32 startIdx = vertices.Num();
  vertices.Append({ pos0, pos1, pos2, pos3 });
  uvs.Append({ FVector2D(0.f, 0.f), FVector2D(0.f, 1.f), FVector2D(1.f, 1.f), FVector2D(1.f, 0.f) });
  triangles.Append({ startIdx + 0, startIdx + 1, startIdx + 3, startIdx + 1, startIdx + 2, startIdx + 3 });
  colors.Append({ color, color, color, color });
}

void AChunk::GenerateCube(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, FVector pos, uint8_t flags)
{
  static const uint32 CubeSize = 100;
  const FVector offset = pos * CubeSize;

  const TArray<FVector> CubeVertices( {
    offset + FVector(0, 0, 0),
    offset + FVector(CubeSize, 0, 0),
    offset + FVector(0, CubeSize, 0),
    offset + FVector(CubeSize, CubeSize, 0),
    offset + FVector(0, 0, CubeSize),
    offset + FVector(CubeSize, 0, CubeSize),
    offset + FVector(0, CubeSize, CubeSize),
    offset + FVector(CubeSize, CubeSize, CubeSize) }
  );

  if (flags & (uint8_t)EChunkCubeFace::BOTTOM)
  {
    GenerateQuad( vertices, triangles, uvs, colors, CubeVertices[0], CubeVertices[1], CubeVertices[3], CubeVertices[2], FLinearColor(1.f / 255.f, 0.f, 0.f));
  }
  if (flags & (uint8_t)EChunkCubeFace::TOP)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[5], CubeVertices[4], CubeVertices[6], CubeVertices[7], FLinearColor(1.f / 255.f, 0.f, 0.f));
  }
  if (flags & (uint8_t)EChunkCubeFace::BACK)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[4], CubeVertices[0], CubeVertices[2], CubeVertices[6], FLinearColor(1.f / 255.f, 0.f, 0.f));
  }
  if (flags & (uint8_t)EChunkCubeFace::FRONT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[7], CubeVertices[3], CubeVertices[1], CubeVertices[5], FLinearColor(1.f / 255.f, 0.f, 0.f));
  }
  if (flags & (uint8_t)EChunkCubeFace::RIGHT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[5], CubeVertices[1], CubeVertices[0], CubeVertices[4], FLinearColor(1.f / 255.f, 0.f, 0.f));
  }
  if (flags & (uint8_t)EChunkCubeFace::LEFT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[6], CubeVertices[2], CubeVertices[3], CubeVertices[7], FLinearColor(1.f / 255.f, 0.f, 0.f));
  }
}

