// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunk.h"
#include "ChunkManager.h"
#include "FastNoise.h"
#include "BlockTypeRegistery.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AChunk::AChunk()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	m_ChunkManager = nullptr;

	m_ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	RootComponent = m_ProceduralMesh;

	bReplicates = true;

	m_AllBlocks.SetNumZeroed(CHUNKSIZEX * CHUNKSIZEY * CHUNKSIZEZ);
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

	SetNetDormancy(ENetDormancy::DORM_DormantAll);
	NetCullDistanceSquared = 368640000.f;
}

// Called every frame
void AChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AChunk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChunk, m_CompressedBlocks);
}

void AChunk::UpdateCompressedBlocks()
{
	//UE_LOG(LogTemp, Warning, TEXT("AChunkManager::UpdateCompressedBlocks 1 %d"), m_AllBlocks.Num());

	FindChunkManager();

	m_CompressedBlocks.Empty();

	FCompressedBlock compressedBlock;
	compressedBlock.BlockType = m_AllBlocks[0].BlockType;
	compressedBlock.NmbBlocks = 0;

	for (auto& currBlock : m_AllBlocks)
	{
		if (currBlock.BlockType == compressedBlock.BlockType)
		{
			compressedBlock.NmbBlocks++;
		}
		else
		{
			m_CompressedBlocks.Add(compressedBlock);

			compressedBlock = FCompressedBlock();
			compressedBlock.BlockType = currBlock.BlockType;
			compressedBlock.NmbBlocks = 1;
		}
	}

	//m_CompressedBlocks.MarkArrayDirty();

	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		OnRep_CompressedBlocks();
	}

	//UE_LOG(LogTemp, Warning, TEXT("AChunkManager::UpdateCompressedBlocks 2 %d"), m_CompressedBlocks.Num());
}

void AChunk::SetBlock(FIntVector relativePos, FBlock& block)
{
	m_AllBlocks[Get1DIndex(relativePos)].CopyFrom(block);
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
	return &(m_AllBlocks[Get1DIndex(relativePos)]);
}

void AChunk::Generate()
{
	FIntVector chunkPos = GetChunkPos();

	static const int32 airID = ABlockTypeRegistery::BlockTypeRegistery->GetBlockIdFromName("Air");
	static const int32 stoneID = ABlockTypeRegistery::BlockTypeRegistery->GetBlockIdFromName("Stone");
	static const int32 dirtID = ABlockTypeRegistery::BlockTypeRegistery->GetBlockIdFromName("Dirt");
	static const int32 grassID = ABlockTypeRegistery::BlockTypeRegistery->GetBlockIdFromName("Grass");
	static const int32 waterID = ABlockTypeRegistery::BlockTypeRegistery->GetBlockIdFromName("Water");

	static const int WaterLevel = 64;

	FastNoise noise(5000);
	noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	noise.SetFractalType(FastNoise::FractalType::FBM);
	noise.SetFrequency(0.0039f);
	noise.SetFractalOctaves(6);
	//noise.SetFractalLacunarity(2.0f);
	//noise.SetFractalGain(0.6f);

	for (int x = 0; x < CHUNKSIZEX; ++x)
	{
		for (int y = 0; y < CHUNKSIZEY; ++y)
		{
			FVector2D WorldBlockCoord = FVector2D(x + chunkPos[0] * CHUNKSIZEX, y + chunkPos[1] * CHUNKSIZEY);

			// Set to 0..1 instead -1..1
			float heightFractal = noise.GetSimplexFractal(WorldBlockCoord.X, WorldBlockCoord.Y) * 0.5f + 0.5f;
			const int finalHeight = 32 + heightFractal * (128 - 32);

			for (int z = 0; z < CHUNKSIZEZ; ++z)
			{
				const int deap = finalHeight - z;

				FBlock newBlock;

				if (z <= finalHeight)
				{
					if (deap == 0)
					{
						newBlock.BlockType = grassID;
					}
					else if (deap <= 3)
					{
						newBlock.BlockType = dirtID;
					}
					else
					{
						newBlock.BlockType = stoneID;
					}
				}
				else if (z <= WaterLevel)
				{
					newBlock.BlockType = waterID;
				}
				else
				{
					newBlock.BlockType = airID;
				}

				SetBlock(FIntVector(x, y, z), newBlock);
			}
		}
	}
}

