// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"
#include "BlockTypeRegistery.h"

#include "Windows/PreWindowsApi.h"
#include "MineUE4/BitBuffer.h"
#include "Windows/PostWindowsApi.h"

bool FBlock::IsTransluscent()
{
	if (!ABlockTypeRegistery::BlockTypeRegistery)
		return false;

	FBlockType* blockType = ABlockTypeRegistery::BlockTypeRegistery->GetBlockTypeById(BlockType);
	if (!blockType)
		return false;

	return (blockType->Visibility == EBLockVisibility::TRANSLUSCENT);
}

bool FBlock::IsInvisible()
{
	if (!ABlockTypeRegistery::BlockTypeRegistery)
		return false;

	FBlockType* blockType = ABlockTypeRegistery::BlockTypeRegistery->GetBlockTypeById(BlockType);
	if (!blockType)
		return false;

	return (blockType->Visibility == EBLockVisibility::INVISIBLE);
}

void FBlock::CopyFrom(FBlock& block)
{
	BlockType = block.BlockType;
}

void FBlock::CopyFrom(FCompressedBlock& block)
{
	BlockType = block.BlockType;
}
