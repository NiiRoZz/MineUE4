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
	// Sets default values for this actor's properties
	AChunkManager();

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  void AddBlock(FIntVector& pos, uint32 BlockType);

  void AddChunk(AChunk* chunk);

  void RemoveChunk(AChunk* chunk);

  UMaterial* GetDefaultMaterialChunk();

  //Pos should be in cube space
  FBlock* GetBlock(FIntVector chunkPos, FIntVector relativePos);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterial* m_DefaultMaterialChunk;

private:	
	TMap<FIntVector, AChunk*> m_Chunks;
};
