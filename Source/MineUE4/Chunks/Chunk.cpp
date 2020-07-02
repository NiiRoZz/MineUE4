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

  if (GetLocalRole() == ROLE_Authority)
  {
    SetReplicates(true);
  }

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

  FindChunkManager();

  for (auto& currBlock : m_AllBlocks)
  {
    bool remove = (currBlock.Value.BlockType == 0 || GetCubeFlags(currBlock.Key) == 0);

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

void AChunk::Generate()
{
  FIntVector chunkPos = GetChunkPos();

  for (int x = 0; x < CHUNKSIZEX; ++x)
  {
    for (int y = 0; y < CHUNKSIZEY; ++y)
    {
      FVector2D WorldBlockCoord = FVector2D(x + chunkPos[0] * CHUNKSIZEX, y + chunkPos[1] * CHUNKSIZEY);
      const float height = FMath::PerlinNoise2D(WorldBlockCoord / 64) * 8 + 8;

      for (int z = 0; z < height; ++z)
      {
        FBlock newBlock;
        newBlock.RelativeLocation = FIntVector(x, y, z);
        newBlock.BlockType = 1;

        SetBlock(newBlock.RelativeLocation, newBlock);
      }
    }
  }
}

void AChunk::OnRep_VisibleBlocks()
{
  UE_LOG(LogTemp, Warning, TEXT("OnRep_VisibleBlocks 1 %d"), m_VisibleBlocks.VisibleBlocks.Num());

  FindChunkManager();

  if (GetLocalRole() != ENetRole::ROLE_Authority)
  {
    m_AllBlocks.Empty();

    for (auto& visibleBlock : m_VisibleBlocks.VisibleBlocks)
    {
      m_AllBlocks.Add( visibleBlock.RelativeLocation, visibleBlock);
    }

    m_VisibleBlocks.VisibleBlocks.Empty();
  }

  BuildMeshes();

  //Update adjacent chunks
  FIntVector myChunkPos = GetChunkPos();

  TArray<FIntVector> adjPosses = { FIntVector(1, 0, 0), FIntVector(0, 1, 0), FIntVector(0, 0, 1) };

  for (auto& adjPos : adjPosses)
  {
    AChunk** adjacentChunk = m_ChunkManager->GetChunk(myChunkPos - adjPos);
    if (adjacentChunk)
    {
      (*adjacentChunk)->BuildMeshes();
    }

    adjacentChunk = m_ChunkManager->GetChunk(myChunkPos + adjPos);
    if (adjacentChunk)
    {
      (*adjacentChunk)->BuildMeshes();
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
  bool imTransluscent = currBlock->IsTransluscent();

  //X Side
  if (relativePos[0] - 1 < 0)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos - FIntVector(1, 0, 0), FIntVector(CHUNKSIZEX - 1, relativePos[1], relativePos[2]));

    if ( ( block && ( ( block->IsTransluscent() && !imTransluscent) || block->IsInvisible() ) ) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BACK;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos - FIntVector(1, 0, 0));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BACK;
    }
  }

  if (relativePos[0] + 1 >= CHUNKSIZEX)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos + FIntVector(1, 0, 0), FIntVector(0, relativePos[1], relativePos[2]));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::FRONT;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos + FIntVector(1, 0, 0));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::FRONT;
    }
  }

  //Y Side
  if (relativePos[1] - 1 < 0)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos - FIntVector(0, 1, 0), FIntVector(relativePos[0], CHUNKSIZEY - 1, relativePos[2]));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::RIGHT;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos - FIntVector(0, 1, 0));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::RIGHT;
    }
  }

  if (relativePos[1] + 1 >= CHUNKSIZEY)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos + FIntVector(0, 1, 0), FIntVector(relativePos[0], 0, relativePos[2]));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::LEFT;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos + FIntVector(0, 1, 0));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::LEFT;
    }
  }

  //Z Side
  if (relativePos[2] - 1 < 0)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos - FIntVector(0, 0, 1), FIntVector(relativePos[0], relativePos[1], CHUNKSIZEZ - 1));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BOTTOM;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos - FIntVector(0, 0, 1));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::BOTTOM;
    }
  }

  if (relativePos[2] + 1 >= CHUNKSIZEZ)
  {
    FBlock* block = m_ChunkManager->GetBlock(chunkPos + FIntVector(0, 0, 1), FIntVector(relativePos[0], relativePos[1], 0));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::TOP;
    }
  }
  else
  {
    FBlock* block = m_AllBlocks.Find(relativePos + FIntVector(0, 0, 1));

    if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
    {
      flags |= (uint8_t)EChunkCubeFace::TOP;
    }
  }

  return flags;
}

