// LeroyWorks 2024 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "Engine/Texture2D.h"
#include "Engine/Texture.h"
#include "TextureResource.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Engine/World.h"
#include "Async/ParallelFor.h"
#include "Templates/Atomic.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"


#include "FlowField.generated.h"

//--------------------------ENUM-----------------------------

UENUM(BlueprintType)
enum class EInitMode : uint8
{
	Construction UMETA(DisplayName = "Construction"),
	BeginPlay UMETA(DisplayName = "BeginPlay"),
	Runtime UMETA(DisplayName = "Runtime")
};

UENUM(BlueprintType)
enum class EStyle : uint8
{
	AdjacentFirst UMETA(DisplayName = "AdjacentFirst"),
	DiagonalFirst UMETA(DisplayName = "DiagonalFirst")
};

UENUM(BlueprintType)
enum class ECellType : uint8
{
	Ground UMETA(DisplayName = "Ground"),
	Obstacle UMETA(DisplayName = "Obstacle"),
	Empty UMETA(DisplayName = "Empty")
};

UENUM(BlueprintType)
enum class EDigitType : uint8
{
	Cost UMETA(DisplayName = "Cost"),
	Dist UMETA(DisplayName = "Dist")
};

//--------------------------Struct-----------------------------

USTRUCT(BlueprintType) struct FCellStruct
{
	GENERATED_BODY()

	public:

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "CostField", ToolTip = "The cost of this cell"))
	int32 cost = 255;// using a byte here can improve performance but increases code complexity

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "IntegrationField", ToolTip = "Generated integration field. Same as water, it always flow towards the the ground with the lowest value nearby."))
	int32 dist = 65535;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "FlowField", ToolTip = "The direction the pawn need to go next when standing on this cell"))
	FVector dir = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "CellType", ToolTip = "The cell's type"))
	ECellType type = ECellType::Ground;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "XYCoordinate", ToolTip = "The coordinate of this cell on grid"))
	FVector2D gridCoord = FVector2D(0, 0);

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "WorldLocation", ToolTip = "The world location of this cell's center point"))
	FVector worldLoc = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "GroundNormal", ToolTip = "The ground's up vector, decided by line trace if ground trace enabled"))
	FVector normal = FVector(0, 0, 1);

};


//--------------------------FlowFieldClass-----------------------------

UCLASS()
class FLOWFIELDCANVAS_API AFlowField : public AActor
{
	GENERATED_BODY()

	//--------------------------------------------------------Functions-----------------------------------------------------------------

public:

