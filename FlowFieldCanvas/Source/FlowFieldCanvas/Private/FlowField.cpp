// LeroyWorks 2024 All Rights Reserved.

#include "FlowField.h"
#include <queue>
#include <vector>
#include "Async/Async.h"
#include "Kismet/KismetSystemLibrary.h"

AFlowField::AFlowField()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultRoot;

	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetupAttachment(DefaultRoot); 
	Volume->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Volume->SetGenerateOverlapEvents(false);

	ISM_Arrows = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Direction Arrows"));
	ISM_Arrows->SetupAttachment(DefaultRoot);
	ISM_Arrows->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	ISM_Arrows->SetGenerateOverlapEvents(false);
	ISM_Arrows->SetCastShadow(false);
	ISM_Arrows->SetReceivesDecals(false);

	//ISM_Digits = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Digit Display"));
	//ISM_Digits->SetupAttachment(DefaultRoot);
	//ISM_Digits->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	//ISM_Digits->SetGenerateOverlapEvents(false);
	//ISM_Digits->SetCastShadow(false);
	//ISM_Digits->SetReceivesDecals(false);

	Decal_Cells = CreateDefaultSubobject<UDecalComponent>(TEXT("Cell Decal"));
	Decal_Cells->SetupAttachment(DefaultRoot);
	Decal_Cells->SetRelativeRotation(FRotator(90,0,0));

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("BillBoard"));
	Billboard->SetupAttachment(DefaultRoot);
}

void AFlowField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	DrawDebug();
}

void AFlowField::DrawDebug()
{
	InitFlowField(EInitMode::Construction);

	if (bEditorLiveUpdate)
	{
		GetGoalLocation();

		CreateGrid();

		CurrentCellsArray = InitialCellsArray;

		CalculateFlowField(CurrentCellsArray);

		DrawCells(EInitMode::Construction);

		DrawArrows(EInitMode::Construction);

		//DrawDigits(EInitMode::Construction);
	}
}

void AFlowField::UpdateFlowField()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("UpdateFlowField");

	EInitMode InitMode = bIsBeginPlay ? EInitMode::BeginPlay : EInitMode::Runtime;

	InitFlowField(InitMode);

	GetGoalLocation();

	CreateGrid();

	CurrentCellsArray = InitialCellsArray;

	CalculateFlowField(CurrentCellsArray);

	DrawCells(InitMode);

	DrawArrows(InitMode);

	//DrawDigits(InitMode);

	bIsBeginPlay = false;
}

void AFlowField::TickFlowField()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("TickFlowField");

	if (nextTickTimeLeft <= 0)
	{
		UpdateFlowField();
	}

	UpdateTimer();
}

bool AFlowField::WorldToGridBP(UPARAM(ref) const FVector& Location, FVector2D& gridCoord)
{
	return WorldToGrid(Location, gridCoord);
}

bool AFlowField::GetCellAtLocationBP(UPARAM(ref) const FVector& Location, FCellStruct& CurrentCell)
{
	return GetCellAtLocation(Location, CurrentCell);
}

