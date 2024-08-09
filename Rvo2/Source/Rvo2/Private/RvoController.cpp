// Fill out your copyright notice in the Description page of Project Settings.


#include "RvoController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Rvo2/Rvo2Library/Vector2.h"

// Sets default values for this component's properties
URvoController::URvoController(): RvoSim(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void URvoController::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

void URvoController::RVO_UpdateAgentVelocity()
{
	TArray<int32> RemoveIndex;
	for (auto& AgentData : AgentDataMap)
	{
		if (!AgentData.Value.isDeath)
		{
			//同步位置
			RVO::Vector2 pos = RvoSim->getAgentPosition(AgentData.Value.RvoID);
			RVO::Vector2 oldPos = RvoSim->getAgentOldPosition(AgentData.Value.RvoID);
			FVector Location = FVector(pos.x() * RvoSettings.RVO_LocationScale, pos.y() * RvoSettings.RVO_LocationScale,
			                           0);
			FVector OldLocation = FVector(oldPos.x() * RvoSettings.RVO_LocationScale,
			                              oldPos.y() * RvoSettings.RVO_LocationScale, 0);
			FVector TargetLocation = FVector(AgentData.Value.TargetLocation.X, AgentData.Value.TargetLocation.Y, 0);
			AgentData.Value.CurrentLocation = OldLocation;
			FRotator TargetRotator = UKismetMathLibrary::FindLookAtRotation(OldLocation, TargetLocation);
			AgentData.Value.CurrentRotator = FMath::Lerp<FRotator>(AgentData.Value.CurrentRotator, TargetRotator,
			                                                       RvoSettings.TickDeltaTime);
			//计算移动方向
			const FVector vector = TargetLocation - AgentData.Value.CurrentLocation;
			RVO::Vector2 goalVector = RVO::Vector2(vector.X, vector.Y);
			if (RVO::absSq(goalVector) > 1.0f)
			{
				goalVector = RVO::normalize(goalVector);
			}
			const bool IsReachedAttackRange = RVO_IsReachedAttackRange(AgentData.Value.RvoID);
			if (IsReachedAttackRange && !AgentData.Value.CanAttack)
			{
				AgentData.Value.CanAttack = true;
				RvoSim->setAgentPrefVelocity(AgentData.Value.RvoID, RVO::Vector2(0.0f, 0.0f));
				OnAgentEnterAttack.Broadcast(AgentData.Value.RvoID);
			}
			if (!IsReachedAttackRange)
			{
				RvoSim->setAgentPrefVelocity(AgentData.Value.RvoID, goalVector);
				if (AgentData.Value.CanAttack)
				{
					AgentData.Value.CanAttack = false;
					OnAgentExitAttack.Broadcast(AgentData.Value.RvoID);
				}
			}
		}
		else
		{
			RemoveIndex.Add(AgentData.Value.RvoID);
		}
	}

	for (const auto Index : RemoveIndex)
	{
		RVO_Dormant(Index);
	}
}

void URvoController::RVO_Init()
{
	RvoSim = new RVO::RVOSimulator();
	RvoSim->setTimeStep(RvoSettings.RVO_TimeStep);
	RvoSim->setAgentDefaults(RvoSettings.RVO_NeighborDist, RvoSettings.RVO_MaxNeighbors, RvoSettings.RVO_TimeHorizon,
	                         RvoSettings.RVO_TimeHorizonObst,
	                         RvoSettings.RVO_Radius / RvoSettings.RVO_LocationScale,
	                         RvoSettings.RVO_MaxSpeed / RvoSettings.RVO_LocationScale);
	if (RvoSettings.Obstacles.Num() > 0)
	{
		for (const auto& Obstacles : RvoSettings.Obstacles)
		{
			RVO_AddObstacles(Obstacles.Vertices);
		}
		RVO_BuildObstacles();
	}
}

void URvoController::RVO_Dormant(int32 RvoID)
{
	if (AgentDataMap.Contains(RvoID))
	{
		AgentDataMap[RvoID].CanAttack = false;
		RvoDormantMap.Add(RvoID, AgentDataMap[RvoID]);
		AgentDataMap.Remove(RvoID);
		// Some position far outside the simulation area
		const RVO::Vector2 FarAway(100000 + FMath::RandRange(0, 10000), 100000 + FMath::RandRange(0, 10000));
		RvoSim->setAgentMaxSpeed(RvoID, 0);
		RvoSim->setAgentMaxNeighbors(RvoID, 0);
		RvoSim->setAgentRadius(RvoID, 0);
		RvoSim->setAgentPosition(RvoID, FarAway);
		RvoSim->setAgentPrefVelocity(RvoID, RVO::Vector2(0, 0));
	}
}

void URvoController::RVO_WakeUpAgent(int32 RvoID, const FVector& WakeUpLocation)
{
	if (RvoDormantMap.Contains(RvoID))
	{
		FRvoAgentData RvoAgentData = RvoDormantMap.FindRef(RvoID);
		RvoAgentData.isDeath = false;
		RvoSim->setAgentMaxSpeed(RvoID, RvoAgentData.RvoSpeed / RvoSettings.RVO_LocationScale);
		RvoSim->setAgentMaxNeighbors(RvoID, RvoSettings.RVO_MaxNeighbors);
		RvoSim->setAgentRadius(RvoID, RvoAgentData.RvoRadius / RvoSettings.RVO_LocationScale);
		RvoSim->setAgentPosition(RvoID,
		                         RVO::Vector2(WakeUpLocation.X / RvoSettings.RVO_LocationScale,
		                                      WakeUpLocation.Y / RvoSettings.RVO_LocationScale));
		RvoDormantMap.Remove(RvoID);
		AgentDataMap.Add(RvoID, RvoAgentData);
	}
}


int32 URvoController::RVO_HostingAgent(FRvoAgentData RvoAgentData, const FVector& InstanceLocation)
{
	if (RvoSim)
	{
		const RVO::Vector2 Position = RVO::Vector2(InstanceLocation.X / RvoSettings.RVO_LocationScale,
		                                           InstanceLocation.Y / RvoSettings.RVO_LocationScale);
		const int32 ID = RvoSim->addAgent(Position, RvoSettings.RVO_NeighborDist, RvoSettings.RVO_MaxNeighbors,
		                                  RvoSettings.RVO_TimeHorizon, RvoSettings.RVO_TimeHorizonObst,
		                                  RvoAgentData.RvoRadius / RvoSettings.RVO_LocationScale,
		                                  RvoAgentData.RvoSpeed / RvoSettings.RVO_LocationScale);
		RvoAgentData.RvoID = ID; //储存ID
		AgentDataMap.Add(ID, RvoAgentData);
		return ID;
	}
	return -1;
}


void URvoController::RVO_Tick(float TickTime)
{
	RVO_UpdateAgentVelocity();
	RvoSim->doStep();
}

void URvoController::RVO_AddObstacles(TArray<FVector2D> PointList)
{
	std::vector<RVO::Vector2> obstacle;
	for (int index = 0; index < PointList.Num(); index++)
	{
		obstacle.push_back(RVO::Vector2(PointList[index].X / RvoSettings.RVO_LocationScale,
		                                PointList[index].Y / RvoSettings.RVO_LocationScale));
	}
	RvoSim->addObstacle(obstacle);
}

void URvoController::RVO_BuildObstacles()
{
	RvoSim->processObstacles();
}

FRvoAgentData URvoController::RVO_GetRvoAgentData(int32 RvoID)
{
	if (AgentDataMap.Contains(RvoID))
	{
		return AgentDataMap.FindRef(RvoID);
	}
	return {};
}

void URvoController::RVO_SetRvoAgentData(int32 RvoID, const FRvoAgentData& NewRvoAgentData)
{
	if (AgentDataMap.Contains(RvoID))
	{
		AgentDataMap[RvoID] = NewRvoAgentData;
	}
}

bool URvoController::RVO_IsReachedGoal(int32 RvoID)
{
	const FRvoAgentData AgentData = RVO_GetRvoAgentData(RvoID);
	const RVO::Vector2 Pos = RvoSim->getAgentPosition(RvoID);
	const FVector TargetLocation = FVector(AgentData.TargetLocation.X, AgentData.TargetLocation.Y, 0);
	const FVector Location = FVector(Pos.x() * RvoSettings.RVO_LocationScale, Pos.y() * RvoSettings.RVO_LocationScale,
	                                 0);
	const FVector vector = TargetLocation - Location;
	const RVO::Vector2 disVector = RVO::Vector2(vector.X, vector.Y);
	if (RVO::absSq(disVector) > RvoSim->getAgentRadius(RvoID) * RvoSim->getAgentRadius(RvoID))
	{
		return false;
	}
	return true;
}

bool URvoController::RVO_IsReachedAttackRange(int32 RvoID)
{
	const FRvoAgentData AgentData = RVO_GetRvoAgentData(RvoID);
	//const RVO::Vector2 Pos = RvoSim->getAgentPosition(RvoID);
	const FVector TargetLocation = FVector(AgentData.TargetLocation.X, AgentData.TargetLocation.Y, 0);
	const FVector Location = FVector(AgentData.CurrentLocation.X, AgentData.CurrentLocation.Y, 0);
	const FVector vector = TargetLocation - Location;
	const RVO::Vector2 disVector = RVO::Vector2(vector.X, vector.Y);
	const float Radius = AgentData.AttackRadius/* / RvoSettings.RVO_LocationScale + 1*/;
	if (RVO::absSq(disVector) > Radius * Radius)
	{
		return false;
	}
	return true;
}

FRvoAgentData URvoController::RVO_MakeRvoAgentData(FRvoAgentData RvoAgentData) const
{
	FRvoAgentData RvoAgentDataTemp = RvoAgentData;
	return RvoAgentDataTemp;
}

FVector2D URvoController::RVO_Get2DVelocity(int32 RvoID)
{
	if (RvoSim)
	{
		return FVector2D(RvoSim->getAgentVelocity(RvoID).x(), RvoSim->getAgentVelocity(RvoID).y());
	}
	return FVector2D::Zero();
}

void URvoController::RVO_DrawObstaclesDebugLine()
{
#if UE_EDITOR
	FlushPersistentDebugLines(GetWorld());
	for (const auto& Obstacles : RvoSettings.Obstacles)
	{
		for (int index = 0; index < Obstacles.Vertices.Num(); index++)
		{
			if (index == Obstacles.Vertices.Num() - 1)
			{
				DrawDebugLine(GetWorld(), FVector(Obstacles.Vertices[index].X, Obstacles.Vertices[index].Y, 100),
				              FVector(Obstacles.Vertices[0].X, Obstacles.Vertices[0].Y, 100),
				              FColor::Orange, false, 1000, SDPG_World, 10);
			}
			else
			{
				DrawDebugLine(GetWorld(), FVector(Obstacles.Vertices[index].X, Obstacles.Vertices[index].Y, 100),
				              FVector(Obstacles.Vertices[index + 1].X, Obstacles.Vertices[index + 1].Y, 100),
				              FColor::Orange, false, 1000, SDPG_World, 10);
			}
		}
	}
#endif
}

// Called every frame
void URvoController::TickComponent(float DeltaTime, ELevelTick TickType,
                                   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