	AFlowField();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "FFCanvas")
	void DrawDebug();

	UFUNCTION(BlueprintCallable, Category = "FFCanvas", meta = (ToolTip = "Recalculate flow field"))
	void UpdateFlowField();

	UFUNCTION(BlueprintCallable, Category = "FFCanvas", meta = (ToolTip = "Recalculate flow field periodically by timer"))
	void TickFlowField();

	UFUNCTION(BlueprintCallable, Category = "FFCanvas", meta = (ToolTip = "Get the grid coordinate at the given world location"))
	bool WorldToGridBP(UPARAM(ref) const FVector& Location, FVector2D& gridCoord);

	UFUNCTION(BlueprintCallable, Category = "FFCanvas", meta = (ToolTip = "Get the cell at the given world location"))
	bool GetCellAtLocationBP(UPARAM(ref) const FVector& Location, FCellStruct& CurrentCell);

	FORCEINLINE int32 CoordToIndex(const FVector2D& GridCoord)
	{
		return GridCoord.X * yNum + GridCoord.Y;
	};

	FORCEINLINE bool WorldToGrid(const FVector& Location, FVector2D& gridCoord)
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("WorldToGrid");

		FVector relativeLocation = (Location - actorLoc).RotateAngleAxis(-actorRot.Yaw, FVector(0, 0, 1)) + offsetLoc;
		float cellRadius = cellSize / 2;

		int32 gridCoordX = FMath::RoundToInt((relativeLocation.X - cellRadius) / cellSize);
		int32 gridCoordY = FMath::RoundToInt((relativeLocation.Y - cellRadius) / cellSize);

		// inside grid?
		bool bIsValidCoord = (gridCoordX >= 0 && gridCoordX < xNum) && (gridCoordY >= 0 && gridCoordY < yNum);

		// clamp to nearest
		gridCoord = FVector2D(FMath::Clamp(gridCoordX, 0, xNum - 1), FMath::Clamp(gridCoordY, 0, yNum - 1));

		return bIsValidCoord;
	};

	FORCEINLINE bool GetCellAtLocation(const FVector& Location, FCellStruct& CurrentCell)
	{
		//TRACE_CPUPROFILER_EVENT_SCOPE_STR("GetCellAtLocation");

		CurrentCell = FCellStruct();

		FVector2D NearestCoord;
		bool bIsValidCoord = WorldToGrid(Location, NearestCoord);

		int32 Index = CoordToIndex(NearestCoord);
		int32 CellCount = CurrentCellsArray.Num();

		bool bIsValidIndex = Index < CellCount;

		// clamp to max index
		int32 NearestIndex = FMath::Clamp(Index, 0, CellCount - 1);

		CurrentCell = CurrentCellsArray[NearestIndex];

		return bIsValidCoord && bIsValidIndex;
	};


	void InitFlowField(EInitMode InitMode);
	void GetGoalLocation();
	void CreateGrid();
	void CalculateFlowField(TArray<FCellStruct>& InCurrentCellsArray);
	void DrawCells(EInitMode InitMode);
	void DrawArrows(EInitMode InitMode);
	//void DrawDigits(EInitMode InitMode);
	void UpdateTimer();
	FCellStruct EnvQuery(const FVector2D gridCoord);


	//--------------------------------------------------------Exposed To Instance Settings-----------------------------------------------------------------

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "Live update the flowfield in editor"))
	bool bEditorLiveUpdate = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the debug grid in-editor"))
	bool drawCellsInEditor = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the flow field arrows in-editor"))
	bool drawArrowsInEditor = true;

	//UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Display digits in-editor"))
	//bool drawDigitsInEditor = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the debug grid in-game"))
	bool drawCellsInGame = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the flow field arrows in-game"))
	bool drawArrowsInGame = true;

	//UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Display digits in-game"))
	//bool drawDigitsInGame = true;

	//UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Display digits in-game"))
	//EDigitType DigitType = EDigitType::Cost;


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas", meta = (ToolTip = "Goal Actor. If none, will use variable goalLocation instead"))
	TSoftObjectPtr<AActor> goalActor;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas", meta = (ToolTip = "The scale of the flow field pathfinding boundary in units"))
	FVector flowFieldSize = FVector(1000, 1000, 300);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas", meta = (ToolTip = "The scale of the cells in units"))
	float cellSize = 150.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas", meta = (ToolTip = "Initial cost of all cells, ranging from 0 to 255"))
	int32 initialCost = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas", meta = (ToolTip = "Which direction flowfield prefer to go ?"))
	EStyle Style = EStyle::AdjacentFirst;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|EnvQuery", meta = (ToolTip = "If True: Trace all cells for any given Ground Object Type and aligns it to the ground"))
	bool traceGround = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|EnvQuery", meta = (ToolTip = "Object types that the flow field system will align to"))
	TArray<TEnumAsByte<EObjectTypeQuery>> groundObjectType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|EnvQuery", meta = (ToolTip = "If True: When creating the grid it will sphere-trace for any mesh that isnt of the given Ground Object Type"))
	bool traceObstacles = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|EnvQuery", meta = (ToolTip = "Type of object the flow field deem as an obstacle"))
	TArray<TEnumAsByte<EObjectTypeQuery>> obstacleObjectType;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|EnvQuery", meta = (ToolTip = "Cell inclination over this limit will be recognized as obstacle."))
	float maxWalkableAngle = 45.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|Performance", meta = (ToolTip = "How long until next update in seconds during runtime"))
	float RefreshInterval = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|Performance", meta = (ToolTip = "Skip cells inside obstacles during calculation"))
	bool bIgnoreInternalObstacleCells = false;


	//--------------------------------------------------------Not Exposed Settings-----------------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas|Material")
	UMaterialInterface* ArrowBaseMat;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas|Material")
	UMaterialInterface* CellBaseMat;

	//UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas|Material")
	//UMaterialInterface* DigitBaseMat;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas", meta = (ToolTip = "Update this value manually if you don't want to fill in a goal actor"))
	FVector goalLocation = GetActorLocation();

	//--------------------------------------------------------Components-----------------------------------------------------------------

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UBoxComponent* Volume;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UInstancedStaticMeshComponent* ISM_Arrows;

	//UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	//UInstancedStaticMeshComponent* ISM_Digits;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UDecalComponent* Decal_Cells;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UBillboardComponent* Billboard;

	//--------------------------------------------------------ReadOnly-----------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (ToolTip = "Store ground info"))
	TArray<FCellStruct> InitialCellsArray;
	//TArray<FCellStruct> InitialCellsArray;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (ToolTip = "Store generated flow field data"))
	TArray<FCellStruct> CurrentCellsArray;
	//TArray<FCellStruct> CurrentCellsArray;

	//--------------------------------------------------------Cached-----------------------------------------------------------------

	float nextTickTimeLeft = RefreshInterval;
	bool bIsGridDirty = true;
	bool bIsBeginPlay = true;

	FVector actorLoc = GetActorLocation();
	FRotator actorRot = GetActorRotation();

	FVector offsetLoc = FVector(0, 0, 0);
	FVector relativeLoc = FVector(0, 0, 0);

	UMaterialInstanceDynamic* ArrowDMI;
	UMaterialInstanceDynamic* CellDMI;
	UMaterialInstanceDynamic* DigitDMI;

	bool bIsValidGoalCoord = false;
	FVector2D goalGridCoord = FVector2D(0, 0);

	int32 xNum = FMath::RoundToInt(flowFieldSize.X / cellSize);
	int32 yNum = FMath::RoundToInt(flowFieldSize.Y / cellSize);

	UTexture2D* TransientTexture;

	TAtomic<int32> TraceRemaining{ 0 };

};