void AFlowField::InitFlowField(EInitMode InitMode)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("InitFlowField");

	if (InitMode == EInitMode::Construction || InitMode == EInitMode::BeginPlay)
	{
		nextTickTimeLeft = FMath::Clamp(RefreshInterval, 0.f, FLT_MAX);
		bIsGridDirty = true;
	}

	cellSize = FMath::Clamp(cellSize, 0.1f, FLT_MAX);
	flowFieldSize = FVector(FMath::Clamp(flowFieldSize.X, cellSize, FLT_MAX), FMath::Clamp(flowFieldSize.Y, cellSize, FLT_MAX), flowFieldSize.Z);

	xNum = FMath::RoundToInt(flowFieldSize.X / cellSize);
	yNum = FMath::RoundToInt(flowFieldSize.Y / cellSize);

	actorLoc = GetActorLocation();
	actorRot = GetActorRotation();

	offsetLoc = FVector(xNum * cellSize / 2, yNum * cellSize / 2, 0);
	relativeLoc = actorLoc - offsetLoc;

	FVector halfSize = FVector(xNum * cellSize, yNum * cellSize, flowFieldSize.Z) / 2;

	Volume->SetBoxExtent(halfSize);
	Volume->SetRelativeLocation(FVector(0.f, 0.f, halfSize.Z));

	initialCost = FMath::Clamp(initialCost, 0, 255);

	// decal
	Decal_Cells->DecalSize = FVector(halfSize.Z, halfSize.Y, halfSize.X);
	Decal_Cells->SetRelativeLocation(FVector(0.f, 0.f, halfSize.Z));

	// dynamic materials
	if ((InitMode == EInitMode::Construction) || InitMode == EInitMode::BeginPlay)
	{
		if (IsValid(CellBaseMat))// for cell decal
		{
			CellDMI = UMaterialInstanceDynamic::Create(CellBaseMat, this);

			if (IsValid(CellDMI))
			{
				Decal_Cells->SetMaterial(0, CellDMI);
			}
		}

		if (IsValid(ArrowBaseMat))// for arrow ism
		{
			ArrowDMI = UMaterialInstanceDynamic::Create(ArrowBaseMat, this);

			if (IsValid(ArrowDMI))
			{
				ISM_Arrows->SetMaterial(0, ArrowDMI);
			}
		}

		//if (IsValid(DigitBaseMat))// for digit ism
		//{
		//	DigitDMI = UMaterialInstanceDynamic::Create(DigitBaseMat, this);

		//	if (IsValid(DigitDMI))
		//	{
		//		ISM_Digits->SetMaterial(0, DigitDMI);
		//	}
		//}
	}

	if (IsValid(CellDMI))
	{
		CellDMI->SetScalarParameterValue("XNum", xNum);
		CellDMI->SetScalarParameterValue("YNum", yNum);
		CellDMI->SetScalarParameterValue("CellSize", cellSize);
		CellDMI->SetScalarParameterValue("OffsetX", offsetLoc.X);
		CellDMI->SetScalarParameterValue("OffsetY", offsetLoc.Y);
		CellDMI->SetScalarParameterValue("Yaw", actorRot.Yaw);
	}

	// toggle visibility of arrows, cells and digits
	if ((InitMode == EInitMode::Construction && drawCellsInEditor) || (InitMode != EInitMode::Construction && drawCellsInGame))
	{
		Decal_Cells->SetVisibility(true);
	}
	else
	{
		Decal_Cells->SetVisibility(false);
	}

	if ((InitMode == EInitMode::Construction && drawArrowsInEditor) || (InitMode != EInitMode::Construction && drawArrowsInGame))
	{
		ISM_Arrows->SetVisibility(true);
	}
	else
	{
		ISM_Arrows->SetVisibility(false);
	}

	//if ((InitMode == EInitMode::Construction && drawDigitsInEditor) || (InitMode != EInitMode::Construction && drawDigitsInGame))
	//{
	//	ISM_Digits->SetVisibility(true);
	//}
	//else
	//{
	//	ISM_Digits->SetVisibility(false);
	//}
}

void AFlowField::GetGoalLocation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("GetGoalLocation");

	if (IsValid(goalActor.Get()))
	{
		goalLocation = goalActor->GetActorLocation();
		WorldToGrid(goalLocation, goalGridCoord);
	}
	else
	{
		WorldToGrid(goalLocation, goalGridCoord);
	}
}

void AFlowField::CreateGrid()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("CreateGrid");

	if (!bIsGridDirty) return;

	InitialCellsArray.Empty();
	CurrentCellsArray.Empty();

	for (int x = 0; x < xNum; ++x)
	{
		for (int y = 0; y < yNum; ++y)
		{
			FVector2D gridCoord = FVector2D(x, y);
			FCellStruct newCell = EnvQuery(gridCoord);
			InitialCellsArray.Add(newCell);
		}
	}

	bIsGridDirty = false;
}

