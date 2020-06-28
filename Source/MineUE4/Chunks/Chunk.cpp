// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "ChunkManager.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChunk::AChunk()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  m_ChunkManager = nullptr;

  m_ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
  RootComponent = m_ProceduralMesh;
}

AChunk::~AChunk()
{
  if (m_ChunkManager)
  {
    m_ChunkManager->RemoveChunk(this);
  }
}

// Called when the game starts or when spawned
void AChunk::BeginPlay()
{
  Super::BeginPlay();

  SetReplicates(true);
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

  if (GetLocalRole() == ENetRole::ROLE_Authority)
  {
    OnRep_VisibleBlocks();
  }

  UE_LOG(LogTemp, Warning, TEXT("AChunkManager::UpdateVisibleBlocks 2 %d"), m_VisibleBlocks.VisibleBlocks.Num());
}

void AChunk::SetBlock(FIntVector& relativePos, FBlock& block)
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

FIntVector AChunk::GetChunkPos()
{
  return FIntVector(
    GetActorLocation()[0] / CHUNKSIZEX / CubeSize,
    GetActorLocation()[1] / CHUNKSIZEY / CubeSize,
    GetActorLocation()[2] / CHUNKSIZEZ / CubeSize
  );
}

FBlock* AChunk::GetBlock(FIntVector& relativePos)
{
  return m_AllBlocks.Find(relativePos);
}

void AChunk::OnRep_VisibleBlocks()
{
  UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 1 %d"), m_VisibleBlocks.VisibleBlocks.Num());

  if (!m_ChunkManager)
  {
    TArray<AActor*> arrayChunkManager;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChunkManager::StaticClass(), arrayChunkManager);
    check(arrayChunkManager.Num() > 0);

    m_ChunkManager = Cast<AChunkManager>(arrayChunkManager[0]);
    check(m_ChunkManager);

    if (GetLocalRole() != ENetRole::ROLE_Authority)
    {
      m_ChunkManager->AddChunk(this);
    }
  }

  if (GetLocalRole() != ENetRole::ROLE_Authority)
  {
    m_AllBlocks.Empty();

    for (auto& visibleBlock : m_VisibleBlocks.VisibleBlocks)
    {
      m_AllBlocks.Add( visibleBlock.RelativeLocation, visibleBlock);
    }

    m_VisibleBlocks.VisibleBlocks.Empty();
  }

  m_ProceduralMesh->ClearAllMeshSections();

  TArray<FVector> vertices;
  TArray<int32> triangles;
  TArray<FVector2D> uvs;
  TArray<FLinearColor> colors;

  bool created = false;

  if (GetLocalRole() == ENetRole::ROLE_Authority)
  {
    for (auto& visibleBlock : m_VisibleBlocks.VisibleBlocks)
    {
      created |= GenerateCube(vertices, triangles, uvs, colors, visibleBlock.RelativeLocation, GetCubeFlags(visibleBlock.RelativeLocation));
    }
  }
  else
  {
    for (auto& visibleBlock : m_AllBlocks)
    {
      created |= GenerateCube(vertices, triangles, uvs, colors, visibleBlock.Value.RelativeLocation, GetCubeFlags(visibleBlock.Value.RelativeLocation));
    }
  }

  if (created)
  {
    m_ProceduralMesh->CreateMeshSection_LinearColor(0, vertices, triangles, TArray<FVector>(), uvs, colors, TArray<FProcMeshTangent>(), false);

    if (m_ChunkManager && m_ChunkManager->GetDefaultMaterialChunk())
    {
      m_ProceduralMesh->SetMaterial(0, m_ChunkManager->GetDefaultMaterialChunk());
    }
  }
}

