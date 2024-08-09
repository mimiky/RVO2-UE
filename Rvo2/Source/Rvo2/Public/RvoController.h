// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RvoData.h"
#include "Components/ActorComponent.h"
#include "Rvo2/Rvo2Library/RVOSimulator.h"
#include "RvoController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAgentDelegate, int32, RvoId);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent, DisplayName = "AC_RvoController"))
class RVO2_API URvoController : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	URvoController();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RVO Settings") 
	FRvoSettings RvoSettings;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void RVO_UpdateAgentVelocity();
public:	
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void RVO_Init();
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void RVO_Dormant(int32 RvoID);
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void RVO_WakeUpAgent(int32 RvoID, const FVector& WakeUpLocation);
	UFUNCTION(BlueprintCallable, Category = "RVO")
	int32 RVO_HostingAgent(FRvoAgentData RvoAgentData, const FVector& InstanceLocation);
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void RVO_Tick(float TickTime);
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void RVO_AddObstacles(TArray<FVector2D> PointList);
	UFUNCTION(BlueprintCallable, Category = "RVO")
	void RVO_BuildObstacles();
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	FRvoAgentData RVO_GetRvoAgentData(int32 RvoID);
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	void RVO_SetRvoAgentData(int32 RvoID,const FRvoAgentData& NewRvoAgentData);
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	bool RVO_IsReachedGoal(int32 RvoID);
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	bool RVO_IsReachedAttackRange(int32 RvoID);
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	FRvoAgentData RVO_MakeRvoAgentData(FRvoAgentData RvoAgentData) const;
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	FVector2D RVO_Get2DVelocity(int32 RvoID);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "RVO")
	void RVO_DrawObstaclesDebugLine();

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnAgentDelegate OnAgentEnterAttack;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnAgentDelegate OnAgentExitAttack;

protected:
	RVO::RVOSimulator* RvoSim;	
	UPROPERTY()
	TMap<int32,FRvoAgentData> AgentDataMap;
	UPROPERTY()
	TMap<int32,FRvoAgentData> RvoDormantMap;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
};