void AFlowField::CalculateFlowField(TArray<FCellStruct>& InCurrentCellsArray)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("CalculateFlowField");

	int32 numCells = InCurrentCellsArray.Num();

	auto IsValidCoord = [&](FVector2D gridCoord) -> bool { return gridCoord.X >= 0 && gridCoord.X < xNum && gridCoord.Y >= 0 && gridCoord.Y < yNum; };
	auto IsValidDiagonal = [&](std::vector<FVector2D> neighborCoords, int32 indexA, int32 indexB) -> bool
		{
			bool result = true;

			if (IsValidCoord(neighborCoords[indexA]))
			{
				if (InCurrentCellsArray[CoordToIndex(neighborCoords[indexA])].cost == 255)
				{
					result = false;
				}
			}
			if (IsValidCoord(neighborCoords[indexB]))
			{
				if (InCurrentCellsArray[CoordToIndex(neighborCoords[indexB])].cost == 255)
				{
					result = false;
				}
			}
			return result;
		};

	struct CostCompare {
		bool operator()(const FCellStruct& lhs, const FCellStruct& rhs) const {
			return lhs.dist > rhs.dist;
		}
	};

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("CreateIntegrationField");

		FCellStruct& targetCell = InCurrentCellsArray[CoordToIndex(goalGridCoord)];
		targetCell.cost = 0;
		targetCell.dist = 0;

		std::priority_queue<FCellStruct, std::vector<FCellStruct>, CostCompare> CellsToCheck;
		CellsToCheck.push(targetCell);

		while (!CellsToCheck.empty())
		{
			FCellStruct currentCell = CellsToCheck.top();
			FVector2D currentCoord = currentCell.gridCoord;
			CellsToCheck.pop();

			std::vector<FVector2D> neighborCoords;

			if (Style == EStyle::AdjacentFirst)
			{
				neighborCoords = {
					currentCoord + FVector2D(1,-1),
					currentCoord + FVector2D(1,1),
					currentCoord + FVector2D(-1,1),
					currentCoord + FVector2D(-1,-1),
					currentCoord + FVector2D(0,-1),
					currentCoord + FVector2D(1,0),
					currentCoord + FVector2D(0,1),
					currentCoord + FVector2D(-1,0)
				};
			}
			else if (Style == EStyle::DiagonalFirst)
			{
				neighborCoords = {
					currentCoord + FVector2D(0,-1),
					currentCoord + FVector2D(1,0),
					currentCoord + FVector2D(0,1),
					currentCoord + FVector2D(-1,0)
				};
			}

			for (int32 i = 0; i < neighborCoords.size(); ++i)
			{
				FVector2D neighborCoord = neighborCoords[i];

				if (!IsValidCoord(neighborCoord)) continue;

				FCellStruct& neighborCell = InCurrentCellsArray[CoordToIndex(neighborCoord)];

				if (bIgnoreInternalObstacleCells && neighborCell.cost == 255) continue;

				if (Style == EStyle::AdjacentFirst)
				{
					if (i == 0 && !IsValidDiagonal(neighborCoords, 4, 5)) continue;
					if (i == 1 && !IsValidDiagonal(neighborCoords, 5, 6)) continue;
					if (i == 2 && !IsValidDiagonal(neighborCoords, 6, 7)) continue;
					if (i == 3 && !IsValidDiagonal(neighborCoords, 7, 4)) continue;
				}

				float heightDifference = FMath::Abs(currentCell.worldLoc.Z - neighborCell.worldLoc.Z);
				float horizontalDistance = FVector2D::Distance(FVector2D(currentCell.worldLoc.X, currentCell.worldLoc.Y), FVector2D(neighborCell.worldLoc.X, neighborCell.worldLoc.Y));
				float slopeAngle = FMath::RadiansToDegrees(FMath::Atan(heightDifference / horizontalDistance));

				if (slopeAngle > maxWalkableAngle && currentCell.cost != 255) continue;

				int32 newDist = neighborCell.cost + currentCell.dist;

				if (newDist < neighborCell.dist)
				{
					neighborCell.dist = newDist;
					CellsToCheck.push(neighborCell);
				}
			}
		}
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE_STR("CreateFlowField");

		ParallelFor(numCells, [&](int32 j)
			{
				int32 currentIndex = j;
				FVector2D currentCoord = FVector2D(currentIndex / yNum, currentIndex % yNum);
				FCellStruct& currentCell = InCurrentCellsArray[CoordToIndex(currentCoord)];

				bool hasBestCell = false;
				FCellStruct bestCell;
				int32 bestDist = currentCell.dist;

				std::vector<FVector2D> neighborCoords =
				{
					currentCoord + FVector2D(0,-1),
					currentCoord + FVector2D(1,0),
					currentCoord + FVector2D(0,1),
					currentCoord + FVector2D(-1,0),
					currentCoord + FVector2D(1,-1),
					currentCoord + FVector2D(1,1),
					currentCoord + FVector2D(-1,1),
					currentCoord + FVector2D(-1,-1),
				};

				for (int32 i = 0; i < 8; i++)
				{
					FVector2D neighborCoord = neighborCoords[i];

					if (!IsValidCoord(neighborCoord)) continue;

					FCellStruct& neighborCell = InCurrentCellsArray[CoordToIndex(neighborCoord)];

					if (bIgnoreInternalObstacleCells && neighborCell.cost == 255) continue;

					if (currentCell.cost != 255)
					{
						if (i == 4 && !IsValidDiagonal(neighborCoords, 0, 1)) continue;
						if (i == 5 && !IsValidDiagonal(neighborCoords, 1, 2)) continue;
						if (i == 6 && !IsValidDiagonal(neighborCoords, 2, 3)) continue;
						if (i == 7 && !IsValidDiagonal(neighborCoords, 3, 0)) continue;
					}

					float heightDifference = FMath::Abs(currentCell.worldLoc.Z - neighborCell.worldLoc.Z);
					float horizontalDistance = FVector2D::Distance(FVector2D(currentCell.worldLoc.X, currentCell.worldLoc.Y), FVector2D(neighborCell.worldLoc.X, neighborCell.worldLoc.Y));
					float slopeAngle = FMath::RadiansToDegrees(FMath::Atan(heightDifference / horizontalDistance));

					if (slopeAngle > maxWalkableAngle && currentCell.cost != 255) continue;

					if (neighborCell.dist < bestDist)
					{
						hasBestCell = true;
						bestCell = neighborCell;
						bestDist = neighborCell.dist;
					}
				}

				if (hasBestCell)
				{
					currentCell.dir = UKismetMathLibrary::GetDirectionUnitVector(currentCell.worldLoc, bestCell.worldLoc);
				}
				else
				{
					currentCell.dir = FVector::ZeroVector;
				}
			});
	}

}

