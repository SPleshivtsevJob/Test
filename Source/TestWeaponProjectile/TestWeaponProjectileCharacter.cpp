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
	this->SetReplicates(true);

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
	if (!isFreeCamera && Controller != nullptr)
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
}

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
	if(!isFreeCamera)
		AttackCharEvent(true);
}

void ATestWeaponProjectileCharacter::InputAttackReleased(const FInputActionValue& Value)
{
	AttackCharEvent(false);
}

void ATestWeaponProjectileCharacter::AimFirstViewPressed(const FInputActionValue& Value)
{
	if (!isAimFirstView && !isFreeCamera)
	{
		bUseControllerRotationYaw = true;
		FollowCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		FollowCamera->AttachToComponent(CurrentWeapon->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false), "CameraSocket");

		FollowCamera->SetRelativeLocation(FVector(0.0f, -8.0f, 20.0f));
		FollowCamera->SetRelativeRotation(FRotator((FollowCamera->GetRelativeRotation()).Pitch, 90.0f, (FollowCamera->GetRelativeRotation()).Roll));
		
		isAimFirstView = true;
		SetAimFirstView_OnServer(true);
	}
	else
	{
		if (!isFreeCamera)
		{
			bUseControllerRotationYaw = false;
			FollowCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false));

			FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			FollowCamera->SetRelativeRotation(FRotator((FollowCamera->GetRelativeRotation()).Pitch, 0.0f, (FollowCamera->GetRelativeRotation()).Roll));
			
			isAimFirstView = false;
			SetAimFirstView_OnServer(false);
		}
	}
}

void ATestWeaponProjectileCharacter::SetAimFirstView_OnServer_Implementation(bool bIsAimFirst)
{
	if (bIsAimFirst)
	{
		bUseControllerRotationYaw = true;
		FollowCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
		FollowCamera->AttachToComponent(CurrentWeapon->GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false), "CameraSocket");

		FollowCamera->SetRelativeLocation(FVector(0.0f, -8.0f, 20.0f));
		FollowCamera->SetRelativeRotation(FRotator((FollowCamera->GetRelativeRotation()).Pitch, 90.0f, (FollowCamera->GetRelativeRotation()).Roll));

		isAimFirstView = true;
		//SetAimFirstView_OnServer(true);
	}
	else
	{

			bUseControllerRotationYaw = false;
			FollowCamera->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, false));
			FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false));

			FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			FollowCamera->SetRelativeRotation(FRotator((FollowCamera->GetRelativeRotation()).Pitch, 0.0f, (FollowCamera->GetRelativeRotation()).Roll));

			isAimFirstView = false;
			//SetAimFirstView_OnServer(false);

	}
}

void ATestWeaponProjectileCharacter::AimThirdViewPressed(const FInputActionValue& Value)
{
	if (!isAimFirstView && !isFreeCamera)
	{
		if (isAimThirdView)
		{
			MoveCamera(this, FollowCamera, FVector(0.0f, 0.0f, 0.0f));
			isAimThirdView = false;
			SetAimThirdView_OnServer(false);
		}
		else
		{
			MoveCamera(this, FollowCamera, FVector(140.0f, 30.0f, -55.0f));
			isAimThirdView = true;
			SetAimThirdView_OnServer(true);
		}
	}
}

void ATestWeaponProjectileCharacter::SetAimThirdView_OnServer_Implementation(bool bIsAimThird)
{
	if (!bIsAimThird)
	{
		MoveCamera(this, FollowCamera, FVector(0.0f, 0.0f, 0.0f));
		isAimThirdView = false;
		//SetAimThirdView_OnServer(false);
	}
	else
	{
		MoveCamera(this, FollowCamera, FVector(140.0f, 30.0f, -55.0f));
		isAimThirdView = true;
		//SetAimThirdView_OnServer(true);
	}
}


void ATestWeaponProjectileCharacter::MoveCamera(ATestWeaponProjectileCharacter* Char, UCameraComponent* Camera, FVector Location)
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = Char;
	UKismetSystemLibrary::MoveComponentTo(Camera, Location, FRotator((Camera->GetRelativeRotation()).Pitch, 0.0f, (Camera->GetRelativeRotation()).Roll), true, true, 0.2f, false, EMoveComponentAction::Move, LatentInfo);
}

void ATestWeaponProjectileCharacter::GetFreeCameraPressed(const FInputActionValue& Value)
{
	if (!isAimFirstView)
	{
		isFreeCamera = true;
		SetIsFreeCamera_OnServer(true);
	}
}

void ATestWeaponProjectileCharacter::GetFreeCameraReleased(const FInputActionValue& Value)
{
	if (!isAimFirstView)
	{
		isFreeCamera = false;
		SetIsFreeCamera_OnServer(false);
	}
}

void ATestWeaponProjectileCharacter::SetIsFreeCamera_OnServer_Implementation(bool bisFree)
{
	isFreeCamera = bisFree;
}

AWeaponBase* ATestWeaponProjectileCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}

void ATestWeaponProjectileCharacter::AttackCharEvent_Implementation(bool bIsFiring)
{
	CurrentWeapon = GetCurrentWeapon();
	if (CurrentWeapon)
		SetWeaponStateFire(bIsFiring);
}

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

void ATestWeaponProjectileCharacter::Fire()
{
	CurrentWeapon->Fire(ShootArrow->GetComponentLocation(), (UKismetMathLibrary::GetForwardVector(ShootArrow->GetComponentRotation())), isAimFirstView);
}
	
void ATestWeaponProjectileCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATestWeaponProjectileCharacter, CurrentWeapon);
	//DOREPLIFETIME(ATestWeaponProjectileCharacter, FollowCamera);
}

/*		UArrowComponent* ShootLocation = CurrentWeapon->ShootLocation;
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

					FCollisionQueryParams CollisionParams;
					CollisionParams.AddIgnoredActor(this);
					CollisionParams.AddIgnoredActor(CurrentWeapon);
					CollisionParams.AddIgnoredActor(myProjectile);

					if (!isAimFirstView)
					{
						//Camera shoot
						UE_LOG(LogTemp, Warning, TEXT("Camera shoot"));
						UpdateTraceLocAndRot(ShootArrow->GetComponentLocation(), ShootArrow->GetComponentRotation());
						DrawDebugLine(GetWorld(), TraceLocation, (TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + TraceLocation, FColor(255, 255, 0), false, 3.0f, 0, 1);

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
						UpdateTraceLocAndRot(CurrentWeapon->AimLocation->GetComponentLocation(), CurrentWeapon->AimLocation->GetComponentRotation());
						DrawDebugLine(GetWorld(), TraceLocation, (TraceForwardVector * 100 * CurrentWeapon->WeaponSetting.ProjectileSpeed) + CurrentWeapon->AimLocation->GetComponentLocation(), FColor(255, 255, 0), false, 3.0f, 0, 1);

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
					CurrentWeapon->Fire(myProjectile, PathResult.PathData, UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - TraceLocation).Rotation()), (OutHit.Distance < 600 && OutHit.Distance > 0));
				}
			}
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CurrentWeapon->WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CurrentWeapon->WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());
		}
}*/