void AChunk::BuildMeshes()
{
  ensure(m_ProceduralMesh);

  m_ProceduralMesh->ClearAllMeshSections();

  //Opaque blocks
  {
    TArray<FVector> vertices;
    TArray<int32> triangles;
    TArray<FVector2D> uvs;
    TArray<FLinearColor> colors;
    TArray<FVector> normals;

    bool created = false;

    for (auto& visibleBlock : m_AllBlocks)
    {
      if (!visibleBlock.Value.IsTransluscent() && !visibleBlock.Value.IsInvisible())
      {
        created |= GenerateCube(vertices, triangles, uvs, colors, normals, visibleBlock.Value.BlockType, visibleBlock.Value.RelativeLocation, GetCubeFlags(visibleBlock.Value.RelativeLocation));
      }
    }

    if (created && m_ChunkManager && m_ChunkManager->GetDefaultOpaqueMaterialChunk())
    {
      m_ProceduralMesh->CreateMeshSection_LinearColor(0, vertices, triangles, normals, uvs, colors, TArray<FProcMeshTangent>(), true);
      m_ProceduralMesh->SetMaterial(0, m_ChunkManager->GetDefaultOpaqueMaterialChunk());
    }
  }

  //Transluscent blocks
  {
    TArray<FVector> vertices;
    TArray<int32> triangles;
    TArray<FVector2D> uvs;
    TArray<FLinearColor> colors;
    TArray<FVector> normals;

    bool created = false;

    for (auto& visibleBlock : m_AllBlocks)
    {
      if (visibleBlock.Value.IsTransluscent() && !visibleBlock.Value.IsInvisible())
      {
        created |= GenerateCube(vertices, triangles, uvs, colors, normals, visibleBlock.Value.BlockType, visibleBlock.Value.RelativeLocation, GetCubeFlags(visibleBlock.Value.RelativeLocation));
      }
    }

    if (created && m_ChunkManager && m_ChunkManager->GetDefaultTransluscentMaterialChunk())
    {
      m_ProceduralMesh->CreateMeshSection_LinearColor(1, vertices, triangles, normals, uvs, colors, TArray<FProcMeshTangent>(), false);
      m_ProceduralMesh->SetMaterial(1, m_ChunkManager->GetDefaultTransluscentMaterialChunk());
    }
  }
}

void AChunk::GenerateQuad(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, TArray<FVector>& normals, FVector pos0, FVector pos1, FVector pos2, FVector pos3, FLinearColor color, FVector normal)
{
  const int32 startIdx = vertices.Num();
  vertices.Append({ pos0, pos1, pos2, pos3 });
  uvs.Append({ FVector2D(0.f, 0.f), FVector2D(0.f, 1.f), FVector2D(1.f, 1.f), FVector2D(1.f, 0.f) });
  triangles.Append({ startIdx + 0, startIdx + 1, startIdx + 3, startIdx + 1, startIdx + 2, startIdx + 3 });
  colors.Append({ color, color, color, color });
  normals.Append({ normal, normal, normal, normal });
}

bool AChunk::GenerateCube(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, TArray<FVector>& normals, uint32 BlockType, FIntVector pos, uint8_t flags)
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
    GenerateQuad( vertices, triangles, uvs, colors, normals, CubeVertices[0], CubeVertices[1], CubeVertices[3], CubeVertices[2], FLinearColor((float)BlockType / 255.f, 0.f, 0.f), FVector(0, 0, -1));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::TOP)
  {
    GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[5], CubeVertices[4], CubeVertices[6], CubeVertices[7], FLinearColor((float)BlockType / 255.f, 0.f, 0.f), FVector(0, 0, 1));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::BACK)
  {
    GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[4], CubeVertices[0], CubeVertices[2], CubeVertices[6], FLinearColor((float)BlockType / 255.f, 0.f, 0.f), FVector(-1, 0, 0));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::FRONT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[7], CubeVertices[3], CubeVertices[1], CubeVertices[5], FLinearColor((float)BlockType / 255.f, 0.f, 0.f), FVector(1, 0, 0));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::RIGHT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[5], CubeVertices[1], CubeVertices[0], CubeVertices[4], FLinearColor((float)BlockType / 255.f, 0.f, 0.f), FVector(0, -1, 0));
    created |= true;
  }
  if (flags & (uint8_t)EChunkCubeFace::LEFT)
  {
    GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[6], CubeVertices[2], CubeVertices[3], CubeVertices[7], FLinearColor((float)BlockType / 255.f, 0.f, 0.f), FVector(0, 1, 0));
    created |= true;
  }

  return created;
}

void AChunk::FindChunkManager()
{
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
}

