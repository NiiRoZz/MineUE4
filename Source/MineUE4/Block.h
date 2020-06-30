// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"
#include "Block.generated.h"

/**
 * 
 */
USTRUCT()
struct MINEUE4_API FBlock : public FFastArraySerializerItem
{
  GENERATED_BODY()

public:
  UPROPERTY()
  uint32 BlockType = 0;

  UPROPERTY()
  FIntVector RelativeLocation;

  bool IsTransluscent();

  bool IsInvisible();

  bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FBlock> : public TStructOpsTypeTraitsBase2<FBlock>
{
  enum
  {
    WithNetSerializer = true
  };
};
