// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestWeaponProjectileCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyGameInstance.h"
#include "TestWeaponProjectile.h"
#include "Net/UnrealNetWork.h"


//////////////////////////////////////////////////////////////////////////
// ATestWeaponProjectileCharacter

ATestWeaponProjectileCharacter::ATestWeaponProjectileCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	/*if (HasAuthority()) {
		this->SetReplicates(true);
	}*/
	//this->SetReplicates(true);
	bReplicates = true;
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->SetupAttachment(GetMesh(), "headSocket");
	CameraBoom->TargetArmLength = 200.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	ShootArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootArrow->SetupAttachment(FollowCamera);

	ArrowLeft = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowLeft"));
	ArrowLeft->SetupAttachment(GetMesh());
	ArrowLeft->SetRelativeLocation(FVector((GetMesh()->GetComponentLocation()).X + 150, (GetMesh()->GetComponentLocation()).Y, (GetMesh()->GetComponentLocation()).Z+100));
	ArrowLeft->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

	ArrowRight = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowRight"));
	ArrowRight->SetupAttachment(GetMesh());
	ArrowRight->SetRelativeLocation(FVector((GetMesh()->GetComponentLocation()).X - 150, (GetMesh()->GetComponentLocation()).Y, (GetMesh()->GetComponentLocation()).Z+100));
	ArrowRight->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ATestWeaponProjectileCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	InitWeapon(WeaponName);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATestWeaponProjectileCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATestWeaponProjectileCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestWeaponProjectileCharacter::Look);

		//Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::InputAttackPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATestWeaponProjectileCharacter::InputAttackReleased);

		//EnhancedInputComponent->BindAction(ChangePosCameraAction, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::ChangePosCameraPlayer);

		/*EnhancedInputComponent->BindAction(ChangePosCameraLeft, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::ChangePosCameraLeftPressed);
		EnhancedInputComponent->BindAction(ChangePosCameraLeft, ETriggerEvent::Completed, this, &ATestWeaponProjectileCharacter::ChangePosCameraLeftReleased);

		EnhancedInputComponent->BindAction(ChangePosCameraRight, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::ChangePosCameraRightPressed);
		EnhancedInputComponent->BindAction(ChangePosCameraRight, ETriggerEvent::Completed, this, &ATestWeaponProjectileCharacter::ChangePosCameraRightReleased);
		*/
		EnhancedInputComponent->BindAction(AimFirstView, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::AimFirstViewPressed);
		EnhancedInputComponent->BindAction(AimThirdView, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::AimThirdViewPressed);
		EnhancedInputComponent->BindAction(GetFreeCamera, ETriggerEvent::Started, this, &ATestWeaponProjectileCharacter::GetFreeCameraPressed);
		EnhancedInputComponent->BindAction(GetFreeCamera, ETriggerEvent::Completed, this, &ATestWeaponProjectileCharacter::GetFreeCameraReleased);
		
	}
}

void ATestWeaponProjectileCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();
	//!FreeCamera &&
	if (!FreeCamera && Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}

	//TraceLeftRight();
}

/*void ATestWeaponProjectileCharacter::TraceLeftRight()
{
	FHitResult OutHitLeft;
	FHitResult OutHitRight;
	//if (ChangeCameraPos)
	//{
		DrawDebugLine(GetWorld(), ArrowLeft->GetComponentLocation(), ArrowLeft->GetComponentLocation() + ((UKismetMathLibrary::GetForwardVector(ArrowLeft->GetComponentRotation())) * 300), FColor(255, 0, 0), false, -1, 0, 5);
		GetWorld()->LineTraceSingleByChannel(OutHitLeft, ArrowLeft->GetComponentLocation(), ArrowLeft->GetComponentLocation() + ((UKismetMathLibrary::GetForwardVector(ArrowLeft->GetComponentRotation())) * 300), ECC_Visibility);

		DrawDebugLine(GetWorld(), ArrowRight->GetComponentLocation(), ArrowRight->GetComponentLocation() + ((UKismetMathLibrary::GetForwardVector(ArrowRight->GetComponentRotation())) * 300), FColor(0, 255, 0), false, -1, 0, 5);
		GetWorld()->LineTraceSingleByChannel(OutHitRight, ArrowRight->GetComponentLocation(), ArrowRight->GetComponentLocation() + ((UKismetMathLibrary::GetForwardVector(ArrowRight->GetComponentRotation())) * 300), ECC_Visibility);

	//}
	
		UE_LOG(LogTemp, Warning, TEXT("TraceLeft  %f"), OutHitLeft.Distance);
		UE_LOG(LogTemp, Warning, TEXT("TraceRight  %f"), OutHitRight.Distance);

		if (OutHitRight.Distance == 0 && OutHitLeft.Distance > 0)
		{
			ChangePosCamera(true);
		}

		if (OutHitLeft.Distance == 0 && OutHitRight.Distance > 0)
		{
			ChangePosCamera(false);
		}
	
}*/


void ATestWeaponProjectileCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	//AimCameraView&&
	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
	//TraceLeftRight();
}

void ATestWeaponProjectileCharacter::InitWeapon(FName IdWeaponName)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}

	UMyGameInstance* myGI = Cast<UMyGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;
	if (myGI)
	{
		if (myGI->GetWeaponInfoByName(IdWeaponName, myWeaponInfo))
		{
			if (myWeaponInfo.WeaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();

				AWeaponBase* myWeapon = Cast<AWeaponBase>(GetWorld()->SpawnActor(myWeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myWeapon)
				{
					myWeapon->WeaponSetting = myWeaponInfo;
					FProjectileInfo ProjectileInfo;
					myGI->GetProjectileInfoByName(myWeapon->WeaponSetting.TypeProjectile, ProjectileInfo);
					myWeapon->ProjectileInfo = ProjectileInfo;
					myWeapon->WeaponInit();
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = myWeapon;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("InitWeapon - Weapon not found in table -NULL"));
		}
	}
}



void ATestWeaponProjectileCharacter::InputAttackPressed(const FInputActionValue& Value)
{
	if(!FreeCamera)
		AttackCharEvent_OnServer(true);
	

	/*if (CameraView)
	{
		if (TimerHandle_WaitCameraAfterFireTimer.IsValid())
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_WaitCameraAfterFireTimer);
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector(160.0f, 30.0f, -45.0f), CameraBoom->GetRelativeRotation(), true, true, 0.2f, false, EMoveComponentAction::Move, LatentInfo);

	}*/
	
	//UKismetSystemLibrary::MoveComponentTo(CameraBoom, FVector(140.0f, 30.f, 40.0f), CameraBoom->GetRelativeRotation(), true, true, 0.3f, false, EMoveComponentAction::Move, LatentInfo);
}

void ATestWeaponProjectileCharacter::InputAttackReleased(const FInputActionValue& Value)
{
	AttackCharEvent_OnServer(false);
	

	//if (CameraView)
		//GetWorld()->GetTimerManager().SetTimer(TimerHandle_WaitCameraAfterFireTimer, this, &ATestWeaponProjectileCharacter::MoveCameraAfterFireTimer, 0.5f, false, 0.5f);

	
}

void ATestWeaponProjectileCharacter::AimFirstViewPressed(const FInputActionValue& Value)
{
	if (!isAimFirstView && !FreeCamera)
	{
		bUseControllerRotationYaw = true;
		FollowCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		//SocketAim
		//CameraBoom->SetupAttachment(CurrentWeapon->GetRootComponent(), "SocketAim");
		FollowCamera->AttachToComponent(CurrentWeapon->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false), "SocketAim");

		//CameraBoom->TargetArmLength = 0.0f;
		FollowCamera->SetRelativeLocation(FVector(0.0f, -8.0f, 20.0f));
		FollowCamera->SetRelativeRotation(FRotator((FollowCamera->GetRelativeRotation()).Pitch, 90.0f, (FollowCamera->GetRelativeRotation()).Roll));
		//CameraBoom->SetRelativeLocation(FVector(0.0f, -5.0f, -1.0f));
		//CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 0.0f);

		//CameraBoom->SocketOffset = FVector(5.0f, 12.3f, -6.0f);
		//FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

		UpdateAimCameraView_OnServer(true);
		//isAimFirstView = true;
	}
	else
	{
		if (!FreeCamera)
		{
			bUseControllerRotationYaw = false;
			FollowCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
			//CameraBoom->SetupAttachment(GetMesh(), "headSocket");
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false));

			FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			FollowCamera->SetRelativeRotation(FRotator((FollowCamera->GetRelativeRotation()).Pitch, 0.0f, (FollowCamera->GetRelativeRotation()).Roll));
			//FollowCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
			//CameraBoom->SetRelativeLocation(FVector(35.0f, 0.0f, 70.0f));
			//CameraBoom->TargetArmLength = 200.0f;
			//CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 50.0f);


			UpdateAimCameraView_OnServer(false);
			//isAimFirstView = false;
		}
	}
}




void ATestWeaponProjectileCharacter::UpdateAimCameraView_OnServer_Implementation(bool bAimCameraView)
{
	isAimFirstView = bAimCameraView;
}


