// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RvoData.generated.h"

USTRUCT(BlueprintType)
struct FObstacleVertices
{
	GENERATED_USTRUCT_BODY()
public:
	//区域，按照逆时针添加
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	TArray<FVector2D> Vertices;
};
USTRUCT(BlueprintType)
struct FRvoSettings
{
	GENERATED_USTRUCT_BODY()
	FRvoSettings()
	{
	}

public:
	/**
	 *  时间缩放
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO")
	float RVO_TimeStep = 0.25f;
	/**
	 * 位置缩放
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	float RVO_LocationScale = 20.0f;
	/**
	 * 导航中考虑新代理到其他代理的默认最大距离
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	float RVO_NeighborDist = 15.0f;
	/**
	 * 导航中考虑新代理的其他代理的默认最大数量
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	int64 RVO_MaxNeighbors = 10;
	/**
	 * 默认最小时间量，通过该最小时间量，模拟计算出的新代理相对于其他代理的速度是安全的
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	float RVO_TimeHorizon = 5.0f;
	/**
	 * 默认最小时间量，通过该最小时间量，模拟计算出的新代理的速度相对于障碍物是安全的
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	float RVO_TimeHorizonObst = 5.0f;
	/**
	 * 默认新代理的半径
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	float RVO_Radius = 50.0f;
	/**
	 * 默认新代理的速度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RVO") 
	float RVO_MaxSpeed = 30.0f;
	/**
	 *  Tick的时间
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float TickDeltaTime = 0.0302f;
	//障碍
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	TArray<FObstacleVertices> Obstacles;
	
};

USTRUCT(BlueprintType)
struct FRvoAgentData
{
	GENERATED_USTRUCT_BODY()
	FRvoAgentData()
	{
	}

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RVO")
	int32 RvoID = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float RvoSpeed = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float RvoRadius = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	float AttackRadius = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	bool isDeath = false;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO")
	FVector TargetLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RVO")
	FVector CurrentLocation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RVO")
	FRotator CurrentRotator;	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RVO")
	bool CanAttack = false;
};
