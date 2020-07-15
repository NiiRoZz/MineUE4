// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Engine/ActorChannel.h"
#include "Block.h"
#include "BlockArray.h"
#include "BlockTypeRegistery.h"
#include "Chunk.generated.h"

UCLASS()
class MINEUE4_API AChunk : public AActor
{
	GENERATED_BODY()

public:
	static const uint16 CHUNKSIZEX = 16u;
	static const uint16 CHUNKSIZEY = 16u;
	static const uint16 CHUNKSIZEZ = 128u;
	static const uint16 CHUNKSIZEXY = CHUNKSIZEX * CHUNKSIZEY;
	static const uint16 CubeSize = 100u;
	static const uint16 MAXTYPEOFBLOCKS = 256u;
	
public:	
	// Sets default values for this actor's properties
	AChunk();
	~AChunk();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateCompressedBlocks();

	void SetBlock(FIntVector relativePos, FBlock &block);

	//Get chunk pos in chunk space
	FIntVector GetChunkPos();

	FBlock* GetBlock(FIntVector& relativePos);

	void Generate();

	void BuildMeshes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** "On receive chunk data" logic. Client-side*/
	UFUNCTION()
	void OnRep_CompressedBlocks();

	uint8_t GetCubeFlags(FIntVector& relativePos);

	void GenerateQuad(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, TArray<FVector>& normals, FVector pos0, FVector pos1, FVector pos2, FVector pos3, FLinearColor color, FVector normal);

	bool GenerateCube(TArray<FVector> &vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, TArray<FVector>& normals, uint32 blockType, FIntVector pos, uint8_t flags);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UProceduralMeshComponent* m_ProceduralMesh;

private:

	void FindChunkManager();

	int Get1DIndex(FIntVector pos);
	FIntVector Get3DPosition(int idx);

	UPROPERTY(ReplicatedUsing = OnRep_CompressedBlocks)
	TArray<FCompressedBlock>                                                m_CompressedBlocks;

	UPROPERTY()
	TSet<FIntVector>                                                        m_VisibleBlocksPos;

	//Contains every block
	TArray<FBlock, TFixedAllocator<CHUNKSIZEX * CHUNKSIZEY * CHUNKSIZEZ>>   m_AllBlocks;

	class AChunkManager*                                                    m_ChunkManager;
};
