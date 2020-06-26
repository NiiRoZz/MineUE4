// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Serialization/Archive.h"
#include "Block.generated.h"

/**
 * 
 */
UCLASS()
class MINEUE4_API UBlock: public UObject
{
  GENERATED_BODY()

public:
  UPROPERTY(Replicated)
  uint32 BlockType;

  UPROPERTY(Replicated)
  FIntVector RelativeLocation;

  bool IsSupportedForNetworking() const override;

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
