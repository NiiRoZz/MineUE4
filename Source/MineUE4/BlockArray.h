// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"
#include "BlockArray.generated.h"

USTRUCT()
struct FCompressedBlock : public FFastArraySerializerItem
{
  GENERATED_BODY()

  uint32 BlockType = 0;

  uint32 NmbBlocks = 1;

  bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FCompressedBlock> : public TStructOpsTypeTraitsBase2<FCompressedBlock>
{
  enum
  {
    WithNetSerializer = true
  };
};


USTRUCT()
struct MINEUE4_API FCompressedBlockArray : public FFastArraySerializer
{
  GENERATED_BODY()

  UPROPERTY()
  TArray<FCompressedBlock>  AllCompressedBlocks;

  bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
  {
    return FFastArraySerializer::FastArrayDeltaSerialize<FCompressedBlock, FCompressedBlockArray>(AllCompressedBlocks, DeltaParms, *this);
  }
};

template<>
struct TStructOpsTypeTraits< FCompressedBlockArray > : public TStructOpsTypeTraitsBase2< FCompressedBlockArray >
{
  enum
  {
    WithNetDeltaSerializer = true,
  };
};
