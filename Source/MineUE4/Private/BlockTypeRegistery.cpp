// Fill out your copyright notice in the Description page of Project Settings.

#include "BlockTypeRegistery.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"

//Define static member
ABlockTypeRegistery* ABlockTypeRegistery::BlockTypeRegistery = nullptr;

// Sets default values
ABlockTypeRegistery::ABlockTypeRegistery()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

  ABlockTypeRegistery::BlockTypeRegistery = this;
}

// Called every frame
void ABlockTypeRegistery::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

}

int32 ABlockTypeRegistery::GetBlockIdFromName(FName name)
{
  for (int32 i = 0; i < m_BlockTypes.Num(); ++i)
  {
    if (name == m_BlockTypes[i].Name)
    {
      return i;
    }
  }
  
  return -1;
}

FLinearColor ABlockTypeRegistery::GetTextureForBlock(int32 blockType, EChunkCubeFace face)
{
  if (blockType < 0 || !m_BlockTypes.IsValidIndex(blockType))
    return FLinearColor();

  /*if (face == EChunkCubeFace::TOP)
  {
    UE_LOG(LogTemp, Warning, TEXT("ABlockTypeRegistery::GetTextureForBlock 1 %s %d %d %f"), *(m_BlockTypes[blockType].Name.ToString()), face, m_BlockTypes[blockType].TextureTopId, (m_BlockTypes[blockType].TextureTopId / 4095.f));
  }*/

  switch (face)
  {
    case EChunkCubeFace::TOP:
    {
      return FLinearColor(m_BlockTypes[blockType].TextureTopId / 255.f, 0.f, 0.f);
    }

    case EChunkCubeFace::BOTTOM:
    {
      return FLinearColor(m_BlockTypes[blockType].TextureBottomId / 255.f, 0.f, 0.f);
    }

    case EChunkCubeFace::FRONT:
    {
      return FLinearColor(m_BlockTypes[blockType].TextureFrontId / 255.f, 0.f, 0.f);
    }

    case EChunkCubeFace::BACK:
    {
      return FLinearColor(m_BlockTypes[blockType].TextureBackId / 255.f, 0.f, 0.f);
    }

    case EChunkCubeFace::RIGHT:
    {
      return FLinearColor(m_BlockTypes[blockType].TextureRightId / 255.f, 0.f, 0.f);
    }

    case EChunkCubeFace::LEFT:
    {
      return FLinearColor(m_BlockTypes[blockType].TextureLeftId / 255.f, 0.f, 0.f);
    }
  }

  return FLinearColor();
}

FBlockType* ABlockTypeRegistery::GetBlockTypeById(int32 blockType)
{
  if (blockType < 0 || !m_BlockTypes.IsValidIndex(blockType))
    return nullptr;

  return &(m_BlockTypes[blockType]);
}

// Called when the game starts or when spawned
void ABlockTypeRegistery::BeginPlay()
{
	Super::BeginPlay();
	
  DrawTextureAtlas();
}

void ABlockTypeRegistery::DrawAtlasElement(UCanvas* canvas, UTexture2D* texture, uint32 currTextureIdx, FVector2D atlasElemSize)
{
  const int indexX = currTextureIdx % (int)atlasElemSize.X;
  const int indexY = currTextureIdx / (int)atlasElemSize.X;

  canvas->DrawTile(texture, (float)indexX * m_TextureSize, (float)indexY * m_TextureSize, m_TextureSize, m_TextureSize, 0, 0, texture->GetSizeX(), texture->GetSizeY());
}