void AFlowField::DrawCells(EInitMode InitMode)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("DrawCells");

	if ((InitMode == EInitMode::Construction && drawCellsInEditor) || (InitMode != EInitMode::Construction && drawCellsInGame))
	{
		float largestCellDist = 0.0001f;

		for (const FCellStruct& Cell : CurrentCellsArray)
		{
			if (Cell.dist > largestCellDist && Cell.dist != 65535)
			{
				largestCellDist = Cell.dist;
			}
		}

		// Create transient texture
		TransientTexture = UTexture2D::CreateTransient(xNum, yNum);

		FTexture2DMipMap* MipMap = &TransientTexture->GetPlatformData()->Mips[0];// for version api compatibility
		FByteBulkData* ImageData = &MipMap->BulkData;
		uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

		for (const FCellStruct& Cell : CurrentCellsArray)
		{
			// Calculate pixel index based on grid coordinates
			int32 PixelX = xNum - Cell.gridCoord.X - 1;
			int32 PixelY = Cell.gridCoord.Y;
			int32 PixelIndex = (PixelY * xNum + PixelX) * 4;

			RawImageData[PixelIndex + 3] = Cell.type != ECellType::Empty ? 255 : 0; // Set the Alpha channel
			RawImageData[PixelIndex + 2] = Cell.cost; // Set the R channel with cost value
			RawImageData[PixelIndex + 1] = 0;
			RawImageData[PixelIndex + 0] = FMath::Clamp(FMath::RoundToInt(FMath::Clamp(float(Cell.dist), 0.f, largestCellDist) / largestCellDist * 255), 0, 255); // Set the B channel with dist
		}

		ImageData->Unlock();
		TransientTexture->UpdateResource();

		// update decal material
		if (IsValid(CellDMI))
		{
			CellDMI->SetTextureParameterValue("TransientTexture", TransientTexture);
		}
	}
}