void ATestWeaponProjectileCharacter::AimThirdViewPressed(const FInputActionValue& Value)
{
	if (!isAimFirstView && !FreeCamera)
	{
		if (isAimThirdView)
		{
			MoveCamera_OnServer(this, FollowCamera, FVector(0.0f, 0.0f, 0.0f));
			isAimThirdView = false;

		}
		else
		{
			MoveCamera_OnServer(this, FollowCamera, FVector(140.0f, 30.0f, -55.0f));
			isAimThirdView = true;
		}
	}

}

void ATestWeaponProjectileCharacter::MoveCamera_OnServer_Implementation(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location)
{
	MoveCamera_Multicast(Char, Camera, Location);
	//TraceLocation = Location;
	//TraceForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);
}

void ATestWeaponProjectileCharacter::MoveCamera_Multicast_Implementation(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location)
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = Char;
	UKismetSystemLibrary::MoveComponentTo(Camera, Location, FRotator((Camera->GetRelativeRotation()).Pitch, 0.0f, (Camera->GetRelativeRotation()).Roll), true, true, 0.2f, false, EMoveComponentAction::Move, LatentInfo);

	//TraceLocation = Location;
	//TraceForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);
}



void ATestWeaponProjectileCharacter::GetFreeCameraPressed(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("GetFreeCameraPressed"));
	if (!isAimFirstView)
	{
		FreeCamera = true;
	}
}

void ATestWeaponProjectileCharacter::GetFreeCameraReleased(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("GetFreeCameraReleased"));
	if (!isAimFirstView)
	{
		FreeCamera = false;
	}
}

/*void ATestWeaponProjectileCharacter::MoveCameraAfterFireTimer()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	
	UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector(0.0f, 0.0f, 0.0f), CameraBoom->GetRelativeRotation(), true, true, 0.5f, false, EMoveComponentAction::Move, LatentInfo);
	//UKismetSystemLibrary::MoveComponentTo(CameraBoom, FVector(0.0f, 0.0f, 70.0f), CameraBoom->GetRelativeRotation(), true, true, 0.5f, false, EMoveComponentAction::Move, LatentInfo);
	if (TimerHandle_WaitCameraAfterFireTimer.IsValid())
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_WaitCameraAfterFireTimer);
}*/

AWeaponBase* ATestWeaponProjectileCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATestWeaponProjectileCharacter::AttackCharEvent_OnServer_Implementation(bool bIsFiring)
{
	/*AWeaponBase* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();
	myWeapon->ShootLocationCharacterCamera = &ShootArrow;
	myWeapon->TraceCamera = &AimCameraView;
	if (myWeapon)
	{
		myWeapon->SetWeaponStateFire(bIsFiring);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("AttackCharEvent - CurrentWeapon -NULL"));*/
	CurrentWeapon = GetCurrentWeapon();
	//CurrentWeapon->ShootLocationCharacterCamera = &ShootArrow;
	//CurrentWeapon->TraceCamera = &AimCameraView;
	if (CurrentWeapon)
		SetWeaponStateFire(bIsFiring);

}

/*
void ATestWeaponProjectileCharacter::ChangeCameraViewPressed(const FInputActionValue& Value)
{
	if (CameraView)
	{
		CameraBoom->SetupAttachment(GetMesh(), "headSocket");
		CameraBoom->TargetArmLength = 0.0f;

		//CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
		FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CameraView = false;
	}
	else
	{
		//CameraBoom->SetupAttachment(RootComponent);
		CameraBoom->TargetArmLength = 200.0f;
		//CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 50.0f);
		CameraView = true;
	}
}

void ATestWeaponProjectileCharacter::ChangePosCameraLeftPressed(const FInputActionValue& Value)
{
	if (CameraView)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector(0.0f, -100.f, 0.0f), FollowCamera->GetRelativeRotation(), true, true, 0.3f, false, EMoveComponentAction::Move, LatentInfo);
	}
}

void ATestWeaponProjectileCharacter::ChangePosCameraLeftReleased(const FInputActionValue& Value)
{
	if (CameraView)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector(0.0f, 0.0f, 00.0f), FollowCamera->GetRelativeRotation(), true, true, 0.3f, false, EMoveComponentAction::Move, LatentInfo);
	}
}

void ATestWeaponProjectileCharacter::ChangePosCameraRightPressed(const FInputActionValue& Value)
{
	if (CameraView)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector(0.0f, 100.f, 0.0f), FollowCamera->GetRelativeRotation(), true, true, 0.3f, false, EMoveComponentAction::Move, LatentInfo);
	}
}

void ATestWeaponProjectileCharacter::ChangePosCameraRightReleased(const FInputActionValue& Value)
{
	if (CameraView)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(FollowCamera, FVector(0.0f, 0.0f, 0.0f), FollowCamera->GetRelativeRotation(), true, true, 0.3f, false, EMoveComponentAction::Move, LatentInfo);
	}
}*/

