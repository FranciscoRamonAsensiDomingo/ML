// Fill out your copyright notice in the Description page of Project Settings.


#include "../public/MyCube.h"

// Sets default values
AMyCube::AMyCube()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	OurVisibleActor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Our mesh"));
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Our collision"));
	Component = CreateDefaultSubobject<UBoxComponent>(TEXT("Our box"));
	RootComponent = Component;

	OurCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	OurCameraSpringArm->SetupAttachment(RootComponent);
	OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
	OurCameraSpringArm->TargetArmLength = 400.f;
	OurCameraSpringArm->bEnableCameraLag = true;
	OurCameraSpringArm->CameraLagSpeed = 0.0f;

	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	OurCamera->SetupAttachment(OurCameraSpringArm, USpringArmComponent::SocketName);

	OurVisibleActor->SetupAttachment(RootComponent);

	BoxComponent->SetupAttachment(RootComponent);

	BoxComponent->SetNotifyRigidBodyCollision(true);
	BoxComponent->BodyInstance.SetCollisionProfileName("Cube");
	BoxComponent->OnComponentHit.AddDynamic(this, &AMyCube::OnCompHit);
	BoxComponent->SetRelativeScale3D(FVector(1.75, 1.5, 0.5));
	BoxComponent->SetSimulatePhysics(false);

	CollisionParams.AddIgnoredActor(this);

	Component->SetSimulatePhysics(true);
	Component->BodyInstance.SetCollisionProfileName("Cube");
	OurVisibleActor->BodyInstance.SetCollisionProfileName("NoCollision");
	

	NNproportion = 4.f / MaxDistance;


}

// Called when the game starts or when spawned
void AMyCube::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMyCube::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	deltatime = DeltaTime;

	if (ActualVelocity < VelocityX) {
		time += DeltaTime;
		ActualVelocity = time * Aceleration;
	}

	if (this->isPossessed) {
		result[0] = RotationPlayer;// *SmoothInput + (1.f - SmoothInput)*result[!actual][0];
	}
	else {
		result = nnetwork.forward(Input);
	}


	UpdateStick();
	UpdateRotation();
	UpdateLocation();
	UpdateCameraRotation();
	UpdateStickRotation();
	UpdateStickLength();

}

// Called to bind functionality to input
void AMyCube::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("CameraPitch", this, &AMyCube::Input_CameraPitch);
	InputComponent->BindAxis("CameraYaw", this, &AMyCube::Input_CameraYaw);
	InputComponent->BindAxis("StickPitch", this, &AMyCube::Input_StickPitch);
	InputComponent->BindAxis("Turn", this, &AMyCube::Input_StickYaw);
	InputComponent->BindAxis("Approach", this, &AMyCube::Input_Approach);

}

void AMyCube::OnCompHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (GEngine) {
			//GEngine->AddOnScreenDebugMessage(-1,1.f, FColor::Green, FString::Printf(TEXT("I Just hit:%s"), *OtherActor->GetName()));
			//this->Destroy();
			hit = true;
			this->SetActorTickEnabled(false);
		}
	}
}

void AMyCube::ResetMovement(FTransform transform, int initTarget)
{
	this->SetActorTransform(transform);
	this->hit = false;
	this->lastTarget = initTarget;
	this->laps = 0;
	this->SetActorTickEnabled(true);
}

void AMyCube::Change()
{
	for (auto&a : nnetwork.NNetwork) {
		for (auto&b : a) {
			for (auto&c : b) {
				c += (FMath::FRand() * 2 - 1)*0.4;
			}
		}
	}
}

void AMyCube::InitNet(TArray<int> topology)
{
	result.Init(0, topology.Last());
	nnetwork.Init(topology);
	Input.Init(0, topology[0]);
}

void AMyCube::StartPossessing()
{
	GetController()->Possess(this);
}

void AMyCube::StopPossessing()
{
	GetController()->UnPossess();
}

void AMyCube::UpdateStick()
{
	if (Input.Num() == 0)
		return;
	FHitResult OutHit;
	FVector start = this->GetActorLocation();
	FRotator actorRot(this->GetActorRotation());
	actorRot.Yaw -= amplitude * 0.5;

	float stickrotation = amplitude / (StickNumber - 1);
	for (int i = 0; i < StickNumber; ++i) {
		FVector end(actorRot.Vector() * 2000 + start);
		if (drawLine)
			DrawDebugLine(GetWorld(), start, end, FColor::Green, false, deltatime + 0.01, 0, 1);
		bool isHit = GetWorld()->LineTraceSingleByObjectType(OutHit, start, end, ECC_WorldStatic, CollisionParams);
		if (isHit && OutHit.bBlockingHit) {
			if (GEngine) {
				float distance = (OutHit.ImpactPoint - start).Size();
				Input[i] = distance;
				//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Distance: %f"), distance));
				//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Actor: %s"), *OutHit.GetActor()->GetName()));
			}
		}
		else {
			Input[i] = MaxDistance;
		}
		if (!isPossessed)
			Input[i] *= NNproportion;
		actorRot.Yaw += stickrotation;
		//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::Printf(TEXT("result: %f"),  Input[i]));
	}

}

void AMyCube::UpdateLocation()
{
	SetActorLocation(GetActorForwardVector()* (deltatime*ActualVelocity) + GetActorLocation());
}

void AMyCube::UpdateRotation()
{
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw + RotationVelocityPawn * deltatime*(result[0]), 0.f));
}

void AMyCube::UpdateCameraRotation()
{
	FRotator CameraRotationTemp = OurCamera->GetComponentRotation();
	CameraRotation.Pitch += CameraInput.Y*camera_velocity;
	CameraRotation.Yaw += CameraInput.X*camera_velocity;
	OurCamera->SetWorldRotation(CameraRotation);
}

void AMyCube::UpdateStickRotation()
{
	FRotator StickRotationTemp = OurCameraSpringArm->GetComponentRotation();
	StickRotation.Pitch += StickInput.Y*stick_velocity;
	StickRotation.Yaw += StickInput.X*stick_velocity;
	OurCameraSpringArm->SetWorldRotation(StickRotation);
}

void AMyCube::UpdateStickLength()
{
	OurCameraSpringArm->TargetArmLength += approach_velocity * StickDistance;
}

void AMyCube::Input_Approach(float AxisValue)
{
	StickDistance = FMath::Clamp<float>(AxisValue, -1.0f, 1.f);
}

void AMyCube::Input_CameraPitch(float AxisValue)
{
	CameraInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AMyCube::Input_CameraYaw(float AxisValue)
{
	CameraInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AMyCube::Input_StickYaw(float AxisValue)
{
	RotationPlayer = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void AMyCube::Input_StickPitch(float AxisValue)
{
	StickInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}
