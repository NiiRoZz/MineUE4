// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/Archive.h"
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
  uint32 BlockType;

  UPROPERTY()
  FIntVector RelativeLocation;
};
