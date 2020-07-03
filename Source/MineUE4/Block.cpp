// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"

bool FBlock::IsTransluscent()
{
  switch (BlockType)
  {
    case 255:
    {
      return true;
    }
  }

  return false;
}

bool FBlock::IsInvisible()
{
  switch (BlockType)
  {
    //AIR
    case 0:
    {
      return true;
    }
  }

  return false;
}

void FBlock::CopyFrom(FBlock& block)
{
  BlockType = block.BlockType;
}

void FBlock::CopyFrom(FCompressedBlock& block)
{
  BlockType = block.BlockType;
}
