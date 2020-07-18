// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockTypeRegistery.generated.h"

UENUM(BlueprintType)
enum class EBLockVisibility: uint8
{
	NONE = 0,
	OPAQUE = 1 << 0,
	TRANSLUSCENT = 1 << 1,
	INVISIBLE = 1 << 2,
	ALL = 7
};

UENUM()
enum class EChunkCubeFace : uint8
{
	TOP = 1 << 0,
	BOTTOM = 1 << 1,
	FRONT = 1 << 2,
	BACK = 1 << 3,
	RIGHT = 1 << 4,
	LEFT = 1 << 5,
	ALL = 63
};

USTRUCT(BlueprintType)
struct MINEUE4_API FBlockType
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBLockVisibility Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Visibility == EBLockVisibility::OPAQUE"))
	UTexture2D* TextureTop;
	uint32      TextureTopId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Visibility == EBLockVisibility::OPAQUE"))
	UTexture2D* TextureBottom;
	uint32      TextureBottomId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Visibility == EBLockVisibility::OPAQUE"))
	UTexture2D* TextureFront;
	uint32      TextureFrontId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Visibility == EBLockVisibility::OPAQUE"))
	UTexture2D* TextureBack;
	uint32      TextureBackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Visibility == EBLockVisibility::OPAQUE"))
	UTexture2D* TextureRight;
	uint32      TextureRightId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "Visibility == EBLockVisibility::OPAQUE"))
	UTexture2D* TextureLeft;
	uint32      TextureLeftId;
};

UCLASS()
class MINEUE4_API ABlockTypeRegistery : public AActor
{
	GENERATED_BODY()

public:
	static ABlockTypeRegistery* BlockTypeRegistery;
	
public:	
	// Sets default values for this actor's properties
	ABlockTypeRegistery();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	int32 GetBlockIdFromName(FName name);

	FLinearColor GetTextureForBlock(int32 blockType, EChunkCubeFace face);

	FBlockType* GetBlockTypeById(int32 blockType);

	int32 GetMaxTypeOfBlocks();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBlockType> m_BlockTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTextureRenderTarget2D* m_RenderTarget;

private:
	void DrawAtlasElement(UCanvas* canvas, UTexture2D* texture, uint32 currTextureIdx, const FVector2D& atlasElemSize);
	void DrawTextureAtlas(); 

	const int32 m_TextureSize = 16;
};