void AFlowField::DrawArrows(EInitMode InitMode)
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("DrawArrows");

	// Initialize instances if needed
	if (InitMode == EInitMode::Construction || InitMode == EInitMode::BeginPlay)
	{
		ISM_Arrows->ClearInstances();

		TArray<FTransform> TranArray;
		TranArray.Init(FTransform(FVector::ZeroVector), InitialCellsArray.Num());

		ISM_Arrows->AddInstances(TranArray, false);
	}

	if ((InitMode == EInitMode::Construction && drawArrowsInEditor) || (InitMode != EInitMode::Construction && drawArrowsInGame))
	{
		for (const FCellStruct& Cell : CurrentCellsArray)
		{
			FVector Dir = Cell.dir;
			FVector Normal = Cell.normal.GetSafeNormal();
			Dir = FVector::VectorPlaneProject(Dir, Normal).GetSafeNormal();
			FQuat AlignToNormalQuat = FQuat::FindBetweenNormals(FVector::UpVector, Normal);
			FVector AlignedDir = AlignToNormalQuat.RotateVector(FVector::ForwardVector);
			FQuat RotateInPlaneQuat = FQuat::FindBetweenNormals(AlignedDir, Dir);
			FQuat CombinedQuat = RotateInPlaneQuat * AlignToNormalQuat;
			FRotator AlignToNormalRot = CombinedQuat.Rotator();
			FTransform Trans(AlignToNormalRot, Cell.worldLoc + Cell.normal, FVector(cellSize / 200.0f));

			int32 CellIndex = CoordToIndex(Cell.gridCoord);

			if (Cell.type == ECellType::Empty)
			{
				// If no ground, hide
				Trans.SetScale3D(FVector::ZeroVector);
				ISM_Arrows->UpdateInstanceTransform(CellIndex, Trans, true, false, true);
			}
			else if (Dir.Size() == 0)
			{
				// If it has no direction, set custom data = 2 (so they will be red dots)
				//Trans = ToLocalTransform(ISM_Digits, Trans);
				ISM_Arrows->UpdateInstanceTransform(CellIndex, Trans, true, false, true);
				ISM_Arrows->SetCustomDataValue(CellIndex, 0, 2, false);
			}
			else				
			{
				// If it was not modified, set custom data = 0 (so they will be white arrows)
				//Trans = ToLocalTransform(ISM_Arrows, Trans);
				ISM_Arrows->UpdateInstanceTransform(CellIndex, Trans, true, false, true);
				ISM_Arrows->SetCustomDataValue(CellIndex, 0, 0, false);
			}
		}

		ISM_Arrows->MarkRenderStateDirty();
	}
}

