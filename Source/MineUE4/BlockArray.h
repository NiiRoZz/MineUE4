// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Block.h"
#include "BlockArray.generated.h"

/**
 * 
 */
USTRUCT()
struct MINEUE4_API FBlockArray : public FFastArraySerializer
{
  GENERATED_BODY()

  UPROPERTY()
  TArray<FBlock>  VisibleBlocks;

  bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
  {
    return FFastArraySerializer::FastArrayDeltaSerialize<FBlock, FBlockArray>(VisibleBlocks, DeltaParms, *this);
  }
};

template<>
struct TStructOpsTypeTraits< FBlockArray > : public TStructOpsTypeTraitsBase2< FBlockArray >
{
  enum
  {
    WithNetDeltaSerializer = true,
  };
};
