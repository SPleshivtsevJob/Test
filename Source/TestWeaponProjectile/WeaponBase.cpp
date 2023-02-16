// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "MyGameInstance.h"
#include "Net/UnrealNetWork.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//this->SetReplicates(true);
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);
	//RootComponent = StaticMeshWeapon;

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(StaticMeshWeapon);
	AimLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("AimLocation"));
	AimLocation->SetupAttachment(StaticMeshWeapon);
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::WeaponInit()
{
	StaticMeshWeapon->SetStaticMesh(WeaponSetting.StaticMeshWeapon);
	StaticMeshWeapon->SetRelativeTransform(WeaponSetting.WeaponStaticMeshOffset);
	ShootLocation->SetRelativeTransform(WeaponSetting.ShootLocationOffset);
	AimLocation->SetRelativeTransform(WeaponSetting.AimLocationOffset);
}

//fix if you quickly press the shot button once, the bullet will not appear as the timer will be created and immediately destroyed
void AWeaponBase::SetWeaponStateFire(bool bIsFire)
{
	if (GetWorld())
	{
		if (bIsFire)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_FireRateTimer, this, &AWeaponBase::Fire, WeaponSetting.RateOfFire, true, 0.0f);
		}
		else
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle_FireRateTimer);
		}
	}
}

void AWeaponBase::DispersionWeapon()
{
	float Dis = WeaponSetting.Dispersion;
	if(CurrentDispersion.Size() == 0.0f)
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_RateDecreaseDispersionTimer, this, &AWeaponBase::DecreaseDispersion, WeaponSetting.RateOfDecreaseDispersion, true, WeaponSetting.RateOfDecreaseDispersion);

	CurrentDispersion = CurrentDispersion + FVector(FMath::RandRange((-1) * Dis, Dis), FMath::RandRange((-1) * Dis, Dis), FMath::RandRange((-1) * Dis, Dis));
}

void AWeaponBase::DecreaseDispersion()
{
	CurrentDispersion = FVector(CurrentDispersion.X - WeaponSetting.Dispersion, CurrentDispersion.Y - WeaponSetting.Dispersion, CurrentDispersion.Z - WeaponSetting.Dispersion);

	if (CurrentDispersion.X < 0.0f)
		CurrentDispersion.X = 0.0f;
	if (CurrentDispersion.Y < 0.0f)
		CurrentDispersion.Y = 0.0f;
	if (CurrentDispersion.Z < 0.0f)
		CurrentDispersion.Z = 0.0f;



	if(CurrentDispersion.Size() == 0.0f)
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_RateDecreaseDispersionTimer);
}

void AWeaponBase::Fire2(AProjectileBase* myProjectile, TArray <FPredictProjectilePathPointData> PathData, FVector FireVector, bool CloseHit)
{
	myProjectile->InitProjectile(ProjectileInfo, PathData, FireVector, CloseHit);
}