//void AFlowField::DrawDigits(EInitMode InitMode)
//{
//	TRACE_CPUPROFILER_EVENT_SCOPE_STR("DrawDigits");
//
//	if (InitMode == EInitMode::Construction || InitMode == EInitMode::BeginPlay)
//	{
//		ISM_Digits->ClearInstances();
//
//		TArray<FTransform> TranArray;
//		TranArray.Init(FTransform(FVector::ZeroVector), InitialCellsArray.Num());
//
//		ISM_Digits->AddInstances(TranArray, false);
//	}
//
//	if ((InitMode == EInitMode::Construction && drawDigitsInEditor) || (InitMode != EInitMode::Construction && drawDigitsInGame))
//	{
//		for (const TPair<FVector2D, FCellStruct>& Element : CurrentCellsArray)
//		{
//			const FCellStruct& Cell = Element.Value;
//
//			FVector Dir = Cell.dir;
//			FVector Normal = Cell.normal.GetSafeNormal();
//			Dir = FVector::VectorPlaneProject(Dir, Normal).GetSafeNormal();
//			FQuat AlignToNormalQuat = FQuat::FindBetweenNormals(FVector::UpVector, Normal);
//			FVector AlignedDir = AlignToNormalQuat.RotateVector(FVector::ForwardVector);
//			FQuat RotateInPlaneQuat = FQuat::FindBetweenNormals(AlignedDir, Dir);
//			FQuat CombinedQuat = RotateInPlaneQuat * AlignToNormalQuat;
//			FRotator AlignToNormalRot = CombinedQuat.Rotator();
//			FTransform Trans(AlignToNormalRot, Cell.worldLoc + Cell.normal, FVector(cellSize / 130.0f));
//
//			int32 CellIndex = CoordToIndex(Cell.gridCoord);
//
//			if (Cell.type == ECellType::Empty)
//			{
//				// If no ground, hide
//				Trans.SetScale3D(FVector::ZeroVector);
//				ISM_Digits->UpdateInstanceTransform(CellIndex, Trans, false, false, true);
//			}
//			else if (DigitType == EDigitType::Cost)
//			{
//				Trans = ToLocalTransform(ISM_Digits, Trans);
//				Trans.SetRotation(FQuat::Identity);
//				ISM_Digits->UpdateInstanceTransform(CellIndex, Trans, false, false, true);
//				ISM_Digits->SetCustomDataValue(CellIndex, 0, Cell.cost, false);
//			}
//			else
//			{
//				Trans = ToLocalTransform(ISM_Arrows, Trans);
//				Trans.SetRotation(FQuat::Identity);
//				ISM_Digits->UpdateInstanceTransform(CellIndex, Trans, false, false, true);
//				ISM_Digits->SetCustomDataValue(CellIndex, 0, Cell.dist, false);
//			}
//		}
//
//		ISM_Digits->MarkRenderStateDirty();
//	}
//
//}

void AFlowField::UpdateTimer()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("UpdateTimer");

	if (nextTickTimeLeft <= 0)
	{
		nextTickTimeLeft = FMath::Clamp(RefreshInterval, 0.f, FLT_MAX);
	}
	else
	{
		nextTickTimeLeft -= GetWorld()->GetDeltaSeconds();
	}
}
   
