// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPawn.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

// Sets default values
AVRPawn::AVRPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//########################################## V R   C O N T R O L L E R ################################################
	StereoOffset = 6.5f;

	//scene & camera
	VRTrackingCenter = CreateDefaultSubobject<USceneComponent>(TEXT("VRTrackingCenter"));
	RightEye = CreateDefaultSubobject<UCameraComponent>(TEXT("RightEye"));
	LeftEye = CreateDefaultSubobject<UCameraComponent>(TEXT("LeftEye"));
	PlayerBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	//outputText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("outputText"));

	//Left Hand VR
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftContoller"));
	LeftCone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftCone"));

	//Right Hand VR
	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightContoller"));
	RightCone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightCone"));

	//Headset hierarchy
	RightEye->SetupAttachment(VRTrackingCenter);
	LeftEye->SetupAttachment(VRTrackingCenter);
	PlayerBody->SetupAttachment(VRTrackingCenter);//PlayerBody is child of VRTracking Center
	//outputText->SetupAttachment(RightEye);//outputText is child of RightEye etc.

	//Left hand hierarchy
	LeftController->SetupAttachment(VRTrackingCenter);
	LeftCone->SetupAttachment(LeftController);

	//Right hand hierarchy
	RightController->SetupAttachment(VRTrackingCenter);
	RightCone->SetupAttachment(RightController);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> cone(TEXT("StaticMesh'/Engine/BasicShapes/Cone.Cone'"));
	this->LeftCone->SetStaticMesh(cone.Object);
	this->RightCone->SetStaticMesh(cone.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> body(TEXT("StaticMesh'/Game/MobileStarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule'"));
	this->PlayerBody->SetStaticMesh(body.Object);

	//this is how to initialize and load a material
	static ConstructorHelpers::FObjectFinder<UMaterial> gray(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	LeftCone->SetMaterial(0, gray.Object);
	RightCone->SetMaterial(0, gray.Object);
	PlayerBody->SetMaterial(0, gray.Object);

	//make the right and left hands work with VR Controllers
	RightController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	LeftController->MotionSource = FXRMotionControllerBase::LeftHandSourceId;

	//Transformations in Unreal Location, Rotation, Scale
	RightEye->SetRelativeLocation(FVector(50, 32, SeatedHeightOffset));
	LeftEye->SetRelativeLocation(FVector(50, -32, SeatedHeightOffset));
	PlayerBody->SetRelativeLocation(FVector(0, 0, -20));
	PlayerBody->SetRelativeScale3D(FVector(2, 2, SeatedHeightOffset / 100));

	LeftCone->SetRelativeRotation(FRotator(-90, 90, -90));
	LeftCone->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));

	RightCone->SetRelativeRotation(FRotator(-90, 90, -90));
	RightCone->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));

	//set root component, default is the first one to be called from the code if not specified
	this->RootComponent = VRTrackingCenter;

	//make this the default controller (hardcoded)
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	//code for text
	/*static ConstructorHelpers::FObjectFinder<UMaterial> unlitText(TEXT("Material'/Engine/EngineMaterials/DefaultTextMaterialTranslucent.DefaultTextMaterialTranslucent'"));
	outputText->SetMaterial(0, unlitText.Object);
	outputText->SetTextRenderColor(FColor::Red);
	outputText->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
	outputText->VerticalAlignment = EVerticalTextAligment::EVRTA_TextCenter;
	outputText->SetRelativeLocation(FVector(150, 0, 0));
	outputText->SetRelativeRotation(FRotator(0, 180.0f, 0));*/

	//physics
	PlayerBody->SetSimulatePhysics(true);
	PlayerBody->SetEnableGravity(true);
	PlayerBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlayerBody->SetCollisionProfileName(TEXT("PhysicsActor"));
	PlayerBody->SetMassOverrideInKg(NAME_None, 50.0f, true);
	PlayerBody->BodyInstance.bLockXRotation = true;
	PlayerBody->BodyInstance.bLockYRotation = true;
	PlayerBody->BodyInstance.bLockZRotation = false;

	RightCone->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LeftCone->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//collisions
	LeftCone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	RightCone->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	//recasting
	navmesh = dynamic_cast<ARecastNavMesh*>(UGameplayStatics::GetActorOfClass(GetWorld(), ARecastNavMesh::StaticClass()));

	//##################################################### C A P T U R I N G #####################################################
	// Create Color Scene Capture
	//SceneCaptureCenter = CreateDefaultSubobject<USceneComponent>(TEXT("SceneCaptureCenter"));
	// Normalization

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(TEXT("Material'/Game/M_NormalizedDepth.M_NormalizedDepth'"));
	if (MatFinder.Succeeded())
	{
		DepthNormMaterial = MatFinder.Object;
	}

	//L
	SceneCaptureColorLeft = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureLeft"));
	SceneCaptureColorLeft->SetRelativeLocation(FVector(0, -StereoOffset / 2, 0));
	//R
	SceneCaptureColorRight = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureRight"));
	SceneCaptureColorRight->SetRelativeLocation(FVector(0, StereoOffset / 2, 0));

	// Create Depth Scene Capture
	//L
	SceneCaptureDepthLeft = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureDepthLeft"));
	SceneCaptureDepthLeft->SetRelativeLocation(FVector(0, -StereoOffset / 2, 0));
	//R
	SceneCaptureDepthRight = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureDepthRight"));
	SceneCaptureDepthRight->SetRelativeLocation(FVector(0, StereoOffset / 2, 0));

	//Hierarchy
	SceneCaptureColorLeft->SetupAttachment(LeftEye);
	SceneCaptureDepthLeft->SetupAttachment(LeftEye);
	SceneCaptureColorRight->SetupAttachment(RightEye);
	SceneCaptureDepthRight->SetupAttachment(RightEye);

	//Color Render Target
	//L
	/*static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> ColorAssetL(TEXT("TextureRenderTarget2D'/Game/DataAssets/RTs/RT_ColorL.RT_ColorL'"));
	RT_Color_L = DuplicateObject<UTextureRenderTarget2D>(ColorAssetL.Object, this);
	//R
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> ColorAssetR(TEXT("TextureRenderTarget2D'/Game/DataAssets/RTs/RT_ColorR.RT_ColorR'"));
	RT_Color_R = DuplicateObject<UTextureRenderTarget2D>(ColorAssetR.Object, this);

	//Depth Render Target
	//L
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> DepthAssetL(TEXT("TextureRenderTarget2D'/Game/DataAssets/RTs/RT_DepthL.RT_DepthL'"));
	RT_Depth_L = DuplicateObject<UTextureRenderTarget2D>(DepthAssetL.Object, this);

	//R
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> DepthAssetR(TEXT("TextureRenderTarget2D'/Game/DataAssets/RTs/RT_DepthR.RT_DepthR'"));
	RT_Depth_R = DuplicateObject<UTextureRenderTarget2D>(DepthAssetR.Object, this);

	// Assign Color Capture
	//L
	SceneCaptureColorLeft->TextureTarget = RT_Color_L;
	SceneCaptureColorLeft->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	//R
	SceneCaptureColorRight->TextureTarget = RT_Color_R;
	SceneCaptureColorRight->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;

	// Assign Depth Capture
	//L
	SceneCaptureDepthLeft->TextureTarget = RT_Depth_L;
	SceneCaptureDepthLeft->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	//R
	SceneCaptureDepthRight->TextureTarget = RT_Depth_R;
	SceneCaptureDepthRight->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;*/

}