void AChunk::BuildMeshes()
{
	ensure(m_ProceduralMesh);

	m_ProceduralMesh->ClearAllMeshSections();

	if (!ABlockTypeRegistery::BlockTypeRegistery)
		return;

	//Opaque blocks
	{
		TArray<FVector> vertices;
		TArray<int32> triangles;
		TArray<FVector2D> uvs;
		TArray<FLinearColor> colors;
		TArray<FVector> normals;

		bool created = false;

		for (int i = 0; i < m_AllBlocks.Num(); ++i)
		{
			FBlock &currBlock = m_AllBlocks[i];

			if (!currBlock.IsTransluscent() && !currBlock.IsInvisible())
			{
				FIntVector pos = Get3DPosition(i);
				created |= GenerateCube(vertices, triangles, uvs, colors, normals, currBlock.BlockType, pos, GetCubeFlags(pos));
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

		for (int i = 0; i < m_AllBlocks.Num(); ++i)
		{
			FBlock& currBlock = m_AllBlocks[i];

			if (currBlock.IsTransluscent() && !currBlock.IsInvisible())
			{
				FIntVector pos = Get3DPosition(i);
				created |= GenerateCube(vertices, triangles, uvs, colors, normals, currBlock.BlockType, pos, GetCubeFlags(pos));
			}
		}

		if (created && m_ChunkManager && m_ChunkManager->GetDefaultTransluscentMaterialChunk())
		{
			m_ProceduralMesh->CreateMeshSection_LinearColor(1, vertices, triangles, normals, uvs, colors, TArray<FProcMeshTangent>(), false);
			m_ProceduralMesh->SetMaterial(1, m_ChunkManager->GetDefaultTransluscentMaterialChunk());
		}
	}
}

void AChunk::OnRep_CompressedBlocks()
{
	//UE_LOG(LogTemp, Warning, TEXT("OnRep_CompressedBlocks 1 %d"), m_CompressedBlocks.Num());

	FindChunkManager();

	if (GetLocalRole() != ENetRole::ROLE_Authority)
	{
		int idx = 0;
		for (auto& compressedBlock : m_CompressedBlocks)
		{
			FBlock block;
			block.CopyFrom(compressedBlock);

			for (uint32 i = 0; i < compressedBlock.NmbBlocks; ++i)
			{
				SetBlock(Get3DPosition(idx++), block);
			}
		}
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

	FBlock* currBlock = &(m_AllBlocks[Get1DIndex(relativePos)]);
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
		FBlock* block = &(m_AllBlocks[Get1DIndex(relativePos - FIntVector(1, 0, 0))]);

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
		FBlock* block = &(m_AllBlocks[Get1DIndex(relativePos + FIntVector(1, 0, 0))]);

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
		FBlock* block = &(m_AllBlocks[Get1DIndex(relativePos - FIntVector(0, 1, 0))]);

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
		FBlock* block = &(m_AllBlocks[Get1DIndex(relativePos + FIntVector(0, 1, 0))]);

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
		FBlock* block = &(m_AllBlocks[Get1DIndex(relativePos - FIntVector(0, 0, 1))]);

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
		FBlock* block = &(m_AllBlocks[Get1DIndex(relativePos + FIntVector(0, 0, 1))]);

		if ((block && ((block->IsTransluscent() && !imTransluscent) || block->IsInvisible())) || !block)
		{
			flags |= (uint8_t)EChunkCubeFace::TOP;
		}
	}

	return flags;
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

bool AChunk::GenerateCube(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, TArray<FVector>& normals, uint32 blockType, FIntVector pos, uint8_t flags)
{
	const FVector offset( pos * (float)CubeSize );

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
		GenerateQuad( vertices, triangles, uvs, colors, normals, CubeVertices[0], CubeVertices[1], CubeVertices[3], CubeVertices[2], ABlockTypeRegistery::BlockTypeRegistery->GetTextureForBlock(blockType, EChunkCubeFace::BOTTOM), FVector(0, 0, -1));
		created |= true;
	}
	if (flags & (uint8_t)EChunkCubeFace::TOP)
	{
		GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[5], CubeVertices[4], CubeVertices[6], CubeVertices[7], ABlockTypeRegistery::BlockTypeRegistery->GetTextureForBlock(blockType, EChunkCubeFace::TOP), FVector(0, 0, 1));
		created |= true;
	}
	if (flags & (uint8_t)EChunkCubeFace::BACK)
	{
		GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[4], CubeVertices[0], CubeVertices[2], CubeVertices[6], ABlockTypeRegistery::BlockTypeRegistery->GetTextureForBlock(blockType, EChunkCubeFace::BACK), FVector(-1, 0, 0));
		created |= true;
	}
	if (flags & (uint8_t)EChunkCubeFace::FRONT)
	{
		GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[7], CubeVertices[3], CubeVertices[1], CubeVertices[5], ABlockTypeRegistery::BlockTypeRegistery->GetTextureForBlock(blockType, EChunkCubeFace::FRONT), FVector(1, 0, 0));
		created |= true;
	}
	if (flags & (uint8_t)EChunkCubeFace::RIGHT)
	{
		GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[5], CubeVertices[1], CubeVertices[0], CubeVertices[4], ABlockTypeRegistery::BlockTypeRegistery->GetTextureForBlock(blockType, EChunkCubeFace::RIGHT), FVector(0, -1, 0));
		created |= true;
	}
	if (flags & (uint8_t)EChunkCubeFace::LEFT)
	{
		GenerateQuad(vertices, triangles, uvs, colors, normals, CubeVertices[6], CubeVertices[2], CubeVertices[3], CubeVertices[7], ABlockTypeRegistery::BlockTypeRegistery->GetTextureForBlock(blockType, EChunkCubeFace::LEFT), FVector(0, 1, 0));
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

int AChunk::Get1DIndex(FIntVector pos)
{
	return (pos.X + pos.Y * CHUNKSIZEX + pos.Z * CHUNKSIZEXY);
}

FIntVector AChunk::Get3DPosition(int idx)
{
	return FIntVector(
		idx % CHUNKSIZEX,
		(idx / CHUNKSIZEX) % CHUNKSIZEY,
		idx / CHUNKSIZEXY
	);
}

