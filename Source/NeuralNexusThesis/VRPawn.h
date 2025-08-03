// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "MotionControllerComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "IXRTrackingSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SceneCapture2D.h" 
#include "Engine/TextureRenderTarget2D.h" 
#include "Kismet/KismetRenderingLibrary.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "XRMotionControllerBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavMesh/RecastNavMesh.h"
#include "VRPawn.generated.h"


UCLASS()
class NEURALNEXUSTHESIS_API AVRPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVRPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//initialize components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		USceneComponent* VRTrackingCenter; //scene obj

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		UCameraComponent* RightEye; //camera

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		UCameraComponent* LeftEye; //camera

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UTextRenderComponent* outputText; //text renderer

	//motion controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UMotionControllerComponent* LeftController;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UMotionControllerComponent* RightController;

	//meshers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UStaticMeshComponent* LeftCone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UStaticMeshComponent* RightCone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UStaticMeshComponent* PlayerBody;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Settings")
		float SeatedHeightOffset = 120.0f;
	UPROPERTY(EditAnywhere, meta = (BindWiget))
		UUserWidget* PauseMenuWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		ARecastNavMesh* navmesh;

	//for ScenecaptureComponent to capture the frames
	UPROPERTY(EditAnywhere, Category = "Capture")
		USceneCaptureComponent2D* SceneCaptureColorLeft;
	UPROPERTY(EditAnywhere, Category = "Capture")
		USceneCaptureComponent2D* SceneCaptureColorRight;
	UPROPERTY(EditAnywhere, Category = "Capture")

		USceneCaptureComponent2D* SceneCaptureDepthLeft;
	UPROPERTY(EditAnywhere, Category = "Capture")
		USceneCaptureComponent2D* SceneCaptureDepthRight;

	//Normalization of SceneDepth
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capture")
		UMaterialInterface* DepthNormMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UStaticMeshComponent* SceneDepthFilterPlane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = __hide)
		UStaticMeshComponent* SceneDepthCS;

	//Render Targets with info for the frames
	UPROPERTY(EditAnywhere, Category = "Capture")
		UTextureRenderTarget2D* RT_Color_L;
	UPROPERTY(EditAnywhere, Category = "Capture")
		UTextureRenderTarget2D* RT_Color_R;

	UPROPERTY(EditAnywhere, Category = "Capture")
		UTextureRenderTarget2D* RT_Depth_L;
	UPROPERTY(EditAnywhere, Category = "Capture")
		UTextureRenderTarget2D* RT_Depth_R;

	float StereoOffset;
	TArray<FVector> Waypoints;
	int32 CurrentWaypointIndex = 0;
	//for the fixed path
	bool bIsFollowingPath = false;

	float MoveSpeed;
	float TimeSinceLastCaptured;
	float CaptureIntvl;

	void SaveRenderTarget(UTextureRenderTarget2D* RT, FString FileName);

	//void Start();
	UFUNCTION()
		void CaptureFrame();

	void MoveForward(float AxisValue);

	void MoveRight(float AxisValue);

	void Turn(float AxisValue);
	
	void StartPathCapture();

	UFUNCTION(BlueprintCallable)
		void TPThere();

	UFUNCTION(BlueprintCallable)
		void SetVRMode(bool bIsSeated);

};
