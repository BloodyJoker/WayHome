// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

class UStaticMeshComponent;
class UCameraComponent;
class UUserWidget;
class UWidgetInteractionComponent;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EPawnState : uint8
{
	Horizontal,   //水平
	Vertical      //垂直
};
UENUM(BlueprintType)
enum class EPawnTrend : uint8
{
	Default,       //默认
	LeftAndRight,  //左右偏向
	FrontAndBack   //前后偏向
};

DECLARE_DELEGATE(FOnMoveCompleteDelegate);
DECLARE_DELEGATE(FOnCheckToolCompleteDelegate);

UCLASS()
class WAYHOME_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LineTrace")
		float TraceLength_Forward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LineTrace")
		float TraceLength_Rightward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LineTrace")
		float TraceLength_Backward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LineTrace")
		float TraceLength_Leftward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LineTrace")
		float TraceLength_Base;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UWidgetInteractionComponent* WidgetInteractionComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetComponent* ForwardButton;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetComponent* RightwardButton;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetComponent* BackwardButton;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UWidgetComponent* LeftwardButton;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Move")
		float MoveSpeed;
	UPROPERTY(BlueprintReadOnly, Category = "Tools")
		AActor* FrontTool;
	UPROPERTY(BlueprintReadOnly, Category = "Tools")
		AActor* RightTool;
	UPROPERTY(BlueprintReadOnly, Category = "Tools")
		AActor* BackTool;
	UPROPERTY(BlueprintReadOnly, Category = "Tools")
		AActor* LeftTool;//底座四周的可交互物体
	UPROPERTY(BlueprintReadOnly, Category = "Tools")
		AActor* Basement;//底座

	FRotator EndRotation;
	FVector StartLocation;
	FVector EndLocation;
	bool bMoving;
	FRotator RotationOffset;
	FVector LocationOffset;
	float ForwardRadius;
	float RightwardRadius;
	float BackwardRadius;
	float LeftwardRadius;
	FVector ForwardLocationOffset;
	FVector RightwardLocationOffset;
	FVector BackwardLocationOffset;
	FVector LeftwardLocationOffset;
	EPawnState ForwardState;
	EPawnTrend ForwardTrend;
	EPawnState RightwardState;
	EPawnTrend RightwardTrend;
	EPawnState BackwardState;
	EPawnTrend BackwardTrend;
	EPawnState LeftwardState;
	EPawnTrend LeftwardTrend;

	FOnMoveCompleteDelegate OnMoveCompleteDelegate;
	FOnCheckToolCompleteDelegate OnCheckToolCompleteDelegate;
	UPROPERTY(EditDefaultsOnly, Category = "PawnInfo")
		EPawnState PawnState;
	UPROPERTY(EditDefaultsOnly, Category = "PawnInfo")
		EPawnTrend PawnTrend;
	void CheckMoveTool();
	FCollisionQueryParams ResetQueryParams(FCollisionQueryParams QueryParams);
	void CheckBase();
	void ResetButtonTransform();
	void Moving(float DeltaTime);
	void OnMovingComplete();



	//位移函数
	FVector MathFun1();
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION(BlueprintCallable, Category = "Move")
		void StartMove(AActor* Tool);

};
