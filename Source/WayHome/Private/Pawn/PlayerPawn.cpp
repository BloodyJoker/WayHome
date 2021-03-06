// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerPawn.h"
#include "WayHome.h"
#include "WayHomePlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine.h"
#include "Components/WidgetComponent.h"
#include "UserWidget.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/ChildActorComponent.h"
#include "ToolBase.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RotatePoint= CreateDefaultSubobject<USceneComponent>(TEXT("RotatePoint"));
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	WidgetInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionComp"));
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ForwardButton = CreateDefaultSubobject<UWidgetComponent>(TEXT("ForwardButton"));
	RightwardButton = CreateDefaultSubobject<UWidgetComponent>(TEXT("RightwardButton"));
	BackwardButton = CreateDefaultSubobject<UWidgetComponent>(TEXT("BackwardButton"));
	LeftwardButton = CreateDefaultSubobject<UWidgetComponent>(TEXT("LeftwardButton"));

	RotatePoint->SetupAttachment(RootComponent);
	CameraComp->SetupAttachment(RotatePoint);
	WidgetInteractionComp->SetupAttachment(CameraComp);
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetEnableGravity(false);
	ForwardButton->SetupAttachment(RootComponent);
	RightwardButton->SetupAttachment(RootComponent);
	BackwardButton->SetupAttachment(RootComponent);
	LeftwardButton->SetupAttachment(RootComponent);

	TraceLength_Forward = 150.0f;
	TraceLength_Rightward = 150.0f;
	TraceLength_Backward = 150.0f;
	TraceLength_Leftward = 150.0f;
	TraceLength_Base = 150.0f;
	ForwardButton->SetDrawSize(FVector2D(200, 100));
	RightwardButton->SetDrawSize(FVector2D(200, 100));
	BackwardButton->SetDrawSize(FVector2D(200, 100));
	LeftwardButton->SetDrawSize(FVector2D(200, 100));
	MoveSpeed = 20.0f;
	bMoving = false;
	bIsCameraCircling = false;
	PawnState = EPawnState::Vertical;
	PawnTrend = EPawnTrend::Default;
	OnMoveCompleteDelegate = FOnMoveCompleteDelegate::CreateUObject(this, &APlayerPawn::OnMovingComplete);
	OnCheckToolCompleteDelegate = FOnCheckToolCompleteDelegate::CreateUObject(this, &APlayerPawn::ResetButtonTransform);
	OnCameraCircleCompleteDelegate=FOnCameraCircleCompleteDelegate::CreateUObject(this, &APlayerPawn::OnCameraCircleComplete);
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	CheckMoveTool();
}

