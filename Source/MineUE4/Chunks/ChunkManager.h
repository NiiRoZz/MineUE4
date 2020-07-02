// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.h"
#include "ChunkManager.generated.h"

UCLASS()
class MINEUE4_API AChunkManager : public AActor
{
	GENERATED_BODY()

public:
  static const int CHUNKRENDERDISTANCE = 15;
	
public:	
	// Sets default values for this actor's properties
	AChunkManager();

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  void AddBlock(FIntVector& pos, uint32 BlockType);

  AChunk* CreateChunk(FIntVector chunkPos);

  void AddChunk(AChunk* chunk);

  void RemoveChunk(AChunk* chunk);

  UMaterial* GetDefaultOpaqueMaterialChunk();
  UMaterial* GetDefaultTransluscentMaterialChunk();

  //Pos should be in cube space
  FBlock* GetBlock(FIntVector chunkPos, FIntVector relativePos);

  AChunk** GetChunk(FIntVector chunkPos);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterial* m_DefaultOpaqueMaterialChunk;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterial* m_DefaulTransluscentMaterialChunk;

private:	
	TMap<FIntVector, AChunk*> m_Chunks;
};