// Called when the game starts or when spawned
void AVRPawn::BeginPlay()
{
	Super::BeginPlay();

	//UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
	// Query the current tracking origin and set VR mode accordingly
	EHMDTrackingOrigin::Type CurrentOrigin = UHeadMountedDisplayFunctionLibrary::GetTrackingOrigin();

	if (CurrentOrigin == EHMDTrackingOrigin::Floor)
	{
		SetVRMode(false); // Standing mode
		GLog->Log("VR Initialized with Floor-level Tracking: Standing Mode");
	}
	else if (CurrentOrigin == EHMDTrackingOrigin::Eye)
	{
		SetVRMode(true); // Seated mode
		GLog->Log("VR Initialized with Eye-level Tracking: Seated Mode");
	}
	else
	{
		GLog->Log("Unknown tracking origin. Defaulting to Seated Mode.");
		SetVRMode(true);
	}

	if (DepthNormMaterial)
	{
		//left eye normalization
		SceneCaptureDepthLeft->PostProcessSettings.AddBlendable(DepthNormMaterial, 1.0f);

		//right eye normalization
		SceneCaptureDepthRight->PostProcessSettings.AddBlendable(DepthNormMaterial, 1.0f);
	}
}



// Called every frame
void AVRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector VRLocation = VRTrackingCenter->GetComponentLocation();
	PlayerBody->SetWorldLocation(FVector(VRLocation.X, VRLocation.Y, PlayerBody->GetComponentLocation().Z));

	//detect ground
	FHitResult HitResult;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 1000);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility)) {
		FVector NewLocation = HitResult.Location;
		NewLocation.Z += 50.0f;
		SetActorLocation(NewLocation);
	}

	if (DepthNormMaterial)
	{
		//left eye normalization
		SceneCaptureDepthLeft->PostProcessSettings.AddBlendable(DepthNormMaterial, 1.0f);

		//right eye normalization
		SceneCaptureDepthRight->PostProcessSettings.AddBlendable(DepthNormMaterial, 1.0f);
	}

}