uint8_t AChunk::GetCubeFlags(FIntVector& relativePos)
{
  uint8_t flags = 0;

  if (!m_ChunkManager)
  {
    return flags;
  }

  FBlock* currBlock = m_AllBlocks.Find(relativePos);
  if (!currBlock)
  {
    return flags;
  }

  FIntVector chunkPos = GetChunkPos();

  //X Side
  if (relativePos[0] - 1 < 0)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos - FIntVector(1, 0, 0), FIntVector(CHUNKSIZEX - 1, relativePos[1], relativePos[2]));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BACK;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos - FIntVector(1, 0, 0));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BACK;
    }
  }

  if (relativePos[0] + 1 >= CHUNKSIZEX)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos + FIntVector(1, 0, 0), FIntVector(0, relativePos[1], relativePos[2]));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::FRONT;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos + FIntVector(1, 0, 0));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::FRONT;
    }
  }

  //Y Side
  if (relativePos[1] - 1 < 0)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos - FIntVector(0, 1, 0), FIntVector(relativePos[0], CHUNKSIZEY - 1, relativePos[2]));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::RIGHT;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos - FIntVector(0, 1, 0));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::RIGHT;
    }
  }

  if (relativePos[1] + 1 >= CHUNKSIZEY)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos + FIntVector(0, 1, 0), FIntVector(relativePos[0], 0, relativePos[2]));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::LEFT;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos + FIntVector(0, 1, 0));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::LEFT;
    }
  }

  //Z Side
  if (relativePos[2] - 1 < 0)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos - FIntVector(0, 0, 1), FIntVector(relativePos[0], relativePos[1], CHUNKSIZEZ - 1));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BOTTOM;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos - FIntVector(0, 0, 1));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BOTTOM;
    }
  }

  if (relativePos[2] + 1 >= CHUNKSIZEZ)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos + FIntVector(0, 0, 1), FIntVector(relativePos[0], relativePos[1], 0));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::TOP;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos + FIntVector(0, 0, 1));

    if ((block && block->IsTransparent() && !currBlock->IsTransparent()) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::TOP;
    }
  }

  return flags;
}

void AChunk::GenerateQuad(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, FVector pos0, FVector pos1, FVector pos2, FVector pos3, FLinearColor color)
{
  const int32 startIdx = vertices.Num();
  vertices.Append({ pos0, pos1, pos2, pos3 });
  uvs.Append({ FVector2D(0.f, 0.f), FVector2D(0.f, 1.f), FVector2D(1.f, 1.f), FVector2D(1.f, 0.f) });
  triangles.Append({ startIdx + 0, startIdx + 1, startIdx + 3, startIdx + 1, startIdx + 2, startIdx + 3 });
  colors.Append({ color, color, color, color });
}

bool AChunk::GenerateCube(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, FIntVector pos, uint8_t flags)
{
  const FVector offset = FVector( pos * CubeSize );

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

  bool created = false;

  if (flags & (uint8_t)EChunkCubeFace::BOTTOM)
  {
    GenerateQuad( vertices, triangles, uvs, colors, CubeVertices[0], CubeVertices[1], CubeVertices[3], CubeVertices[2], FLinearColor(1.f / 255.f, 0.f, 0.f));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::TOP)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[5], CubeVertices[4], CubeVertices[6], CubeVertices[7], FLinearColor(1.f / 255.f, 0.f, 0.f));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::BACK)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[4], CubeVertices[0], CubeVertices[2], CubeVertices[6], FLinearColor(1.f / 255.f, 0.f, 0.f));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::FRONT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[7], CubeVertices[3], CubeVertices[1], CubeVertices[5], FLinearColor(1.f / 255.f, 0.f, 0.f));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::RIGHT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[5], CubeVertices[1], CubeVertices[0], CubeVertices[4], FLinearColor(1.f / 255.f, 0.f, 0.f));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::LEFT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, CubeVertices[6], CubeVertices[2], CubeVertices[3], CubeVertices[7], FLinearColor(1.f / 255.f, 0.f, 0.f));
    created |= true;
  }

  return created;
}

