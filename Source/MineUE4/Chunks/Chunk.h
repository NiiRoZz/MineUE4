// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/ActorChannel.h"
#include "../Block.h"
#include "Chunk.generated.h"

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

  bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

  UFUNCTION()
  void UpdateVisibleBlocks();

  TArray<UBlock*>& GetAllBlocks();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  /** "On receive chunk data" logic. Client-side*/
  UFUNCTION()
  void OnRep_VisibleBlocks();

private:

  //Only used at client side
  UPROPERTY(ReplicatedUsing = OnRep_VisibleBlocks)
  TArray<UBlock*>   m_VisibleBlocks;

  //Only used at server side
  UPROPERTY()
  TArray<UBlock*>	  m_AllBlocks;
};