// Called to bind functionality to input
void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AVRPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVRPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AVRPawn::Turn);

	PlayerInputComponent->BindAction("Capture", EInputEvent::IE_Pressed, this, &AVRPawn::CaptureFrame);

}

void AVRPawn::SetVRMode(bool bIsSeated)
{
	if (bIsSeated)
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		RightEye->SetRelativeLocation(FVector(50, 32, SeatedHeightOffset));
		LeftEye->SetRelativeLocation(FVector(50, -32, SeatedHeightOffset));
	}
	else
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
		RightEye->SetRelativeLocation(FVector(50, 32, 0)); // Reset for standing mode
		LeftEye->SetRelativeLocation(FVector(50, -32, 0));
	}
}

/*void AVRPawn::Start() {

}*/


void AVRPawn::MoveForward(float AxisValue) {
	//setting a movement vector using world coordinates
	this->AddActorWorldOffset(this->GetActorForwardVector() * AxisValue * 10);
}

void AVRPawn::MoveRight(float AxisValue) {
	this->AddActorWorldOffset(this->GetActorRightVector() * AxisValue * 10);
}

void AVRPawn::Turn(float AxisValue) {
	//FRotator(pitch,yaw,roll) in world coordinates
	this->AddActorWorldRotation(FRotator(0, AxisValue * 25, 0));
}

/*void AVRPawn::LookUp(float AxisValue) {
	//rotate in local coordinates
	this->AddActorLocalRotation(FRotator(-AxisValue, 0, 0));
}*/

void AVRPawn::TPThere() {
	GLog->Log("TP");
	this->outputText->SetText("Teleporting");

	//raycasting
	FHitResult hit;

	TArray<AActor*> ignored;
	ignored.Add(this);
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), LeftCone->GetComponentLocation(),
		LeftController->GetComponentLocation() + (LeftCone->GetUpVector() * 10000.0f), UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_WorldStatic),
		false, ignored, EDrawDebugTrace::Persistent, hit, true)) {
		if (hit.GetActor()) {
			this->outputText->SetText(hit.GetActor()->GetName());
		}

		//this->SetActorLocation(hit.ImpactPoint);
		FNavLocation outnav;
		if (navmesh->ProjectPoint(hit.ImpactPoint, outnav, FVector(1000, 1000, 1000))) {
			SetActorLocation(outnav.Location);
		}
	}
}

void AVRPawn::CaptureFrame()
{
	//FORCE RENDER (FIX FOR DEPTH MAPS?)
	SceneCaptureColorLeft->CaptureScene();
	SceneCaptureColorRight->CaptureScene();
	SceneCaptureDepthLeft->CaptureScene();
	SceneCaptureDepthRight->CaptureScene();

	int FrameNumber = GFrameNumber;

	FString FileNameColorL = FString::Printf(TEXT("Color/Left/Scene_Color_L%04d.exr"), FrameNumber);
	FString FileNameColorR = FString::Printf(TEXT("Color/Right/Scene_Color_R%04d.exr"), FrameNumber);
	FString FileNameDepthL = FString::Printf(TEXT("Depth/Left/Scene_Depth_L%04d.exr"), FrameNumber);
	FString FileNameDepthR = FString::Printf(TEXT("Depth/Right/Scene_Depth_R%04d.exr"), FrameNumber);

	SaveRenderTarget(RT_Color_L, FileNameColorL);
	SaveRenderTarget(RT_Color_R, FileNameColorR);
	SaveRenderTarget(RT_Depth_L, FileNameDepthL);
	SaveRenderTarget(RT_Depth_R, FileNameDepthR);

	UE_LOG(LogTemp, Warning, TEXT("Captured frame %d!"), FrameNumber);
}

void AVRPawn::SaveRenderTarget(UTextureRenderTarget2D* RT, FString SubPathAndFileName) {
	if (!RT)
	{
		UE_LOG(LogTemp, Error, TEXT("Render Target is NULL!"));
		return;
	}

	FString Directory = FPaths::ProjectSavedDir() + TEXT("Screenshots/Scene0") + FPaths::GetPath(SubPathAndFileName) + TEXT("/");

	if (!IFileManager::Get().DirectoryExists(*Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	FString FileName = FPaths::GetCleanFilename(SubPathAndFileName);
	UE_LOG(LogTemp, Warning, TEXT("Saving screenshot to: %s%s"), *Directory, *FileName);
	UKismetRenderingLibrary::ExportRenderTarget(this, RT, Directory, FileName);
}