void ATestWeaponProjectileCharacter::SetWeaponStateFire(bool bIsFire)
{
	if (GetWorld())
	{
		if (bIsFire)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_FireRateTimer, this, &ATestWeaponProjectileCharacter::Fire, CurrentWeapon->WeaponSetting.RateOfFire, true, 0.0f);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_FireRateTimer);
		}
	}
}




void ATestWeaponProjectileCharacter::UpdateTraceLocAndRot_OnServer_Implementation(FVector Location, FRotator Rotation)
{
	UpdateTraceLocAndRot_Multicast(Location, Rotation);
	//TraceLocation = Location;
	//TraceForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);
}

void ATestWeaponProjectileCharacter::UpdateTraceLocAndRot_Multicast_Implementation(FVector Location, FRotator Rotation)
{
	TraceLocation = Location;
	TraceForwardVector = UKismetMathLibrary::GetForwardVector(Rotation);
}


void ATestWeaponProjectileCharacter::Fire_Server_Implementation()
{
	Fire();
}

void ATestWeaponProjectileCharacter::Fire()
{
	if (!HasAuthority())
	{
		Fire_Server();
	}
	else
	{
		UArrowComponent* ShootLocation = CurrentWeapon->ShootLocation;
		if (ShootLocation)
		{
			FVector SpawnLocation = ShootLocation->GetComponentLocation();
			FRotator SpawnRotation = ShootLocation->GetComponentRotation();

			if (CurrentWeapon->ProjectileInfo.Projectile)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AProjectileBase* myProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(CurrentWeapon->ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myProjectile)
				{
					myProjectile->SetLifeSpan(CurrentWeapon->WeaponSetting.ProjectileLifeTime);
					myProjectile->ProjectileDamage = CurrentWeapon->WeaponSetting.ProjectileDamage;

					FHitResult OutHit;
					FPredictProjectilePathParams PathParams;

					//DispersionWeapon();
					//UE_LOG(LogTemp, Warning, TEXT("DispersionWeapon  x= %f y= %f z= %f"), CurrentDispersion.X, CurrentDispersion.Y, CurrentDispersion.Z);
					//DrawDebugLine(GetWorld(), (*ShootLocationCharacterCamera)->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector((*ShootLocationCharacterCamera)->GetComponentRotation())) * 100 * WeaponSetting.ProjectileSpeed + CurrentDispersion * 100) + (*ShootLocationCharacterCamera)->GetComponentLocation(), FColor(0, 0, 255), false, 3.0f, 0, 1);

					FCollisionQueryParams CollisionParams;
					CollisionParams.AddIgnoredActor(this);
					CollisionParams.AddIgnoredActor(CurrentWeapon);
					CollisionParams.AddIgnoredActor(myProjectile);

					//CollisionParams.AddIgnoredActor(this->GetAttachParentActor()->GetRootComponent());
					if (!isAimFirstView)
					{
						//Camera shoot
						UpdateTraceLocAndRot_OnServer(ShootArrow->GetComponentLocation(), ShootArrow->GetComponentRotation());
						DrawDebugLine(GetWorld(), TraceLocation, (TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + TraceLocation, FColor(255, 255, 0), false, 3.0f, 0, 1);

						//DrawDebugLine(GetWorld(), ShootArrow->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector(ShootArrow->GetComponentRotation())) * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + ShootArrow->GetComponentLocation(), FColor(0, 0, 255), false, 3.0f, 0, 1);
						UE_LOG(LogTemp, Warning, TEXT("Camera shoot"));
						if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceLocation, (TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + TraceLocation, ECC_Visibility, CollisionParams))
						{
							DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 10.0f, 10, FColor(30, 30, 30), false, 3.0f, 0, 1);
							PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - CurrentWeapon->ShootLocation->GetComponentLocation()).Rotation()) * CurrentWeapon->WeaponSetting.ProjectileSpeed;
							UE_LOG(LogTemp, Warning, TEXT("SimFrequency  %f"), FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500));
							PathParams.SimFrequency = FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500);
						}
						else
						{
							PathParams.LaunchVelocity = TraceForwardVector * CurrentWeapon->WeaponSetting.ProjectileSpeed;
							PathParams.SimFrequency = 10.0f;
						}
					}
					else
					{
						//Aim shoot
						UE_LOG(LogTemp, Warning, TEXT("Aim shoot"));
						//UpdateTraceLocAndRot_OnServer(CurrentWeapon->AimLocation->GetComponentLocation(), CurrentWeapon->AimLocation->GetComponentRotation());
						UpdateTraceLocAndRot_OnServer(CurrentWeapon->AimLocation->GetComponentLocation(), CurrentWeapon->AimLocation->GetComponentRotation());
						UE_LOG(LogTemp, Warning, TEXT("AimLocation x = %f y = %f z = %f"), CurrentWeapon->AimLocation->GetComponentLocation().X, CurrentWeapon->AimLocation->GetComponentLocation().Y, CurrentWeapon->AimLocation->GetComponentLocation().Z);
						//TraceLocation = UpdateTraceLoc_OnServer(CurrentWeapon->AimLocation->GetComponentLocation());
						//TraceForwardVector = UpdateTraceRot_OnServer(CurrentWeapon->AimLocation->GetComponentRotation());
						DrawDebugLine(GetWorld(), TraceLocation, (TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + CurrentWeapon->AimLocation->GetComponentLocation(), FColor(255, 255, 0), false, 3.0f, 0, 1);

						//DrawDebugLine(GetWorld(), TraceLocation, TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + TraceLocation, FColor(0, 0, 255), false, 3.0f, 0, 1);

						if (GetWorld()->LineTraceSingleByChannel(OutHit, TraceLocation, TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed + TraceLocation, ECC_Visibility, CollisionParams))
						{
							PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - TraceLocation).Rotation()) * CurrentWeapon->WeaponSetting.ProjectileSpeed;
							UE_LOG(LogTemp, Warning, TEXT("SimFrequency  %f"), FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500));
							PathParams.SimFrequency = FMath::RoundHalfFromZero(((OutHit.ImpactPoint - TraceLocation).Size()) / 500);
						}
						else
						{
							PathParams.LaunchVelocity = TraceForwardVector * CurrentWeapon->WeaponSetting.ProjectileSpeed;
							PathParams.SimFrequency = 10.0f;
						}
					}

					PathParams.StartLocation = CurrentWeapon->ShootLocation->GetComponentLocation();
					PathParams.bTraceWithCollision = true;
					PathParams.ProjectileRadius = 10.0f;
					PathParams.TraceChannel = ECC_Visibility;
					//PathParams.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
					//PathParams.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);
					//PathParams.ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
					PathParams.bTraceComplex = true;
					PathParams.bTraceWithChannel = true;
					PathParams.DrawDebugType = EDrawDebugTrace::None;
					PathParams.DrawDebugTime = 0.0f;
					PathParams.MaxSimTime = 2.0f;
					PathParams.OverrideGravityZ = CurrentWeapon->WeaponSetting.ProjectileMass;
					PathParams.ActorsToIgnore.Add(this);
					PathParams.ActorsToIgnore.Add(CurrentWeapon);
					PathParams.ActorsToIgnore.Add(myProjectile);

					FPredictProjectilePathResult PathResult;
					myProjectile->bHit = UGameplayStatics::PredictProjectilePath(GetWorld(), PathParams, PathResult);
					UE_LOG(LogTemp, Warning, TEXT("OutHit.Distance   %f"), OutHit.Distance);
					CurrentWeapon->Fire2(myProjectile, PathResult.PathData, UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - TraceLocation).Rotation()), (OutHit.Distance < 600 && OutHit.Distance > 0));
					UE_LOG(LogTemp, Warning, TEXT("NEW FIRE2"));
					//myProjectile->InitProjectile(ProjectileInfo, PathResult.PathData, UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Rotation()), (OutHit.Distance < 600 && OutHit.Distance > 0));
				}
			}
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CurrentWeapon->WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CurrentWeapon->WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());
		}
	}
}


void ATestWeaponProjectileCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATestWeaponProjectileCharacter, CurrentWeapon);
	DOREPLIFETIME(ATestWeaponProjectileCharacter, isAimFirstView);
	//
	DOREPLIFETIME(ATestWeaponProjectileCharacter, isAimThirdView);
	DOREPLIFETIME(ATestWeaponProjectileCharacter, TraceLocation);
	DOREPLIFETIME(ATestWeaponProjectileCharacter, TraceForwardVector);
	//
	DOREPLIFETIME(ATestWeaponProjectileCharacter, ShootArrow);
}