void APlayerPawn::CheckMoveTool()
{
	CheckBase();
	FHitResult Hit1, Hit2, Hit3, Hit4, Hit5, Hit6, Hit7, Hit8;
	FVector TraceStart = Basement->GetActorLocation();
	FCollisionQueryParams QueryParams;
	if (Basement)
	{
		FrontTool = nullptr;
		RightTool = nullptr;
		BackTool = nullptr;
		LeftTool = nullptr;
		if (PawnState == EPawnState::Horizontal)
		{
			if (PawnTrend == EPawnTrend::LeftAndRight)
			{
				//FrontTool
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*(100), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorForwardVector()*(100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit2.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							FrontTool = Hit2.GetActor();
							ForwardState = EPawnState::Vertical;
							ForwardTrend = EPawnTrend::Default;
							ForwardLocationOffset = FVector(0, 0, 200);
						}
					}
				}
				if(!GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*(100), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
				{
					if (GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart + GetActorForwardVector()*(100), TraceStart + GetActorForwardVector()*(100) + GetActorUpVector()*(-100), ToolChannel, QueryParams))
					{
						FrontTool = Hit2.GetActor();
						ForwardState = EPawnState::Vertical;
						ForwardTrend = EPawnTrend::Default;
						ForwardLocationOffset = FVector(0, 0, 200);
					}
				}

				//BackTool
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*(-100), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorForwardVector()*(-100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit2.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							BackTool = Hit2.GetActor();
							BackwardState = EPawnState::Vertical;
							BackwardTrend = EPawnTrend::Default;
							BackwardLocationOffset = FVector(0, 0, 200);
						}
					}
				}
				if(GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*(-100), ToolChannel, QueryParams=ResetQueryParams(QueryParams)))
				{
					if (GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart + GetActorForwardVector()*(-100), TraceStart + GetActorForwardVector()*(-100) + GetActorUpVector()*(-100), ToolChannel, QueryParams))
					{
						BackTool = Hit2.GetActor();
						BackwardState = EPawnState::Vertical;
						BackwardTrend = EPawnTrend::Default;
						BackwardLocationOffset = FVector(0, 0, 200);
					}
				}
				//RightTool
				QueryParams = ResetQueryParams(QueryParams);
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*(100), ToolChannel, QueryParams)
					&& GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart, TraceStart + GetActorForwardVector()*(-100), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit3, Basement->GetActorLocation(), Basement->GetActorLocation() + GetActorRightVector()*(100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit4, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorRightVector()*(100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorRightVector()*(100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit3.GetActor());
						QueryParams.AddIgnoredActor(Hit4.GetActor());
						QueryParams.AddIgnoredActor(Hit5.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit6, Hit3.GetActor()->GetActorLocation(), Hit3.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit7, Hit4.GetActor()->GetActorLocation(), Hit4.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit8, Hit5.GetActor()->GetActorLocation(), Hit5.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							RightTool = Hit3.GetActor();
							RightwardState = EPawnState::Horizontal;
							RightwardTrend = EPawnTrend::LeftAndRight;
							RightwardLocationOffset = FVector(0, 0, 100);
						}
					}
				}
				//LeftTool
				QueryParams = ResetQueryParams(QueryParams);
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*(100), ToolChannel, QueryParams)
					&& GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart, TraceStart + GetActorForwardVector()*(-100), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit3, Basement->GetActorLocation(), Basement->GetActorLocation() + GetActorRightVector()*(-100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit4, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorRightVector()*(-100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorRightVector()*(-100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit3.GetActor());
						QueryParams.AddIgnoredActor(Hit4.GetActor());
						QueryParams.AddIgnoredActor(Hit5.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit6, Hit3.GetActor()->GetActorLocation(), Hit3.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit7, Hit4.GetActor()->GetActorLocation(), Hit4.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit8, Hit5.GetActor()->GetActorLocation(), Hit5.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							LeftTool = Hit3.GetActor();
							LeftwardState = EPawnState::Horizontal;
							LeftwardTrend = EPawnTrend::LeftAndRight;
							RightwardLocationOffset = FVector(0, 0, 100);
						}
					}
				}
			}
			else if (PawnTrend == EPawnTrend::FrontAndBack)
			{
				//RightTool
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*(100), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorRightVector()*(100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit2.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							RightTool = Hit2.GetActor();
							RightwardState = EPawnState::Vertical;
							RightwardTrend = EPawnTrend::Default;
							RightwardLocationOffset = FVector(0, 0, 200);
						}
					}
				}
				if (!GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*(100), ToolChannel, QueryParams=ResetQueryParams(QueryParams)))
				{
					if (GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart + GetActorRightVector()*(100), TraceStart + GetActorRightVector()*(100) + GetActorUpVector()*(-100), ToolChannel, QueryParams))
					{
						RightTool = Hit2.GetActor();
						RightwardState = EPawnState::Vertical;
						RightwardTrend = EPawnTrend::Default;
						RightwardLocationOffset = FVector(0, 0, 200);
					}
				}

				//LeftTool
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*(-100), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorRightVector()*(-100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit2.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							LeftTool = Hit2.GetActor();
							LeftwardState = EPawnState::Vertical;
							LeftwardTrend = EPawnTrend::Default;
							LeftwardLocationOffset = FVector(0, 0, 200);
						}
					}
				}
				if(!GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*(-100), ToolChannel, QueryParams=ResetQueryParams(QueryParams)))
				{
					if (GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart + GetActorRightVector()*(-100), TraceStart + GetActorRightVector()*(-100) + GetActorUpVector()*(-100), ToolChannel, QueryParams))
					{
						LeftTool = Hit2.GetActor();
						LeftwardState = EPawnState::Vertical;
						LeftwardTrend = EPawnTrend::Default;
						LeftwardLocationOffset = FVector(0, 0, 200);
					}
				}

				//FrontTool
				QueryParams = ResetQueryParams(QueryParams);
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*(100), ToolChannel, QueryParams)
					&& GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart, TraceStart + GetActorRightVector()*(-100), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit3, Basement->GetActorLocation(), Basement->GetActorLocation() + GetActorForwardVector()*(100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit4, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorForwardVector()*(100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorForwardVector()*(100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit3.GetActor());
						QueryParams.AddIgnoredActor(Hit4.GetActor());
						QueryParams.AddIgnoredActor(Hit5.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit6, Hit3.GetActor()->GetActorLocation(), Hit3.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit7, Hit4.GetActor()->GetActorLocation(), Hit4.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit8, Hit5.GetActor()->GetActorLocation(), Hit5.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							FrontTool = Hit3.GetActor();
							ForwardState = EPawnState::Horizontal;
							ForwardTrend = EPawnTrend::FrontAndBack;
							ForwardLocationOffset = FVector(0, 0, 100);
						}
					}
				}
				//BackTool
				QueryParams = ResetQueryParams(QueryParams);
				if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*(100), ToolChannel, QueryParams)
					&& GetWorld()->LineTraceSingleByChannel(Hit2, TraceStart, TraceStart + GetActorRightVector()*(-100), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit1.GetActor());
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (GetWorld()->LineTraceSingleByChannel(Hit3, Basement->GetActorLocation(), Basement->GetActorLocation() + GetActorForwardVector()*(-100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit4, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorForwardVector()*(-100), ToolChannel, QueryParams)
						&& GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorForwardVector()*(-100), ToolChannel, QueryParams))
					{
						QueryParams.AddIgnoredActor(Hit3.GetActor());
						QueryParams.AddIgnoredActor(Hit4.GetActor());
						QueryParams.AddIgnoredActor(Hit5.GetActor());
						if (!GetWorld()->LineTraceSingleByChannel(Hit6, Hit3.GetActor()->GetActorLocation(), Hit3.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit7, Hit4.GetActor()->GetActorLocation(), Hit4.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams)
							&& !GetWorld()->LineTraceSingleByChannel(Hit8, Hit5.GetActor()->GetActorLocation(), Hit5.GetActor()->GetActorLocation() + GetActorUpVector()*(100), ToolChannel, QueryParams))
						{
							BackTool = Hit3.GetActor();
							BackwardState = EPawnState::Horizontal;
							BackwardTrend = EPawnTrend::FrontAndBack;
							BackwardLocationOffset = FVector(0, 0, 100);
						}
					}
				}
			}
		}
		else if (PawnState == EPawnState::Vertical)
		{
			//FrontTool
			if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*TraceLength_Forward*(1), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorForwardVector()*TraceLength_Forward*(1), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit4, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100 + GetActorForwardVector() * (100), ToolChannel, QueryParams))
					{
						ForwardTrend = EPawnTrend::LeftAndRight;
						ForwardState = EPawnState::Horizontal;
						FrontTool = Hit2.GetActor();
						ForwardLocationOffset = FVector(0, 0, 100);
					}

				}
			}
			if (GetWorld()->LineTraceSingleByChannel(Hit1, Basement->GetActorLocation() + GetActorUpVector() * 100, Basement->GetActorLocation() + GetActorUpVector() * 100 + (100)*GetActorForwardVector(), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + (100)*GetActorForwardVector(), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams))
					{
						ForwardTrend = EPawnTrend::LeftAndRight;
						ForwardState = EPawnState::Horizontal;
						FrontTool = Hit1.GetActor();
						ForwardLocationOffset = FVector(0, 0, 100);
					}
				}
			}
			//RightTool
			if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*TraceLength_Rightward*(1), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorRightVector()*TraceLength_Rightward*(1), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit4, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100 + GetActorRightVector() * (100), ToolChannel, QueryParams))
					{
						RightwardTrend = EPawnTrend::FrontAndBack;
						RightwardState = EPawnState::Horizontal;
						RightTool = Hit2.GetActor();
						RightwardLocationOffset = FVector(0, 0, 100);
					}

				}
			}
			if (GetWorld()->LineTraceSingleByChannel(Hit1, Basement->GetActorLocation() + GetActorUpVector() * 100, Basement->GetActorLocation() + GetActorUpVector() * 100 + (100)*GetActorForwardVector(), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + (100)*GetActorRightVector(), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams))
					{
						RightwardTrend = EPawnTrend::FrontAndBack;
						RightwardState = EPawnState::Horizontal;
						RightTool = Hit1.GetActor();
						RightwardLocationOffset = FVector(0, 0, 100);
					}
				}
			}
			//BackTool
			if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorForwardVector()*TraceLength_Backward*(-1), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorForwardVector()*TraceLength_Backward*(-1), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit4, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100 + GetActorForwardVector() * (-100), ToolChannel, QueryParams))
					{

						BackwardTrend = EPawnTrend::LeftAndRight;
						BackwardState = EPawnState::Horizontal;
						BackTool = Hit2.GetActor();
						BackwardLocationOffset = FVector(0, 0, 100);
					}
				}
			}
			if (GetWorld()->LineTraceSingleByChannel(Hit1, Basement->GetActorLocation() + GetActorUpVector() * 100, Basement->GetActorLocation() + GetActorUpVector() * 100 + (-100)*GetActorForwardVector(), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + (-100)*GetActorForwardVector(), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams))
					{
						BackwardTrend = EPawnTrend::LeftAndRight;
						BackwardState = EPawnState::Horizontal;
						BackTool = Hit1.GetActor();
						BackwardLocationOffset = FVector(0, 0, 100);
					}
				}
			}
			//LeftTool
			if (GetWorld()->LineTraceSingleByChannel(Hit1, TraceStart, TraceStart + GetActorRightVector()*TraceLength_Leftward*(-1), ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorRightVector()*TraceLength_Leftward*(-1), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit4, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit5, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100, Hit2.GetActor()->GetActorLocation() + GetActorUpVector() * 100 + GetActorRightVector() * (-100), ToolChannel, QueryParams))
					{
						LeftwardTrend = EPawnTrend::FrontAndBack;
						LeftwardState = EPawnState::Horizontal;
						LeftTool = Hit2.GetActor();
						LeftwardLocationOffset = FVector(0, 0, 100);
					}

				}
			}
			if (GetWorld()->LineTraceSingleByChannel(Hit1, Basement->GetActorLocation() + (-100)*GetActorRightVector(), Basement->GetActorLocation() + (-100)*GetActorRightVector() + GetActorUpVector() * 100, ToolChannel, QueryParams = ResetQueryParams(QueryParams)))
			{
				QueryParams.AddIgnoredActor(Hit1.GetActor());
				if (GetWorld()->LineTraceSingleByChannel(Hit2, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + (-100)*GetActorRightVector(), ToolChannel, QueryParams))
				{
					QueryParams.AddIgnoredActor(Hit2.GetActor());
					if (!GetWorld()->LineTraceSingleByChannel(Hit3, Hit1.GetActor()->GetActorLocation(), Hit1.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams)
						&& !GetWorld()->LineTraceSingleByChannel(Hit4, Hit2.GetActor()->GetActorLocation(), Hit2.GetActor()->GetActorLocation() + 100 * GetActorUpVector(), ToolChannel, QueryParams))
					{
						LeftwardTrend = EPawnTrend::FrontAndBack;
						LeftwardState = EPawnState::Horizontal;
						LeftTool = Hit1.GetActor();
						LeftwardLocationOffset = FVector(0, 0, 100);
					}
				}
			}
		}
	}
	OnCheckToolCompleteDelegate.ExecuteIfBound();
}

