// LeroyWorks 2024 All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/Texture2D.h"
#include "Engine/Texture.h"
#include "TextureResource.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Engine/World.h"
#include "Async/ParallelFor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"

#include "FlowField.generated.h"

UENUM(BlueprintType)
enum class EInitMode : uint8
{
	Construction UMETA(DisplayName = "Construction"),
	BeginPlay UMETA(DisplayName = "BeginPlay"),
	Ticking UMETA(DisplayName = "Ticking")
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

USTRUCT(BlueprintType) struct FCellStruct
{
	GENERATED_BODY()

	public:

	BYTE cost = 255;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (DisplayName = "CostField", ToolTip = "The cost of this cell"))
	int32 costBP = cost;

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

UCLASS()
class FLOWFIELDCANVAS_API AFlowField : public AActor
{
	GENERATED_BODY()

public:

	AFlowField();
	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(CallInEditor, BlueprintCallable, Category = "FFCanvas")
	void DrawDebug();

	UFUNCTION(BlueprintCallable, Category = "FFCanvas")
	void TickFlowField();

	UFUNCTION(BlueprintCallable, Category = "FFCanvas")
	void WorldToGrid(const FVector Location, bool& bIsValid, FVector2D& gridCoord);

	UFUNCTION(BlueprintCallable, Category = "FFCanvas")
	void GetCellAtLocation(const FVector Location, bool& bIsValid, FCellStruct& CurrentCell);

	void InitFlowField(EInitMode InitMode);
	void InitFlowFieldMinimal(EInitMode InitMode);
	void CreateGrid();
	void UpdateGoalLocation();
	void CalculateFlowField();
	void DrawCells(EInitMode InitMode);
	void DrawArrows(EInitMode InitMode);
	void UpdateTimer();
	int32 AddWorldSpaceInstance(UInstancedStaticMeshComponent* ISM_Component, const FTransform& InstanceTransform);// for version api compatibility
	FCellStruct EnvQuery(const FVector2D gridCoord);




	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "Live update the flowfield in editor"))
	bool bEditorLiveUpdate = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the debug grid in-editor"))
	bool drawCellsInEditor = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the debug grid in-game"))
	bool drawCellsInGame = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the flow field arrows in-editor"))
	bool drawArrowsInEditor = true;

	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "FFCanvas|Visualize", meta = (ToolTip = "If True: Shows the flow field arrows in-game"))
	bool drawArrowsInGame = true;

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
	float updateInterval = 0.5f;


	//--------------------------------------------------------Configs-----------------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas|Material")
	UMaterialInterface* arrowBaseMat;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas|Material")
	UMaterialInterface* DecalBaseMat;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "FFCanvas", meta = (ToolTip = "Update this value manually if you don't want to fill in a goal actor"))
	FVector goalLocation = GetActorLocation();


	//--------------------------------------------------------ReadOnly-----------------------------------------------------------------

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (ToolTip = "Store ground info"))
	TMap<FVector2D, FCellStruct> InitialCellsMap;

	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, Category = "FFCanvas", meta = (ToolTip = "Store generated flow field data"))
	TMap<FVector2D, FCellStruct> CurrentCellsMap;


	//--------------------------------------------------------Cached-----------------------------------------------------------------

	float nextTickTimeLeft = updateInterval;
	bool bIsGridDirty = true;

	FVector actorLoc = GetActorLocation();
	FRotator actorRot = GetActorRotation();

	FVector offsetLoc = FVector(0, 0, 0);
	FVector relativeLoc = FVector(0, 0, 0);

	UPROPERTY()
	UMaterialInstanceDynamic* arrowDMI;

	UPROPERTY()
	UMaterialInstanceDynamic* DecalDMI;

	bool bIsValidGoalCoord = false;
	FVector2D goalGridCoord = FVector2D(0, 0);

	int32 xNum = FMath::RoundToInt(flowFieldSize.X / cellSize);
	int32 yNum = FMath::RoundToInt(flowFieldSize.Y / cellSize);

	UPROPERTY()
	UTexture2D* TransientTexture;


	//--------------------------------------------------------Components-----------------------------------------------------------------

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UBoxComponent* Volume;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UInstancedStaticMeshComponent* ISM_DirArrows;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UInstancedStaticMeshComponent* ISM_NullArrows;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UDecalComponent* Decal_Cells;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "FFCanvas|Components")
	UBillboardComponent* Billboard;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};