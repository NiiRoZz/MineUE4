// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlockArray.h"
#include "Block.generated.h"

/**
 * 
 */
USTRUCT()
struct MINEUE4_API FBlock
{
  GENERATED_BODY()

public:
  UPROPERTY()
  uint32 BlockType = 0;

  bool IsTransluscent();

  bool IsInvisible();

  void CopyFrom(FBlock &block);
  void CopyFrom(FCompressedBlock& block);
};