FCollisionQueryParams APlayerPawn::ResetQueryParams(FCollisionQueryParams QueryParams)
{
	FCollisionQueryParams NewQueryParams;
	NewQueryParams.AddIgnoredActor(this);
	NewQueryParams.AddIgnoredActor(Basement);
	NewQueryParams.bTraceComplex = true;
	return NewQueryParams;
}

void APlayerPawn::CheckBase()
{
	FHitResult Hit;
	FVector TraceStart = MeshComp->GetComponentLocation();
	FVector TraceEnd = GetActorUpVector()*(-1)*TraceLength_Base + MeshComp->GetComponentLocation();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ToolChannel, QueryParams))
	{
		Basement = Hit.GetActor();
		return;
	}
	Basement = nullptr;
	return;
}

void APlayerPawn::ResetButtonTransform()
{
	ForwardButton->SetWorldLocation(MeshComp->GetComponentLocation() + GetActorForwardVector()*(400), false);
	RightwardButton->SetWorldLocation(MeshComp->GetComponentLocation() + GetActorRightVector()*(400), false);
	BackwardButton->SetWorldLocation(MeshComp->GetComponentLocation() + GetActorForwardVector()*(-400), false);
	LeftwardButton->SetWorldLocation(MeshComp->GetComponentLocation() + GetActorRightVector()*(-400), false);

	ForwardButton->SetWorldRotation((CameraComp->GetComponentLocation() - ForwardButton->GetComponentLocation()).Rotation());
	RightwardButton->SetWorldRotation((CameraComp->GetComponentLocation() - RightwardButton->GetComponentLocation()).Rotation());
	BackwardButton->SetWorldRotation((CameraComp->GetComponentLocation() - BackwardButton->GetComponentLocation()).Rotation());
	LeftwardButton->SetWorldRotation((CameraComp->GetComponentLocation() - LeftwardButton->GetComponentLocation()).Rotation());
	if (FrontTool)
	{
		ForwardButton->SetVisibility(true);
	}
	else {
		ForwardButton->SetVisibility(false);
	}
	if (RightTool) {
		RightwardButton->SetVisibility(true);
	}
	else {
		RightwardButton->SetVisibility(false);
	}
	if (BackTool) {
		BackwardButton->SetVisibility(true);
	}
	else {
		BackwardButton->SetVisibility(false);
	}

	if (LeftTool) {
		LeftwardButton->SetVisibility(true);
	}
	else {
		LeftwardButton->SetVisibility(false);
	}
}

