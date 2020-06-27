// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Engine/ActorChannel.h"
#include "../BlockArray.h"
#include "Chunk.generated.h"

enum class EChunkCubeFace : uint8_t
{
  TOP = 1 << 0,
  BOTTOM = 1 << 1,
  FRONT = 1 << 2,
  BACK = 1 << 3,
  RIGHT = 1 << 4,
  LEFT = 1 << 5,
  ALL = 63
};

UCLASS()
class MINEUE4_API AChunk : public AActor
{
  GENERATED_BODY()

public:
  static const uint16 CHUNKSIZEX = 16u;
  static const uint16 CHUNKSIZEY = 16u;
  static const uint16 CHUNKSIZEZ = 32u;
  static const uint16 CHUNKSIZEXY = CHUNKSIZEX * CHUNKSIZEY;
  
public:	
  // Sets default values for this actor's properties
  AChunk();

  // Called every frame
  virtual void Tick(float DeltaTime) override;

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  void UpdateVisibleBlocks();

  void SetBlock(FIntVector relativePos, FBlock &block);

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  /** "On receive chunk data" logic. Client-side*/
  UFUNCTION()
  void OnRep_VisibleBlocks();

  void GenerateQuad(TArray<FVector>& vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, FVector pos0, FVector pos1, FVector pos2, FVector pos3, FLinearColor color);

  void GenerateCube(TArray<FVector> &vertices, TArray<int32>& triangles, TArray<FVector2D>& uvs, TArray<FLinearColor>& colors, FVector pos, uint8_t flags);

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  UMaterial* m_Material;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
  UProceduralMeshComponent* m_ProceduralMesh;

private:

  UPROPERTY(ReplicatedUsing = OnRep_VisibleBlocks)
  FBlockArray	                m_VisibleBlocks;

  UPROPERTY()
  TSet<FIntVector>            m_VisibleBlocksPos;

  //Only used at server side
  UPROPERTY()
  TMap<FIntVector, FBlock>	  m_AllBlocks;

  UPROPERTY()
  TArray<int32>               m_CubeInstancies;
};