void ABlockTypeRegistery::DrawTextureAtlas()
{
  if (!m_RenderTarget)
    return;

  UKismetRenderingLibrary::ClearRenderTarget2D(this, m_RenderTarget, FLinearColor::Black);

  UCanvas* canvas = nullptr;
  FVector2D size;
  FDrawToRenderTargetContext context;
  UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, m_RenderTarget, canvas, size, context);

  if (!canvas)
    return;

  TMap<UTexture2D*, uint32> texturesId;

  const FVector2D atlasElemSize = size / m_TextureSize;
  uint32 currTextureIdx = 0;

  for (int32 i = 0; i < m_BlockTypes.Num(); ++i)
  {
    //Top
    {
      if (m_BlockTypes[i].TextureTop)
      {
        uint32* textureIdx = texturesId.Find(m_BlockTypes[i].TextureTop);

        if (!textureIdx)
        {
          DrawAtlasElement(canvas, m_BlockTypes[i].TextureTop, currTextureIdx, atlasElemSize);

          m_BlockTypes[i].TextureTopId = currTextureIdx;
          texturesId.Add(m_BlockTypes[i].TextureTop, currTextureIdx);
          currTextureIdx++;
        }
        else
        {
          m_BlockTypes[i].TextureTopId = *textureIdx;
        }
      }
    }

    //Bottom
    {
      if (m_BlockTypes[i].TextureBottom)
      {
        uint32* textureIdx = texturesId.Find(m_BlockTypes[i].TextureBottom);

        if (!textureIdx)
        {
          DrawAtlasElement(canvas, m_BlockTypes[i].TextureBottom, currTextureIdx, atlasElemSize);

          m_BlockTypes[i].TextureBottomId = currTextureIdx;
          texturesId.Add(m_BlockTypes[i].TextureBottom, currTextureIdx);
          currTextureIdx++;
        }
        else
        {
          m_BlockTypes[i].TextureBottomId = *textureIdx;
        }
      }
    }

    //Front
    {
      if (m_BlockTypes[i].TextureFront)
      {
        uint32* textureIdx = texturesId.Find(m_BlockTypes[i].TextureFront);

        if (!textureIdx)
        {
          DrawAtlasElement(canvas, m_BlockTypes[i].TextureFront, currTextureIdx, atlasElemSize);

          m_BlockTypes[i].TextureFrontId = currTextureIdx;
          texturesId.Add(m_BlockTypes[i].TextureFront, currTextureIdx);
          currTextureIdx++;
        }
        else
        {
          m_BlockTypes[i].TextureFrontId = *textureIdx;
        }
      }
    }

    //Back
    {
      if (m_BlockTypes[i].TextureBack)
      {
        uint32* textureIdx = texturesId.Find(m_BlockTypes[i].TextureBack);

        if (!textureIdx)
        {
          DrawAtlasElement(canvas, m_BlockTypes[i].TextureBack, currTextureIdx, atlasElemSize);

          m_BlockTypes[i].TextureBackId = currTextureIdx;
          texturesId.Add(m_BlockTypes[i].TextureBack, currTextureIdx);
          currTextureIdx++;
        }
        else
        {
          m_BlockTypes[i].TextureBackId = *textureIdx;
        }
      }
    }

    //Right
    {
      if (m_BlockTypes[i].TextureRight)
      {
        uint32* textureIdx = texturesId.Find(m_BlockTypes[i].TextureRight);

        if (!textureIdx)
        {
          DrawAtlasElement(canvas, m_BlockTypes[i].TextureRight, currTextureIdx, atlasElemSize);

          m_BlockTypes[i].TextureRightId = currTextureIdx;
          texturesId.Add(m_BlockTypes[i].TextureRight, currTextureIdx);
          currTextureIdx++;
        }
        else
        {
          m_BlockTypes[i].TextureRightId = *textureIdx;
        }
      }
    }

    //Left
    {
      if (m_BlockTypes[i].TextureLeft)
      {
        uint32* textureIdx = texturesId.Find(m_BlockTypes[i].TextureLeft);

        if (!textureIdx)
        {
          DrawAtlasElement(canvas, m_BlockTypes[i].TextureLeft, currTextureIdx, atlasElemSize);

          m_BlockTypes[i].TextureLeftId = currTextureIdx;
          texturesId.Add(m_BlockTypes[i].TextureLeft, currTextureIdx);
          currTextureIdx++;
        }
        else
        {
          m_BlockTypes[i].TextureLeftId = *textureIdx;
        }
      }
    }
  }

  UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, context);
}