void APlayerPawn::Moving(float DeltaTime)
{
	FVector CurrentLocation = MeshComp->GetComponentLocation();
	if (CurrentLocation == EndLocation)
	{
		OnMoveCompleteDelegate.ExecuteIfBound();
		return;
	}
	MeshComp->SetWorldLocation(CurrentLocation + (EndLocation - StartLocation) / MoveSpeed);
	MeshComp->AddWorldRotation(RotationOffset * (90 / MoveSpeed));
}

void APlayerPawn::CameraCircling(float DeltaTime)
{
	if (FMath::Abs(RotatePoint->GetComponentRotation().Yaw-EndCirclePointRotation.Yaw)<=2.0f) { 
		OnCameraCircleCompleteDelegate.ExecuteIfBound();
		return;
	}
	RotatePoint->AddLocalRotation(CircleOffset*CameraChangeSpeed);
	ResetButtonTransform();
}

void APlayerPawn::OnMovingComplete()
{
	bMoving = false;
	CheckMoveTool();
}

void APlayerPawn::OnCameraCircleComplete()
{
	bIsCameraCircling = false;
	RotatePoint->SetWorldRotation(EndCirclePointRotation);
}

FVector APlayerPawn::MathFun1()
{
	return FVector();
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bMoving) Moving(DeltaTime);
	if (bIsCameraCircling) CameraCircling(DeltaTime);
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlayerPawn::StartMove(AActor * Tool)
{
	if (Tool == FrontTool)
	{
		RotationOffset = FRotator(-1, 0, 0);
		LocationOffset = ForwardLocationOffset;
		PawnState = ForwardState;
		PawnTrend = ForwardTrend;
	}
	else if (Tool == RightTool)
	{
		RotationOffset = FRotator(0, 0, 1);
		LocationOffset = RightwardLocationOffset;
		PawnState = RightwardState;
		PawnTrend = RightwardTrend;
	}
	else if (Tool == BackTool)
	{
		RotationOffset = FRotator(1, 0, 0);
		LocationOffset = BackwardLocationOffset;
		PawnState = BackwardState;
		PawnTrend = BackwardTrend;
	}
	else if (Tool == LeftTool)
	{
		RotationOffset = FRotator(0, 0, -1);
		LocationOffset = LeftwardLocationOffset;
		PawnState = LeftwardState;
		PawnTrend = LeftwardTrend;
	}
	EndRotation = MeshComp->GetComponentRotation() + RotationOffset * 90;
	StartLocation = MeshComp->GetComponentLocation();
	EndLocation = Tool->GetActorLocation() + LocationOffset;
	bMoving = true;
	ForwardButton->SetVisibility(false);
	RightwardButton->SetVisibility(false);
	BackwardButton->SetVisibility(false);
	LeftwardButton->SetVisibility(false);

}

void APlayerPawn::StartCircleCamera(bool bIsNext)
{
	if (bIsNext) CircleOffset = FRotator(0, -1, 0);
	else CircleOffset = FRotator(0, 1, 0);
	StartCirclePointRotation = RotatePoint->GetComponentRotation();
	EndCirclePointRotation = RotatePoint->GetComponentRotation() + CircleOffset * 90;
	if (FMath::Abs((int32)EndCirclePointRotation.Yaw) / 180)
	{
		if (EndCirclePointRotation.Yaw > 180) EndCirclePointRotation.Yaw = EndCirclePointRotation.Yaw - 360.0f;
		if (EndCirclePointRotation.Yaw < -180)EndCirclePointRotation.Yaw = EndCirclePointRotation.Yaw + 360.0f;
	}
	bIsCameraCircling = true;
}