FCellStruct AFlowField::EnvQuery(const FVector2D gridCoord) 
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("EnvQuery");

	FCellStruct newCell;

	newCell.gridCoord = gridCoord;

	FVector2D worldLoc2D = FVector2D(gridCoord.X * cellSize + relativeLoc.X + (cellSize / 2.f), gridCoord.Y * cellSize + relativeLoc.Y + (cellSize / 2.f));
	FVector worldLoc = FVector(worldLoc2D, actorLoc.Z);
	worldLoc = (worldLoc - actorLoc).RotateAngleAxis(actorRot.Yaw, FVector(0, 0, 1)) + actorLoc;

	// Array of actors for trace to ignore
	TArray<AActor*> IgnoreActors;

	if (traceGround)
	{
		FHitResult GroundHitResult;

		bool hitGround = UKismetSystemLibrary::LineTraceSingleForObjects(
			GetWorld(),
			FVector(worldLoc.X, worldLoc.Y, actorLoc.Z + flowFieldSize.Z),
			FVector(worldLoc.X, worldLoc.Y, actorLoc.Z),
			groundObjectType,
			true,
			IgnoreActors,
			EDrawDebugTrace::None,
			GroundHitResult,
			true,
			FLinearColor::Gray,
			FLinearColor::Blue,
			3);

		if (hitGround)
		{
			worldLoc.Z = GroundHitResult.ImpactPoint.Z;

			newCell.normal = GroundHitResult.ImpactNormal;
			newCell.type = ECellType::Ground;

			float cellAngle = UKismetMathLibrary::Acos(FVector::DotProduct(GroundHitResult.ImpactNormal, FVector(0, 0, 1)));
			float maxAngleRadians = FMath::DegreesToRadians(maxWalkableAngle);

			if (cellAngle > maxAngleRadians)
			{
				newCell.cost = 255;
				newCell.type = ECellType::Obstacle;
			}
			else if (traceObstacles)
			{
				FHitResult ObstacleHitResult;
				bool hitObstacle = UKismetSystemLibrary::SphereTraceSingleForObjects(
					GetWorld(),
					FVector(worldLoc.X, worldLoc.Y, actorLoc.Z + flowFieldSize.Z),
					FVector(worldLoc.X, worldLoc.Y, actorLoc.Z),
					cellSize / 2.f,
					obstacleObjectType,
					true,
					IgnoreActors,
					EDrawDebugTrace::None,
					ObstacleHitResult,
					true,
					FLinearColor::Gray,
					FLinearColor::Red,
					1);

				if (hitObstacle)
				{
					if (ObstacleHitResult.ImpactPoint.Z > GroundHitResult.ImpactPoint.Z)
					{
						newCell.cost = 255;
						newCell.type = ECellType::Obstacle;
					}
					else
					{
						newCell.cost = initialCost;

						if (initialCost == 255)
						{
							newCell.type = ECellType::Obstacle;
						}
					}
				}
				else
				{
					newCell.cost = initialCost;

					if (initialCost == 255)
					{
						newCell.type = ECellType::Obstacle;
					}
				}
			}
			else
			{
				newCell.cost = initialCost;

				if (initialCost == 255)
				{
					newCell.type = ECellType::Obstacle;
				}
			}

		}
		else 
		{
			worldLoc.Z = -FLT_MAX;
			newCell.normal = FVector(0, 0, 1);
			newCell.type = ECellType::Empty;
		}
	}
	else
	{
		worldLoc.Z += flowFieldSize.Z / 2;
		newCell.normal = FVector(0, 0, 1);
		newCell.type = ECellType::Ground;

		if (traceObstacles)
		{
			FHitResult ObstacleHitResult;
			bool hitObstacle = UKismetSystemLibrary::SphereTraceSingleForObjects(
				GetWorld(),
				FVector(worldLoc.X, worldLoc.Y, actorLoc.Z + flowFieldSize.Z),
				FVector(worldLoc.X, worldLoc.Y, actorLoc.Z),
				cellSize / 2.f,
				obstacleObjectType,
				true,
				IgnoreActors,
				EDrawDebugTrace::None,
				ObstacleHitResult,
				true,
				FLinearColor::Gray,
				FLinearColor::Red,
				1);

			if (hitObstacle)
			{
				if (ObstacleHitResult.ImpactPoint.Z > worldLoc.Z)
				{
					newCell.cost = 255;

					newCell.type = ECellType::Obstacle;
				}
				else
				{
					newCell.cost = initialCost;

					if (initialCost == 255)
					{
						newCell.type = ECellType::Obstacle;
					}
				}
			}
			else
			{
				newCell.cost = initialCost;

				if (initialCost == 255)
				{
					newCell.type = ECellType::Obstacle;
				}
			}
		}
		else
		{
			newCell.cost = initialCost;

			if (initialCost == 255)
			{
				newCell.type = ECellType::Obstacle;
			}
		}
	}

	newCell.worldLoc = worldLoc;
	return newCell;
}