void AWeaponBase::Fire()
{
	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();

		if (ProjectileInfo.Projectile)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();

			AProjectileBase* myProjectile = Cast<AProjectileBase>(GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (myProjectile)
			{
				myProjectile->SetLifeSpan(WeaponSetting.ProjectileLifeTime);
				myProjectile->ProjectileDamage = WeaponSetting.ProjectileDamage;

				FHitResult OutHit;
				FPredictProjectilePathParams PathParams;
				
				//DispersionWeapon();
				//UE_LOG(LogTemp, Warning, TEXT("DispersionWeapon  x= %f y= %f z= %f"), CurrentDispersion.X, CurrentDispersion.Y, CurrentDispersion.Z);
				//DrawDebugLine(GetWorld(), (*ShootLocationCharacterCamera)->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector((*ShootLocationCharacterCamera)->GetComponentRotation())) * 100 * WeaponSetting.ProjectileSpeed + CurrentDispersion * 100) + (*ShootLocationCharacterCamera)->GetComponentLocation(), FColor(0, 0, 255), false, 3.0f, 0, 1);

				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(this);
				CollisionParams.AddIgnoredActor(this->GetAttachParentActor());
				//CollisionParams.AddIgnoredActor(this->GetAttachParentActor()->GetRootComponent());
				if (*TraceCamera)
				{
					DrawDebugLine(GetWorld(), (*ShootLocationCharacterCamera)->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector((*ShootLocationCharacterCamera)->GetComponentRotation())) * 100 * WeaponSetting.ProjectileSpeed + CurrentDispersion * 100) + (*ShootLocationCharacterCamera)->GetComponentLocation(), FColor(0, 0, 255), false, 3.0f, 0, 1);

					if (GetWorld()->LineTraceSingleByChannel(OutHit, (*ShootLocationCharacterCamera)->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector((*ShootLocationCharacterCamera)->GetComponentRotation())) * 100 * WeaponSetting.ProjectileSpeed + CurrentDispersion * 100) + (*ShootLocationCharacterCamera)->GetComponentLocation(), ECC_Visibility, CollisionParams))
					{
						PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Rotation()) * WeaponSetting.ProjectileSpeed;
						UE_LOG(LogTemp, Warning, TEXT("SimFrequency  %f"), FMath::RoundHalfFromZero(((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Size()) / 500));
						PathParams.SimFrequency = FMath::RoundHalfFromZero(((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Size()) / 500);
					}
					else
					{
						PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((*ShootLocationCharacterCamera)->GetComponentRotation()) * WeaponSetting.ProjectileSpeed;
						PathParams.SimFrequency = 10.0f;
					}
				}
				else
				{
					DrawDebugLine(GetWorld(), AimLocation->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector(AimLocation->GetComponentRotation())) * 100 * WeaponSetting.ProjectileSpeed + CurrentDispersion * 100) + AimLocation->GetComponentLocation(), FColor(0, 0, 255), false, 3.0f, 0, 1);

					if (GetWorld()->LineTraceSingleByChannel(OutHit, AimLocation->GetComponentLocation(), ((UKismetMathLibrary::GetForwardVector(AimLocation->GetComponentRotation())) * 100 * WeaponSetting.ProjectileSpeed + CurrentDispersion * 100) + AimLocation->GetComponentLocation(), ECC_Visibility, CollisionParams))
					{
						PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Rotation()) * WeaponSetting.ProjectileSpeed;
						UE_LOG(LogTemp, Warning, TEXT("SimFrequency  %f"), FMath::RoundHalfFromZero(((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Size()) / 500));
						PathParams.SimFrequency = FMath::RoundHalfFromZero(((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Size()) / 500);
					}
					else
					{
						PathParams.LaunchVelocity = UKismetMathLibrary::GetForwardVector(ShootLocation->GetComponentRotation()) * WeaponSetting.ProjectileSpeed;
						PathParams.SimFrequency = 10.0f;
					}
				}
				
				PathParams.StartLocation = ShootLocation->GetComponentLocation();
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
				PathParams.OverrideGravityZ = WeaponSetting.ProjectileMass;
				PathParams.ActorsToIgnore.Add(this);
				PathParams.ActorsToIgnore.Add(myProjectile);

				FPredictProjectilePathResult PathResult;
				myProjectile->bHit = UGameplayStatics::PredictProjectilePath(GetWorld(), PathParams, PathResult);
				UE_LOG(LogTemp, Warning, TEXT("OutHit.Distance   %f"), OutHit.Distance);
				myProjectile->InitProjectile(ProjectileInfo, PathResult.PathData, UKismetMathLibrary::GetForwardVector((OutHit.ImpactPoint - ShootLocation->GetComponentLocation()).Rotation()), (OutHit.Distance < 600 && OutHit.Distance > 0));
				UE_LOG(LogTemp, Warning, TEXT("OLD FIRE"));
			}
		}
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon, ShootLocation->GetComponentLocation());
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon, ShootLocation->GetComponentTransform());
	}
}
